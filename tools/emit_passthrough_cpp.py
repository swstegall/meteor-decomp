#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Emit a `__declspec(naked)` C++ source file that compiles to an .obj
whose `.text` section is byte-identical to one function's bytes in the
original binary.

This is the "fallback" path for any function we haven't matched at
source-level: it gets a tiny .cpp under `src/<bin>/_passthrough/<sym>.cpp`
that simply re-emits the orig instruction bytes via MASM-style
`_emit 0xNN` directives. The compiled .obj's `.text` matches orig
byte-for-byte (modulo nothing — there are no relocations because the
bytes are emitted as immediates), so `tools/compare.py` reports GREEN
and the YAML row gets stamped `matched`.

Caveats:
  - **Linking these passthrough .objs into a runnable PE requires
    placing each function at its orig RVA.** Their CALL/JMP/MOV-imm32
    bytes encode absolute or PC-relative targets that are only valid
    at the orig RVA. The companion (future) linker pipeline uses
    `#pragma code_seg(".text$NNNNNNNN")` per RVA + `link.exe /MERGE`
    to lay sections out in sorted RVA order; this tool emits that
    pragma so the .obj is already wired for that strategy.
  - Naked functions can't have C++ exception handling, automatic
    locals, or default calling conventions — but they can have ANY
    body the assembler accepts. We emit only `_emit` (raw byte) and
    no actual instructions, which is universally accepted.
  - This tool is INTENTIONALLY brain-dead. It does not know what the
    function does. It does not produce readable source. It just
    guarantees the bytes round-trip. Source-level matching (the
    `_rosetta/<sym>.cpp` files) is always preferable when available;
    `_passthrough/<sym>.cpp` is the universal fallback so we can
    relink the binary even before every function has a hand-written
    source equivalent.

Usage:
  tools/emit_passthrough_cpp.py FUN_004165e0
  tools/emit_passthrough_cpp.py 0x000165e0                 # bare RVA
  tools/emit_passthrough_cpp.py FUN_004165e0 --binary ffxivlogin
  tools/emit_passthrough_cpp.py --all                      # every still-unmatched fn
  tools/emit_passthrough_cpp.py --all --binary ffxivlogin
  tools/emit_passthrough_cpp.py --all --max 100            # cap output (smoke-test)

Reads:
  config/<binary>.yaml                — work pool (per-function rva, size, status)
  build/pe-layout/<binary>.json       — section table for orig→file-offset mapping
  orig/<binary>.exe                   — orig bytes (symlinked, not committed)

Writes:
  src/<binary>/_passthrough/<sym>.cpp — one per function emitted
"""

from __future__ import annotations

import argparse
import json
import re
import struct
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CONFIG_DIR = REPO_ROOT / "config"
SRC_ROOT = REPO_ROOT / "src"
ORIG_ROOT = REPO_ROOT / "orig"
PE_LAYOUT_ROOT = REPO_ROOT / "build" / "pe-layout"

KNOWN_BINARIES = ("ffxivgame", "ffxivlogin", "ffxivboot", "ffxivconfig", "ffxivupdater")

LICENSE_HEADER = """\
// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SPDX-License-Identifier: AGPL-3.0-or-later
"""


# ----------------------------------------------------------------------
# YAML parsing — streaming, since the YAML is ~1M lines.
# ----------------------------------------------------------------------

RE_YAML_HEADER = re.compile(r"^- rva: (0x[0-9a-fA-F]+)")


def stream_yaml_rows(yaml_path: Path):
    """Yield (rva, end, size, symbol, status) per row. Streaming parser."""
    rva = end = size = None
    symbol = None
    status = None
    with yaml_path.open() as f:
        for line in f:
            m = RE_YAML_HEADER.match(line)
            if m:
                if rva is not None:
                    yield (rva, end, size, symbol, status)
                rva = int(m.group(1), 16)
                end = size = None
                symbol = None
                status = None
                continue
            stripped = line.strip()
            if stripped.startswith("end:"):
                end = int(stripped.split(":", 1)[1].split("#", 1)[0].strip(), 0)
            elif stripped.startswith("size:"):
                size = int(stripped.split(":", 1)[1].split("#", 1)[0].strip(), 0)
            elif stripped.startswith("symbol:"):
                symbol = stripped.split(":", 1)[1].split("#", 1)[0].strip()
            elif stripped.startswith("status:"):
                status = stripped.split(":", 1)[1].split("#", 1)[0].strip()
    if rva is not None:
        yield (rva, end, size, symbol, status)


# ----------------------------------------------------------------------
# PE layout — image base + sections, for RVA → file offset mapping.
# ----------------------------------------------------------------------


def read_pe_layout(binary: str) -> tuple[int, list[dict]]:
    p = PE_LAYOUT_ROOT / f"{binary}.json"
    data = json.loads(p.read_text())
    return int(data["image_base"], 16), data["sections"]


def rva_to_file_off(rva: int, sections: list[dict]) -> int | None:
    for s in sections:
        va = s["virtual_address"]
        rsize = s["raw_size"]
        if va <= rva < va + rsize:
            return s["raw_pointer"] + (rva - va)
    return None


# ----------------------------------------------------------------------
# Emit one passthrough .cpp.
# ----------------------------------------------------------------------


def emit_passthrough_cpp(symbol: str, rva: int, body: bytes, image_base: int,
                         binary: str) -> str:
    """Generate the .cpp text for a single function passthrough."""
    va = rva + image_base
    out: list[str] = []
    out.append(LICENSE_HEADER)
    out.append("//")
    out.append(f"// PASSTHROUGH: {binary}.exe @ rva 0x{rva:08x} (va 0x{va:08x})")
    out.append(f"//   symbol: {symbol}")
    out.append(f"//   size:   {len(body)} bytes")
    out.append("//")
    out.append("// Auto-generated by tools/emit_passthrough_cpp.py — copies orig")
    out.append("// instruction bytes verbatim via MASM `_emit` directives. No")
    out.append("// claim is made about what this function does; replace this")
    out.append("// passthrough with a hand-written source under _rosetta/ when")
    out.append("// the function is properly decompiled.")
    out.append("//")
    out.append("// IMPORTANT: this passthrough only links correctly when placed")
    out.append("// at orig RVA — embedded CALL rel32 / MOV imm32 bytes assume")
    out.append("// the function lives at its original load address. The")
    out.append("// `code_seg(\".text$NNNNNNNN\")` pragma below names the section")
    out.append("// so a future linker driver can sort by RVA via `/MERGE`.")
    out.append("")
    # Section name: ".text$<RVA hex 8 digit>" so link.exe sorts by RVA.
    out.append(f'#pragma code_seg(".text$X{rva:08x}")')
    out.append("")
    out.append(f'extern "C" __declspec(naked) void {symbol}() {{')
    out.append("    __asm {")
    # Emit one byte per line for readability; group 16 per visual block.
    for i, b in enumerate(body):
        if i and i % 16 == 0:
            out.append("")
        out.append(f"        _emit 0x{b:02x}")
    out.append("    }")
    out.append("}")
    out.append("")
    out.append('#pragma code_seg()')
    out.append("")
    return "\n".join(out)


# ----------------------------------------------------------------------
# Driver.
# ----------------------------------------------------------------------


def parse_target(target: str) -> tuple[str | None, int | None]:
    """Parse a CLI target as either FUN_<va> or 0x<rva> or <rva>.

    Returns (symbol, rva). Symbol may be None if user gave a raw RVA;
    rva may be None if user gave a non-FUN_ symbol."""
    if target.startswith("FUN_"):
        m = re.match(r"FUN_([0-9a-fA-F]+)$", target)
        if m:
            va = int(m.group(1), 16)
            return target, va  # caller subtracts image_base
        return target, None
    # Raw hex RVA.
    try:
        rva = int(target, 0)
        return None, rva
    except ValueError:
        return target, None


def find_yaml_row(yaml_path: Path, rva: int) -> tuple[int, int, str, str] | None:
    """Look up (rva, size, symbol, status) for a specific RVA. Streams the YAML."""
    for r, _end, sz, sym, st in stream_yaml_rows(yaml_path):
        if r == rva:
            return (r, sz, sym, st)
        if r > rva:
            return None
    return None


def emit_one(binary: str, target: str, image_base: int, sections: list[dict],
             orig_bytes: bytes, yaml_path: Path, dry_run: bool, force: bool) -> str:
    sym, val = parse_target(target)
    if sym and sym.startswith("FUN_") and val is not None:
        # FUN_<absolute va> → derive RVA.
        rva = val - image_base
    elif val is not None:
        # Raw RVA.
        rva = val
        sym = None
    else:
        return f"error: can't parse target '{target}'"
    row = find_yaml_row(yaml_path, rva)
    if not row:
        return f"error: no YAML row at rva 0x{rva:08x}"
    rva_y, size, sym_y, status = row
    if sym is None:
        sym = sym_y
    file_off = rva_to_file_off(rva, sections)
    if file_off is None:
        return f"error: rva 0x{rva:08x} not in any section"
    body = orig_bytes[file_off:file_off + size]
    if len(body) != size:
        return f"error: short read at rva 0x{rva:08x}: got {len(body)}/{size}"
    out_dir = SRC_ROOT / binary / "_passthrough"
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / f"{sym}.cpp"
    if out_path.exists() and not force:
        return f"skip exists: {out_path}"
    src = emit_passthrough_cpp(sym, rva, body, image_base, binary)
    if not dry_run:
        out_path.write_text(src)
    return f"emit {out_path} ({size} B)"


def cmd_all(binary: str, image_base: int, sections: list[dict],
            orig_bytes: bytes, yaml_path: Path, dry_run: bool, force: bool,
            max_count: int | None, status_filter: set[str],
            order_by_size_desc: bool = False,
            include_rosetta: bool = False) -> dict:
    """When `include_rosetta=False` (default), skip functions that
    already have a `_rosetta/<sym>.cpp` — preserves source-level
    decomp as the canonical match.

    When `include_rosetta=True`, emit a `_passthrough/FUN_<va>.cpp`
    for every YAML row in the status filter EVEN IF a rosetta source
    exists. Use this when you need a complete `.obj` inventory at
    every RVA for the link step — rosetta `.objs` lack the
    `.text$X<rva>` section pragma so they don't land at the orig
    RVA on their own.
    """
    rosetta_dir = SRC_ROOT / binary / "_rosetta"
    out_dir = SRC_ROOT / binary / "_passthrough"
    out_dir.mkdir(parents=True, exist_ok=True)

    n_emitted = 0
    n_skipped_existing = 0
    n_skipped_status = 0
    n_skipped_section = 0
    n_skipped_rosetta = 0
    n_errors = 0

    # Optionally walk YAML rows in size-descending order (biggest fns
    # first). Combined with --max this lets us prioritise high-byte-yield
    # passthroughs without committing to the full 70k for ffxivgame.
    if order_by_size_desc:
        rows = sorted(
            (
                (r, e, sz, sym, st)
                for r, e, sz, sym, st in stream_yaml_rows(yaml_path)
                if st in status_filter and sz is not None
            ),
            key=lambda t: -t[2],
        )
    else:
        rows = stream_yaml_rows(yaml_path)

    for rva, _end, size, symbol, status in rows:
        if status not in status_filter:
            n_skipped_status += 1
            continue
        # Output filename keyed on RVA so common symbol names that
        # collide across the binary (`scalar_deleting_destructor`,
        # `QueryInterface`, etc.) each get their own file.
        rva_va = rva + image_base
        fun_name = f"FUN_{rva_va:08x}"
        # Don't overwrite an existing _rosetta source unless the caller
        # opts in via --include-rosetta. Default skip preserves
        # source-level decomp as the canonical match. The opt-in path
        # is for the link step (every RVA needs a .text$X<rva>-pragma'd
        # .obj; rosetta sources don't have that).
        if not include_rosetta:
            rosetta_candidates = [
                rosetta_dir / f"{symbol}.cpp",
                rosetta_dir / f"{fun_name}.cpp",
            ]
            if any(p.exists() for p in rosetta_candidates):
                n_skipped_rosetta += 1
                continue
        out_path = out_dir / f"{fun_name}.cpp"
        if out_path.exists() and not force:
            n_skipped_existing += 1
            continue
        file_off = rva_to_file_off(rva, sections)
        if file_off is None:
            n_skipped_section += 1
            continue
        body = orig_bytes[file_off:file_off + size]
        if len(body) != size:
            n_errors += 1
            continue
        src = emit_passthrough_cpp(fun_name, rva, body, image_base, binary)
        if not dry_run:
            out_path.write_text(src)
        n_emitted += 1
        if max_count and n_emitted >= max_count:
            break
    return {
        "binary": binary,
        "emitted": n_emitted,
        "skipped_existing_passthrough": n_skipped_existing,
        "skipped_existing_rosetta": n_skipped_rosetta,
        "skipped_status": n_skipped_status,
        "skipped_no_section": n_skipped_section,
        "errors": n_errors,
    }


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("targets", nargs="*", help="FUN_<va> or 0x<rva> targets to emit one-by-one")
    ap.add_argument("--binary", default="ffxivgame", choices=KNOWN_BINARIES)
    ap.add_argument("--all", action="store_true", help="Emit passthroughs for every still-unmatched function")
    ap.add_argument("--status", action="append", default=None,
                    help="Status filter for --all (default: unmatched). Repeatable.")
    ap.add_argument("--max", type=int, default=None, help="Cap --all output count (for smoke-test runs)")
    ap.add_argument("--biggest-first", action="store_true",
                    help="With --all + --max, emit largest functions first (highest byte yield per .cpp)")
    ap.add_argument("--include-rosetta", action="store_true",
                    help="With --all, emit passthroughs even for functions that have a _rosetta/<sym>.cpp. Use for link prep (every RVA needs a .text$X<rva>-pragma'd .obj).")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--force", action="store_true", help="Overwrite existing _passthrough .cpp")
    args = ap.parse_args()

    binary = args.binary
    yaml_path = CONFIG_DIR / f"{binary}.yaml"
    if not yaml_path.exists():
        print(f"error: {yaml_path} not found — run `make split BINARY={binary}.exe`", file=sys.stderr)
        return 1
    image_base, sections = read_pe_layout(binary)
    orig_path = ORIG_ROOT / f"{binary}.exe"
    if not orig_path.exists():
        print(f"error: {orig_path} not found — run `make bootstrap`", file=sys.stderr)
        return 1
    orig_bytes = orig_path.read_bytes()

    if args.all:
        # When --include-rosetta is set, also flip the default status
        # filter to include `matched` rows (otherwise we'd skip them
        # all since most rosetta'd rows have status=matched).
        if args.status:
            status_filter = set(args.status)
        elif args.include_rosetta:
            status_filter = {"unmatched", "matched", "passthrough"}
        else:
            status_filter = {"unmatched"}
        stats = cmd_all(binary, image_base, sections, orig_bytes, yaml_path,
                        args.dry_run, args.force, args.max, status_filter,
                        order_by_size_desc=args.biggest_first,
                        include_rosetta=args.include_rosetta)
        print(f"=== passthrough emit ({binary}) ===")
        for k, v in stats.items():
            print(f"  {k}: {v}")
        return 0

    if not args.targets:
        ap.error("either pass targets or --all")
    for t in args.targets:
        msg = emit_one(binary, t, image_base, sections, orig_bytes, yaml_path,
                       args.dry_run, args.force)
        print(msg)
    return 0


if __name__ == "__main__":
    sys.exit(main())
