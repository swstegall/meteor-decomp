#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Find the best Rosetta Stone candidate function: a small, simple, standalone
function whose pseudo-C is unambiguous enough that we expect it to byte-match
under MSVC 2005 SP1 with default /O2 — used in Phase 2 to pin compiler flags.

Selection criteria (in order):
  - 16 ≤ size ≤ 96 bytes (large enough to be discriminating, small enough
    to hand-translate)
  - in `.text` section
  - no callees (no `call` / `jmp <symbol>` outside the function body)
  - no SEH / try-catch (frame-based exception handlers visible as
    `Catch_All@*` or unique stack-frame setup)
  - prefer ones with named, non-FUN_ symbols (heavier signal — likely a
    real, non-thunk function)
  - prefer pure integer arithmetic / bitwise ops (skip FP code — MSVC's
    x87 vs SSE codegen choices add ambiguity)
  - skip anything with local arrays >= 5 bytes (would trigger /GS cookie
    insertion, complicating the match)

Reads:  config/<binary>.symbols.json + asm/<binary>/*.s
Writes: build/rosetta/<binary>.candidates.json — top 50 ranked functions
        build/rosetta/<binary>.top.txt        — disassembly of the #1 pick
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CONFIG = REPO_ROOT / "config"
ASM_ROOT = REPO_ROOT / "asm"
OUT = REPO_ROOT / "build" / "rosetta"

# DumpFunctions.java emits per-instruction lines shaped like:
#   "    009d0c52:  e8 29 d3 ff ff                    CALL 0x00dcdf80"
# i.e. address-colon, *space-separated* byte hex, padding, mnemonic, operands.
# Our regexes must walk past the multi-byte hex, so we look for the
# mnemonic anchored after at least one address-colon and some whitespace.
# Mnemonic is uppercase, between word boundaries.
def _mnemonic_line(mnemonic_alt: str) -> "re.Pattern":
    return re.compile(
        rf"^\s*[0-9a-f]+:\s+[0-9a-f][0-9a-f \t]*\b({mnemonic_alt})\b",
        re.IGNORECASE | re.MULTILINE,
    )

RE_CALL    = _mnemonic_line(r"CALL")
RE_JMP_FAR = _mnemonic_line(r"JMP")  # any JMP — even short JMP $+N is suspicious for a "leaf" function
RE_FP      = _mnemonic_line(r"F[A-Z]+|MOVS[SD]")
RE_INT_OPS = _mnemonic_line(r"MOV|ADD|SUB|AND|OR|XOR|SHL|SHR|SAR|TEST|CMP|LEA|INC|DEC|NEG|NOT|IMUL|MUL|DIV|IDIV|RET|PUSH|POP")
RE_LOOP    = _mnemonic_line(r"LOOP|REP")

# Symbols we don't want as Rosetta candidates.
SKIP_NAME = re.compile(
    r"^("
    r"thunk_|Catch_All|vector_|FUN_$|"  # Ghidra-internal / SEH stubs
    r".*::~|.*::operator|"                # destructors / operators (ABI quirks)
    r".*::vector_|.*::scalar_|"           # MSVC compiler-generated
    r"_|__"                               # underscore-prefixed (CRT)
    r")"
)


def score_function(asm_text: str) -> tuple[float, dict]:
    """Score 0..100. Higher = better Rosetta candidate."""
    facts = {
        "calls": len(RE_CALL.findall(asm_text)),
        "far_jumps": len(RE_JMP_FAR.findall(asm_text)),
        "fp_ops": len(RE_FP.findall(asm_text)),
        "int_ops": len(RE_INT_OPS.findall(asm_text)),
        "loops": len(RE_LOOP.findall(asm_text)),
    }
    # Disqualifiers — anything > 0 returns score 0.
    if facts["calls"] > 0:
        return (0.0, facts)
    if facts["far_jumps"] > 0:
        return (0.0, facts)
    if facts["fp_ops"] > 0:
        return (0.0, facts)
    if facts["int_ops"] < 4:
        # Trivial 1-2 instr functions (constant returns) are TOO simple —
        # they match under any compiler and prove nothing.
        return (0.0, facts)

    score = 50.0
    score += min(facts["int_ops"], 30)            # density of real arithmetic
    score -= facts["loops"] * 5                   # loops add codegen variance
    return (score, facts)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", help="binary stem, e.g. ffxivgame")
    ap.add_argument("--top", type=int, default=20, help="how many candidates to print (default 20)")
    ap.add_argument("--min-size", type=int, default=16)
    ap.add_argument("--max-size", type=int, default=96)
    args = ap.parse_args()

    stem = args.binary.replace(".exe", "")
    sym_path = CONFIG / f"{stem}.symbols.json"
    asm_dir = ASM_ROOT / stem
    if not sym_path.exists() or not asm_dir.exists():
        print(f"error: missing dumps for {stem}; run import_to_ghidra.py first", file=sys.stderr)
        return 1

    fns = json.loads(sym_path.read_text())
    OUT.mkdir(parents=True, exist_ok=True)

    # Pre-index asm/ by RVA prefix — glob() per-function is O(n²) on 94k
    # dirs. One readdir + dict lookup is O(n).
    print(f"  indexing {asm_dir} ...", file=sys.stderr)
    asm_by_rva: dict[int, Path] = {}
    for p in asm_dir.iterdir():
        if not p.name.endswith(".s"):
            continue
        # Filename: <rva08x>_<...>.s
        try:
            rva = int(p.name[:8], 16)
            asm_by_rva[rva] = p
        except ValueError:
            continue
    print(f"  indexed {len(asm_by_rva)} .s files", file=sys.stderr)

    candidates = []
    skipped = {"size": 0, "section": 0, "name": 0, "no_asm": 0, "low_score": 0}
    for fn in fns:
        size = int(fn.get("size", 0))
        if size < args.min_size or size > args.max_size:
            skipped["size"] += 1
            continue
        if fn.get("section", "") != ".text":
            skipped["section"] += 1
            continue
        if SKIP_NAME.match(fn["name"]):
            skipped["name"] += 1
            continue
        rva = fn["rva"]
        asm_path = asm_by_rva.get(rva)
        if asm_path is None:
            skipped["no_asm"] += 1
            continue
        asm_text = asm_path.read_text()
        score, facts = score_function(asm_text)
        if score == 0:
            skipped["low_score"] += 1
            continue
        candidates.append({
            "rva": rva,
            "rva_hex": f"0x{rva:08x}",
            "name": fn["name"],
            "size": size,
            "score": score,
            "facts": facts,
            "asm_path": str(asm_path.relative_to(REPO_ROOT)),
        })

    candidates.sort(key=lambda c: (-c["score"], c["size"]))
    out_json = OUT / f"{stem}.candidates.json"
    out_json.write_text(json.dumps(candidates[:50], indent=2))

    if candidates:
        top = candidates[0]
        top_asm = (REPO_ROOT / top["asm_path"]).read_text()
        out_top = OUT / f"{stem}.top.txt"
        out_top.write_text(
            f"# Rosetta Stone #1 candidate for {stem}.exe\n"
            f"#   {top['name']} @ {top['rva_hex']}, size 0x{top['size']:x} bytes\n"
            f"#   score {top['score']:.1f}\n"
            f"#   facts {top['facts']}\n\n"
            + top_asm
        )

    # Report
    print(f"=== {stem}: {len(fns)} fns total ===")
    print(f"  skipped: {skipped}")
    print(f"  candidates: {len(candidates)}")
    print()
    print(f"=== top {min(args.top, len(candidates))} ===")
    for c in candidates[: args.top]:
        print(f"  score {c['score']:5.1f}  size {c['size']:3d}B  {c['rva_hex']}  {c['name']}  ({c['facts']})")
    if candidates:
        print()
        print(f"wrote top candidate disassembly: {out_top.relative_to(REPO_ROOT)}")
        print(f"wrote ranked candidate list:    {out_json.relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
