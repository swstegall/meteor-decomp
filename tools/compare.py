#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Byte-level matching diff for one Rosetta-Stone-style function.

Compares the `.text` section of `build/obj/_rosetta/<FUNC>.obj`
(produced by `make rosetta`) against the corresponding bytes in
`orig/ffxivgame.exe` at the function's known RVA. Reports:

  - exact-byte match percentage,
  - side-by-side hex view of any mismatching window,
  - first mismatch offset (so the contributor can navigate
    Ghidra/asm and see exactly which instruction pair diverged),
  - one-line GREEN / PARTIAL / MISMATCH verdict so callers
    (Makefile, CI) can branch on it.

Usage:
  tools/compare.py FUNC=FUN_00b361b0
  tools/compare.py FUNC=Blowfish::Init        # future, when we have
                                              # symbol-name-keyed
                                              # candidates

Resolution rules:
  - The FUNC value is the basename of `src/ffxivgame/_rosetta/<FUNC>.cpp`
    AND the basename of `build/obj/_rosetta/<FUNC>.obj`.
  - For `FUN_<absolute-address>` candidates, the RVA is parsed from the
    name directly (`FUN_00b361b0` → absolute 0x00b361b0 → RVA = abs -
    image_base where image_base = 0x400000 from build/pe-layout/<bin>.json).
  - For symbol-name candidates we'd need a name → RVA map; for now
    only `FUN_<addr>` style names work.

Exit codes:
  0  GREEN — exact byte-for-byte match.
  1  PARTIAL — same length, some bytes differ.
  2  MISMATCH — different lengths or completely off.
  3  USAGE / SETUP error.
"""

from __future__ import annotations

import argparse
import json
import re
import struct
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

# Defaults (single-binary repo; future multi-binary support would parameterise).
ORIG_PE = REPO_ROOT / "orig" / "ffxivgame.exe"
CONFIG_DIR = REPO_ROOT / "config"
PE_LAYOUT = REPO_ROOT / "build" / "pe-layout" / "ffxivgame.json"
OBJ_ROOT = REPO_ROOT / "build" / "obj" / "_rosetta"


def _parse_func_arg(arg: str) -> str:
    """Accept either bare `FUN_xxxx` or `FUNC=FUN_xxxx`."""
    if "=" in arg:
        return arg.split("=", 1)[1]
    return arg


def _abs_from_fun_name(name: str) -> int | None:
    m = re.match(r"FUN_([0-9a-fA-F]+)$", name)
    return int(m.group(1), 16) if m else None


def _read_pe_layout() -> tuple[int, list[dict]]:
    """Parse build/pe-layout/<bin>.json (regenerable via tools/extract_pe.py)
    and return (image_base, sections) where sections is a list of dicts
    matching the JSON schema."""
    if not PE_LAYOUT.exists():
        print(f"error: {PE_LAYOUT} missing — run tools/extract_pe.py first", file=sys.stderr)
        sys.exit(3)
    data = json.loads(PE_LAYOUT.read_text())
    image_base = int(data["image_base"], 16)
    return image_base, data["sections"]


def _rva_to_file_offset(rva: int, sections: list[dict]) -> int | None:
    for s in sections:
        sec_va = s["virtual_address"]
        sec_vsize = s["virtual_size"]
        sec_raw = s["raw_pointer"]
        sec_rsize = s["raw_size"]
        if sec_va <= rva < sec_va + max(sec_vsize, sec_rsize):
            return sec_raw + (rva - sec_va)
    return None


def _read_orig_bytes(rva: int, size: int) -> bytes:
    image_base, sections = _read_pe_layout()
    file_off = _rva_to_file_offset(rva, sections)
    if file_off is None:
        print(f"error: rva {rva:#x} not in any section", file=sys.stderr)
        sys.exit(3)
    with ORIG_PE.open("rb") as f:
        f.seek(file_off)
        return f.read(size)


def _coff_text_bytes(obj_path: Path) -> bytes:
    """Parse a COFF .obj and return the raw bytes of its `.text` section."""
    data = obj_path.read_bytes()
    # COFF file header: machine(2) num_sections(2) timestamp(4) sym_off(4)
    # num_syms(4) opt_size(2) characteristics(2) — total 20 bytes.
    n_sections = struct.unpack_from("<H", data, 2)[0]
    opt_size = struct.unpack_from("<H", data, 16)[0]
    sec_off = 20 + opt_size
    for i in range(n_sections):
        base = sec_off + i * 40
        name = data[base : base + 8].rstrip(b"\0").decode("ascii", errors="replace")
        if name != ".text":
            continue
        raw_size = struct.unpack_from("<I", data, base + 16)[0]
        raw_ptr = struct.unpack_from("<I", data, base + 20)[0]
        return data[raw_ptr : raw_ptr + raw_size]
    raise RuntimeError(f"no .text section in {obj_path}")


def _hex_dump(b: bytes, width: int = 16) -> list[str]:
    out: list[str] = []
    for i in range(0, len(b), width):
        chunk = b[i : i + width]
        out.append(" ".join(f"{x:02x}" for x in chunk))
    return out


def _side_by_side(orig: bytes, ours: bytes, width: int = 16) -> str:
    rows: list[str] = []
    rows.append(f"  {'offset':>6}  {'orig (binary)':<{width*3}}  ours (cl.exe)")
    n = max(len(orig), len(ours))
    diffs = 0
    for i in range(0, n, width):
        o_chunk = orig[i : i + width]
        u_chunk = ours[i : i + width]
        o_hex = _hex_dump(o_chunk)[0] if o_chunk else ""
        u_hex = _hex_dump(u_chunk)[0] if u_chunk else ""
        # Mark mismatching bytes with '*'
        marker_chars = []
        for j in range(width):
            o = o_chunk[j] if j < len(o_chunk) else None
            u = u_chunk[j] if j < len(u_chunk) else None
            if o is None or u is None:
                marker_chars.append("?")
                diffs += 1
            elif o != u:
                marker_chars.append("*")
                diffs += 1
            else:
                marker_chars.append(".")
        marker = " ".join(marker_chars[: max(len(o_chunk), len(u_chunk))])
        rows.append(f"  {i:>06x}  {o_hex:<{width*3}}  {u_hex}")
        rows.append(f"  {'':>6}  {marker:<{width*3}}")
    return "\n".join(rows)


def _verdict(orig: bytes, ours: bytes) -> tuple[str, int, int]:
    """Return (verdict, mismatched_bytes, total_orig_bytes)."""
    if len(orig) == len(ours) and orig == ours:
        return ("GREEN", 0, len(orig))
    if len(orig) != len(ours):
        return ("MISMATCH", -1, len(orig))
    diffs = sum(1 for a, b in zip(orig, ours) if a != b)
    return ("PARTIAL", diffs, len(orig))


def _first_mismatch(orig: bytes, ours: bytes) -> int | None:
    for i, (a, b) in enumerate(zip(orig, ours)):
        if a != b:
            return i
    if len(orig) != len(ours):
        return min(len(orig), len(ours))
    return None


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("func", nargs="?", help="FUNC name or FUNC=name")
    ap.add_argument("rest", nargs="*", help=argparse.SUPPRESS)
    args = ap.parse_args()

    if not args.func:
        print("usage: tools/compare.py FUNC=<symbol-name>", file=sys.stderr)
        return 3
    # Accept the Makefile shape: `compare.py FUNC=<name>`.
    func_name = _parse_func_arg(args.func) or _parse_func_arg((args.rest or [""])[0])
    if not func_name:
        print("usage: tools/compare.py FUNC=<symbol-name>", file=sys.stderr)
        return 3

    abs_addr = _abs_from_fun_name(func_name)
    if abs_addr is None:
        print(f"error: only FUN_<address> names supported today, got {func_name!r}", file=sys.stderr)
        return 3

    image_base, _ = _read_pe_layout()
    rva = abs_addr - image_base

    sym_path = CONFIG_DIR / "ffxivgame.symbols.json"
    if not sym_path.exists():
        print(f"error: {sym_path} missing — run `make split BINARY=ffxivgame.exe`", file=sys.stderr)
        return 3
    syms = json.loads(sym_path.read_text())
    sym = next((s for s in syms if s["rva"] == rva), None)
    if sym is None:
        print(f"error: rva {rva:#x} not in symbols.json (out of date?)", file=sys.stderr)
        return 3
    size = int(sym["size"])

    obj_path = OBJ_ROOT / f"{func_name}.obj"
    if not obj_path.exists():
        print(f"error: {obj_path} missing — run `make rosetta` first", file=sys.stderr)
        return 3

    orig = _read_orig_bytes(rva, size)
    ours = _coff_text_bytes(obj_path)
    verdict, diffs, total = _verdict(orig, ours)

    print(f"=== {func_name} (rva {rva:#x}, size {size} B) ===")
    print(f"  orig: {len(orig)} bytes from {ORIG_PE.relative_to(REPO_ROOT)}")
    print(f"  ours: {len(ours)} bytes from {obj_path.relative_to(REPO_ROOT)}")
    print()

    if verdict == "GREEN":
        print(f"  ✅ GREEN — byte-identical ({total} of {total} bytes match)")
        return 0

    if verdict == "MISMATCH":
        print(f"  ❌ MISMATCH — sizes differ (orig={len(orig)}, ours={len(ours)})")
        delta = len(ours) - len(orig)
        print(f"     ours is {abs(delta)} byte{'s' if abs(delta)!=1 else ''} {'longer' if delta>0 else 'shorter'}")
    else:
        pct = 100.0 * (total - diffs) / total if total else 0.0
        print(f"  🟡 PARTIAL — {total - diffs} of {total} bytes match ({pct:.1f}%)")

    fm = _first_mismatch(orig, ours)
    if fm is not None:
        print(f"  first mismatch at offset {fm:#x} (file rva {rva + fm:#x})")
    print()
    print(_side_by_side(orig, ours))
    print()
    print("Iteration tips: see docs/matching-workflow.md §7. Common knobs:")
    print("  - register-allocation drift: reorder local declarations in the .cpp")
    print("  - branch direction (JBE vs JLE): switch signed/unsigned types or")
    print("    invert if-conditions to flip the natural code path")
    print("  - frame-pointer omission (/Oy- vs /Oy): adjust ROSETTA_FLAGS")
    print("  - one-pointer vs two-pointer addressing: split / merge locals")

    return 1 if verdict == "PARTIAL" else 2


if __name__ == "__main__":
    sys.exit(main())
