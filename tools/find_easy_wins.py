#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Scan a binary's per-function asm dumps and rank functions by how
likely they are to land a clean byte-identical match without needing
Ghidra GUI assistance.

The matching recipe (see `reference_meteor_decomp_rosetta_match.md`)
captures source patterns that consistently produce orig MSVC 2005 /O2
codegen. Some shapes are MUCH harder to match than others — anything
involving SEH, /GS stack cookies, hot-patch prologues, or FP code is
matching-hostile under our current toolchain. This script filters
those out and ranks what's left.

Reject criteria (function gets skipped entirely):
  - **hot-patch prologue** (`mov edi, edi` / `8b ff` as first instr) —
    forces an extra 2-byte hot-patch nop that's hard to coax out of
    cl.exe under our flags.
  - **/GS stack cookie** (references `[0x012ea8b0]`) — the
    `__security_cookie` xor-with-ESP setup adds noise we can't easily
    reproduce; would need exact array-size triggers in the source.
  - **SEH** (any `FS:[…]` access) — `__try`/`__except` frame setup
    via fs:0 is matching-hostile; the unwind tables differ between
    cl.exe builds.
  - **FP ops** (FLD / FSTP / FADD / FMUL / etc.) — MSVC 2005 default
    is x87, sequence is sensitive to schedule and we don't have
    matching infrastructure for it yet.
  - **size <= 6 B** — IAT thunks and trivial constant-getters; not
    valuable as match candidates (they're already shape-trivial).
  - **size > 200 B** — too much surface area; too many simultaneous
    decisions for the optimizer.
  - **`__purecall`** — abstract-vtable stubs that all share one impl.

Score: higher is better. Penalises size, CALL count, and branch
count (each adds a degree of optimizer-state sensitivity).

Usage:
  tools/find_easy_wins.py [binary]            # default ffxivgame
  tools/find_easy_wins.py ffxivgame --top 50

Output:
  build/easy_wins/<binary>.queue.json   ranked candidate queue
  build/easy_wins/<binary>.report.md    human-readable summary
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
ASM_ROOT = REPO_ROOT / "asm"
CONFIG_DIR = REPO_ROOT / "config"
OUT_ROOT = REPO_ROOT / "build" / "easy_wins"


# Pre-compiled regex set — the asm dumps are large and we walk
# 90k+ files, so anything inside a hot loop must be fast.
RE_GS_COOKIE = re.compile(r"012ea8b0", re.IGNORECASE)
RE_FS_PREFIX = re.compile(r"\bFS:")
RE_FP_OP = re.compile(
    r"\b(FADD|FSUB|FMUL|FDIV|FLD|FSTP?|FCOM|FXCH|FCHS|FABS|FSQRT|"
    r"FSIN|FCOS|FYL2X|FSCALE|FRNDINT|FPATAN|FPTAN|FPREM|FNINIT|"
    r"FNSTSW|FNSTCW|FLDCW|FNCLEX|FUCOMP?|F2XM1|FNOP|FINCSTP|"
    r"FDECSTP)\b"
)
RE_CALL = re.compile(r"\bCALL\b")
RE_BRANCH = re.compile(r"\bJ(?!MP\b)[A-Z]+\b")
RE_JMP = re.compile(r"\bJMP\b")
RE_HEADER_RVA = re.compile(r"^# rva\s+0x([0-9a-fA-F]+)", re.MULTILINE)
RE_HEADER_SIZE = re.compile(r"^# size\s+0x([0-9a-fA-F]+)", re.MULTILINE)
RE_HEADER_NAME = re.compile(r"^# function\s+(\S+)", re.MULTILINE)
RE_FIRST_INSTR_BYTES = re.compile(
    r"^\s*[0-9a-fA-F]+:\s+([0-9a-f][0-9a-f](?:\s+[0-9a-f][0-9a-f])*)\s",
    re.MULTILINE,
)
RE_FILENAME = re.compile(r"^([0-9a-f]+)_(.+)\.s$")


def classify(asm_text: str, name: str, size: int) -> dict:
    """Score one function. Returns a dict with verdict + diagnostics."""
    skip_reasons: list[str] = []

    # Size filters first (cheap).
    if size <= 6:
        skip_reasons.append("thunk")
    if size > 200:
        skip_reasons.append("too_big")

    # Special-name skips.
    if name in ("__purecall", "__alloca_probe", "_chkstk"):
        skip_reasons.append("crt_stub")

    # Pattern filters (cheap regex passes).
    if RE_GS_COOKIE.search(asm_text):
        skip_reasons.append("gs_cookie")
    if RE_FS_PREFIX.search(asm_text):
        skip_reasons.append("seh")
    if RE_FP_OP.search(asm_text):
        skip_reasons.append("fp")

    # Hot-patch prologue: first executable byte sequence is `8b ff`.
    first = RE_FIRST_INSTR_BYTES.search(asm_text)
    if first:
        first_bytes = first.group(1).lower().split()
        if len(first_bytes) >= 2 and first_bytes[0] == "8b" and first_bytes[1] == "ff":
            skip_reasons.append("hot_patch")

    # Counts (always, even if skipping — useful for diagnostics).
    call_count = len(RE_CALL.findall(asm_text))
    branch_count = len(RE_BRANCH.findall(asm_text))
    jmp_count = len(RE_JMP.findall(asm_text))

    # If too many transfers, it's a complex control-flow function.
    if call_count > 3:
        skip_reasons.append("many_calls")
    if branch_count > 4:
        skip_reasons.append("many_branches")

    # Simplicity score: bigger = harder; calls/branches add allocator
    # state sensitivity.
    score = 100
    score -= size                # 1 point per byte
    score -= call_count * 8      # each CALL constrains register schedule
    score -= branch_count * 3    # each conditional branch is a test+jcc decision
    score -= jmp_count * 2       # unconditional JMPs are usually epilogue jumps
    # Bonus: very tiny functions (8-30 B) are usually accessors/setters
    # that match almost mechanically.
    if 8 <= size <= 30:
        score += 15

    return {
        "name": name,
        "size": size,
        "calls": call_count,
        "branches": branch_count,
        "jmps": jmp_count,
        "score": score,
        "skip": skip_reasons,
    }


def parse_asm_file(path: Path) -> dict | None:
    """Parse one asm dump file. Returns the classification result, or
    None if the header is malformed."""
    text = path.read_text(errors="replace")
    rva_m = RE_HEADER_RVA.search(text)
    size_m = RE_HEADER_SIZE.search(text)
    name_m = RE_HEADER_NAME.search(text)
    if not (rva_m and size_m and name_m):
        return None
    rva = int(rva_m.group(1), 16)
    size = int(size_m.group(1), 16)
    name = name_m.group(1).strip()
    result = classify(text, name, size)
    result["rva"] = rva
    result["rva_hex"] = f"{rva:#010x}"
    result["asm"] = path.name
    return result


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", default="ffxivgame", nargs="?")
    ap.add_argument("--top", type=int, default=50, help="how many top candidates to dump (default 50)")
    ap.add_argument("--include-skipped", action="store_true", help="include skipped functions in the output (for diagnostics)")
    args = ap.parse_args()
    stem = args.binary.replace(".exe", "")

    asm_dir = ASM_ROOT / stem
    if not asm_dir.is_dir():
        print(f"error: {asm_dir} not found — run `make split BINARY={stem}.exe` first", file=sys.stderr)
        return 1

    OUT_ROOT.mkdir(parents=True, exist_ok=True)

    # Walk asm dumps.
    all_results: list[dict] = []
    skip_reasons_count: dict[str, int] = {}
    n_processed = 0
    for asm_path in sorted(asm_dir.glob("*.s")):
        result = parse_asm_file(asm_path)
        if result is None:
            continue
        n_processed += 1
        for r in result["skip"]:
            skip_reasons_count[r] = skip_reasons_count.get(r, 0) + 1
        all_results.append(result)

    # Cross-reference YAML status to skip already-matched functions.
    matched_rvas: set[int] = set()
    yaml_path = CONFIG_DIR / f"{stem}.yaml"
    if yaml_path.exists():
        # Streaming text scan — same shape as compare.py's YAML lookup.
        current_rva: int | None = None
        for line in yaml_path.read_text().splitlines():
            if line.startswith("- rva:"):
                rva_str = line.split(":", 1)[1].split("#", 1)[0].strip()
                try:
                    current_rva = int(rva_str, 0)
                except ValueError:
                    current_rva = None
            elif current_rva is not None and line.strip().startswith("status:"):
                status = line.split(":", 1)[1].split("#", 1)[0].strip()
                if status in ("matched", "partial"):
                    matched_rvas.add(current_rva)

    # Filter: keep matchable, sort by score descending.
    candidates = [
        r for r in all_results
        if not r["skip"] and r["rva"] not in matched_rvas
    ]
    candidates.sort(key=lambda r: -r["score"])

    # Write the JSON queue (top N).
    top = candidates[: args.top]
    queue_path = OUT_ROOT / f"{stem}.queue.json"
    queue_path.write_text(json.dumps(top, indent=2) + "\n")

    # Write a human-readable report.
    report_path = OUT_ROOT / f"{stem}.report.md"
    with report_path.open("w") as f:
        f.write(f"# {stem}.exe — easy-win candidates\n\n")
        f.write(f"Auto-generated by `tools/find_easy_wins.py`.\n\n")
        f.write(f"## Triage summary\n\n")
        f.write(f"- functions analysed: **{n_processed:,}**\n")
        f.write(f"- already matched (status=matched|partial): **{len(matched_rvas):,}**\n")
        f.write(f"- candidates passing all filters: **{len(candidates):,}**\n")
        f.write(f"- top {args.top} written to `{queue_path.relative_to(REPO_ROOT)}`\n\n")
        f.write(f"### Skip-reason histogram\n\n")
        f.write("| reason | count | %% of total |\n|---|---:|---:|\n")
        total = max(n_processed, 1)
        for reason, n in sorted(skip_reasons_count.items(), key=lambda kv: -kv[1]):
            f.write(f"| `{reason}` | {n:,} | {100*n/total:.1f}%% |\n")
        f.write(f"\n## Top {args.top} candidates\n\n")
        f.write("Ranked by score = `100 - size - 8*calls - 3*branches - 2*jmps + tiny_bonus`.\n\n")
        f.write("| rank | rva | name | size | calls | brn | jmp | score |\n")
        f.write("|---:|---|---|---:|---:|---:|---:|---:|\n")
        for i, r in enumerate(top, 1):
            f.write(f"| {i} | `{r['rva_hex']}` | `{r['name']}` | "
                    f"{r['size']} | {r['calls']} | {r['branches']} | "
                    f"{r['jmps']} | {r['score']} |\n")
        if args.include_skipped:
            skipped = [r for r in all_results if r["skip"]]
            f.write(f"\n## Skipped (sample of 50)\n\n")
            f.write("| rva | name | size | reasons |\n|---|---|---:|---|\n")
            for r in skipped[:50]:
                f.write(f"| `{r['rva_hex']}` | `{r['name']}` | {r['size']} | "
                        f"`{', '.join(r['skip'])}` |\n")

    print(f"wrote: {queue_path.relative_to(REPO_ROOT)}  ({len(top)} candidates)")
    print(f"wrote: {report_path.relative_to(REPO_ROOT)}  (full triage report)")
    print(f"  analysed: {n_processed:,}  already-matched: {len(matched_rvas)}  "
          f"survived filters: {len(candidates):,}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
