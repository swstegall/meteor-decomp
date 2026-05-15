#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Emit the inter-function `.text` gap bytes as `.text$X<rva>` subsections
inside a single .cpp file, so the linker can re-stitch the orig `.text`
section byte-for-byte.

Why: Ghidra-detected functions don't cover 100% of `.text`. The gaps
between `function.end` and `next_function.rva` are typically 0xCC
padding (~65% of gaps), but ~34% contain code/data Ghidra missed —
truncated function tails, jump tables, or inline data. To produce a
byte-identical re-link we need to put SOMETHING at every byte-offset.

This tool walks `config/<binary>.yaml`, scans for gaps within `.text`,
and emits one `.text$X<rva>` subsection per gap. Each subsection is
a naked-asm function (zero-arg, returns) wrapping the gap bytes via
`_emit`. The function never gets called — its purpose is purely to
contribute bytes at the right linker-controlled RVA.

Output: `src/<binary>/_passthrough/_text_gaps.cpp` (single file with
N gap-subsections). Suffix `.text$X<rva>` keeps lexicographic ordering
consistent with the per-function passthroughs.

Reads:
  config/<binary>.yaml         — per-function rva/end/size/section
  build/pe-layout/<binary>.json — section table for `.text` boundaries
  orig/<binary>.exe            — raw bytes for each gap region

Writes:
  src/<binary>/_passthrough/_text_gaps.cpp
  build/wire/<binary>.text_gaps.md   — human-readable gap manifest

Usage:
  tools/emit_text_gaps.py ffxivlogin
  tools/emit_text_gaps.py ffxivlogin --dry-run

Notes:
  - Each gap-bytes block is wrapped in a no-arg
    `extern "C" __declspec(naked) void _gap_<rva>()` function so we
    can also place a label that link.exe / dumpbin recognises.
  - The cl.exe optimizer would normally reorder + drop unreferenced
    naked functions; we set `#pragma comment(linker, "/INCLUDE:_gap_<rva>")`
    so each gap stays alive in the link.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CONFIG = REPO_ROOT / "config"
SRC = REPO_ROOT / "src"
WIRE = REPO_ROOT / "build" / "wire"
PE_LAYOUT = REPO_ROOT / "build" / "pe-layout"
ORIG = REPO_ROOT / "orig"

ALL_BINARIES = ("ffxivboot", "ffxivconfig", "ffxivgame", "ffxivlogin", "ffxivupdater")

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
# Streaming YAML reader (matches emit_passthrough_cpp.py).
# ----------------------------------------------------------------------

RE_YAML_HEADER = re.compile(r"^- rva: (0x[0-9a-fA-F]+)")


def stream_yaml_rows(yaml_path: Path):
    rva = end = size = None
    section = None
    with yaml_path.open() as f:
        for line in f:
            m = RE_YAML_HEADER.match(line)
            if m:
                if rva is not None:
                    yield (rva, end, size, section)
                rva = int(m.group(1), 16)
                end = size = section = None
                continue
            stripped = line.strip()
            if stripped.startswith("end:"):
                end = int(stripped.split(":", 1)[1].split("#", 1)[0].strip(), 0)
            elif stripped.startswith("size:"):
                size = int(stripped.split(":", 1)[1].split("#", 1)[0].strip(), 0)
            elif stripped.startswith("section:"):
                section = stripped.split(":", 1)[1].split("#", 1)[0].strip()
    if rva is not None:
        yield (rva, end, size, section)


# ----------------------------------------------------------------------
# PE layout helpers.
# ----------------------------------------------------------------------


def read_pe_layout(binary: str) -> tuple[int, list[dict]]:
    p = PE_LAYOUT / f"{binary}.json"
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
# Gap discovery.
# ----------------------------------------------------------------------


def find_text_gaps(binary: str) -> list[tuple[int, int]]:
    """Return [(rva, length)] for every gap inside the .text section."""
    yaml_path = CONFIG / f"{binary}.yaml"
    image_base, sections = read_pe_layout(binary)
    text_section = next(s for s in sections if s["name"] == ".text")
    text_va = text_section["virtual_address"]
    # Note: virtual_size is the in-memory size; raw_size is the on-disk
    # size (rounded up to file-alignment, often containing trailing
    # zero padding). For "every byte the loader sees" we use vsize;
    # for "every byte in the .text on disk" we use rsize. orig PEs
    # use rsize for layout, so use raw_size to keep the relink
    # symmetric.
    text_end = text_va + text_section["raw_size"]

    rows = sorted(
        ((rva, end) for rva, end, _sz, sec in stream_yaml_rows(yaml_path)
         if sec == ".text" and end is not None),
        key=lambda t: t[0],
    )
    if not rows:
        return []

    gaps: list[tuple[int, int]] = []
    cursor = text_va
    for rva, end in rows:
        if rva > cursor:
            gap_len = rva - cursor
            if gap_len > 0:
                gaps.append((cursor, gap_len))
        cursor = max(cursor, end)
    if cursor < text_end:
        gaps.append((cursor, text_end - cursor))
    return gaps


# ----------------------------------------------------------------------
# Code emission.
# ----------------------------------------------------------------------


def emit_gap_subsection(rva: int, body: bytes) -> str:
    """One `.text$X<rva>` subsection: pragma + naked function + bytes.

    Symbol name is `_gap_<rva>` in C source; MSVC's cdecl name
    decoration prepends another `_`, giving the linker symbol
    `__gap_<rva>`. The /INCLUDE directive must use the linker form.
    """
    lines = []
    lines.append(f'#pragma code_seg(".text$X{rva:08x}")')
    lines.append(f'#pragma comment(linker, "/INCLUDE:__gap_{rva:08x}")')
    lines.append(f'extern "C" __declspec(naked) void _gap_{rva:08x}() {{')
    lines.append('    __asm {')
    for i, b in enumerate(body):
        if i and i % 16 == 0:
            lines.append('')
        lines.append(f'        _emit 0x{b:02x}')
    lines.append('    }')
    lines.append('}')
    lines.append('')
    return "\n".join(lines)


def emit_gaps_cpp(binary: str, gaps: list[tuple[int, int]],
                  orig_bytes: bytes, sections: list[dict]) -> str:
    out: list[str] = [LICENSE_HEADER, ""]
    out.append("// AUTO-GENERATED by tools/emit_text_gaps.py — do not edit.")
    out.append(f"// {binary}.exe inter-function .text gap manifest.")
    out.append(f"// gap count: {len(gaps)}")
    total = sum(g[1] for g in gaps)
    out.append(f"// total bytes: {total}")
    out.append("//")
    out.append("// Each gap is wrapped in a naked function whose entire body is")
    out.append("// `_emit` of the orig bytes. The function is never called; the")
    out.append("// `/INCLUDE:` linker directive keeps it alive so its")
    out.append("// `.text$X<rva>` subsection contributes bytes at the right")
    out.append("// offset when link.exe sorts subsections lexicographically.")
    out.append("")
    for rva, length in gaps:
        file_off = rva_to_file_off(rva, sections)
        if file_off is None:
            continue
        body = orig_bytes[file_off:file_off + length]
        if len(body) != length:
            continue
        out.append(emit_gap_subsection(rva, body))
    out.append('#pragma code_seg()')
    out.append('')
    return "\n".join(out)


# ----------------------------------------------------------------------
# Manifest report.
# ----------------------------------------------------------------------


def classify_gap(body: bytes) -> str:
    if not body:
        return "empty"
    unique = set(body)
    if unique == {0xcc}:
        return "all_int3"
    if unique == {0x90}:
        return "all_nop"
    if unique == {0x00}:
        return "all_zero"
    if unique <= {0x00, 0xcc, 0x90}:
        return "mixed_filler"
    return "code_or_data"


def emit_report(binary: str, gaps: list[tuple[int, int]],
                orig_bytes: bytes, sections: list[dict]) -> str:
    classes: dict[str, list[tuple[int, int]]] = {}
    for rva, length in gaps:
        file_off = rva_to_file_off(rva, sections)
        if file_off is None:
            continue
        body = orig_bytes[file_off:file_off + length]
        cls = classify_gap(body)
        classes.setdefault(cls, []).append((rva, length))

    out: list[str] = []
    out.append(f"# {binary}.exe — `.text` gap manifest\n")
    out.append("Auto-generated by `tools/emit_text_gaps.py`.\n")
    out.append("## Summary\n")
    out.append(f"- gap count: **{len(gaps)}**")
    out.append(f"- total bytes: **{sum(g[1] for g in gaps):,}**\n")
    out.append("| classification | gaps | bytes | example |")
    out.append("|---|---:|---:|---|")
    for cls in sorted(classes.keys()):
        gs = classes[cls]
        nbytes = sum(g[1] for g in gs)
        ex = gs[0]
        ex_off = rva_to_file_off(ex[0], sections)
        ex_bytes = orig_bytes[ex_off:ex_off + min(ex[1], 16)].hex() if ex_off else "?"
        out.append(f"| `{cls}` | {len(gs):,} | {nbytes:,} | rva 0x{ex[0]:08x} ({ex[1]} B): `{ex_bytes}` |")
    return "\n".join(out) + "\n"


# ----------------------------------------------------------------------
# Driver.
# ----------------------------------------------------------------------


def process(binary: str, dry_run: bool) -> dict:
    yaml_path = CONFIG / f"{binary}.yaml"
    if not yaml_path.exists():
        return {"binary": binary, "skipped": "yaml missing"}
    image_base, sections = read_pe_layout(binary)
    orig_bytes = (ORIG / f"{binary}.exe").read_bytes()
    gaps = find_text_gaps(binary)
    src = emit_gaps_cpp(binary, gaps, orig_bytes, sections)
    report = emit_report(binary, gaps, orig_bytes, sections)
    out_dir = SRC / binary / "_passthrough"
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / "_text_gaps.cpp"
    report_path = WIRE / f"{binary}.text_gaps.md"
    report_path.parent.mkdir(parents=True, exist_ok=True)
    if not dry_run:
        out_path.write_text(src)
        report_path.write_text(report)
    return {
        "binary": binary,
        "gaps": len(gaps),
        "total_bytes": sum(g[1] for g in gaps),
        "out": str(out_path.relative_to(REPO_ROOT)),
        "report": str(report_path.relative_to(REPO_ROOT)),
    }


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", nargs="?", help="binary stem (default: all five)")
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()
    binaries = (args.binary,) if args.binary else ALL_BINARIES
    print(f"=== emit_text_gaps (dry-run={args.dry_run}) ===")
    for stem in binaries:
        res = process(stem, args.dry_run)
        print(f"  {stem}: gaps={res.get('gaps','?')} bytes={res.get('total_bytes','?'):,} → {res.get('out','?')}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
