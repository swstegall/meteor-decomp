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
CONFIG_DIR = REPO_ROOT / "config"


def _binary_paths(binary_stem: str) -> tuple[Path, Path, Path]:
    """Return (orig_pe, pe_layout_json, obj_root) for a given binary stem.

    For ffxivgame the legacy non-suffixed obj_root is used (so existing
    builds continue to work); for any other binary the obj root is
    namespaced under the stem so multi-binary builds don't collide.
    """
    orig_pe = REPO_ROOT / "orig" / f"{binary_stem}.exe"
    pe_layout = REPO_ROOT / "build" / "pe-layout" / f"{binary_stem}.json"
    if binary_stem == "ffxivgame":
        # Backward-compatible: legacy obj root for ffxivgame.
        legacy = REPO_ROOT / "build" / "obj" / "_rosetta"
        nested = legacy / "ffxivgame"
        # Prefer nested (new layout) if it exists, else fall back.
        obj_root = nested if nested.is_dir() else legacy
    else:
        obj_root = REPO_ROOT / "build" / "obj" / "_rosetta" / binary_stem
    return orig_pe, pe_layout, obj_root


def _parse_kv_arg(arg: str, key: str) -> str | None:
    """Parse a `KEY=value` style argument; return value or None if not match."""
    if not arg:
        return None
    if "=" not in arg:
        return None
    k, v = arg.split("=", 1)
    return v if k == key else None


def _parse_func_arg(arg: str) -> str:
    """Accept either bare `FUN_xxxx` or `FUNC=FUN_xxxx`."""
    if "=" in arg:
        return arg.split("=", 1)[1]
    return arg


def _abs_from_fun_name(name: str) -> int | None:
    m = re.match(r"FUN_([0-9a-fA-F]+)$", name)
    return int(m.group(1), 16) if m else None


def _read_pe_layout(pe_layout_path: Path) -> tuple[int, list[dict]]:
    """Parse build/pe-layout/<bin>.json (regenerable via tools/extract_pe.py)
    and return (image_base, sections) where sections is a list of dicts
    matching the JSON schema."""
    if not pe_layout_path.exists():
        print(f"error: {pe_layout_path} missing — run tools/extract_pe.py first", file=sys.stderr)
        sys.exit(3)
    data = json.loads(pe_layout_path.read_text())
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


def _read_orig_bytes(rva: int, size: int, orig_pe: Path, pe_layout_path: Path) -> bytes:
    image_base, sections = _read_pe_layout(pe_layout_path)
    file_off = _rva_to_file_offset(rva, sections)
    if file_off is None:
        print(f"error: rva {rva:#x} not in any section", file=sys.stderr)
        sys.exit(3)
    with orig_pe.open("rb") as f:
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


def _coff_text_relocs(obj_path: Path) -> list[tuple[int, int]]:
    """Parse the COFF relocation table for `.text` and return a list of
    (offset, size) tuples giving the byte ranges occupied by linker
    fixups. Used to mask relocation bytes out of the byte-level diff —
    the .obj carries zeros at those positions while the orig binary
    carries the actual fixed-up value.

    All i386 COFF relocations cover 4 bytes (IMAGE_REL_I386_DIR32 = 6,
    IMAGE_REL_I386_REL32 = 0x14, IMAGE_REL_I386_DIR32NB = 7, plus
    section-relative variants that are 4 bytes wide too). Returning a
    flat (offset, 4) tuple per entry covers them uniformly."""
    data = obj_path.read_bytes()
    n_sections = struct.unpack_from("<H", data, 2)[0]
    opt_size = struct.unpack_from("<H", data, 16)[0]
    sec_off = 20 + opt_size
    for i in range(n_sections):
        base = sec_off + i * 40
        name = data[base : base + 8].rstrip(b"\0").decode("ascii", errors="replace")
        if name != ".text":
            continue
        ptr_relocs = struct.unpack_from("<I", data, base + 24)[0]
        n_relocs = struct.unpack_from("<H", data, base + 32)[0]
        out: list[tuple[int, int]] = []
        for j in range(n_relocs):
            entry = ptr_relocs + j * 10
            vaddr = struct.unpack_from("<I", data, entry)[0]
            # Type at +8 — we treat all i386 reloc types as 4-byte fixups.
            out.append((vaddr, 4))
        return out
    return []


def _build_reloc_mask(relocs: list[tuple[int, int]], size: int) -> bytearray:
    """Build a 1-byte-per-position mask: 1 = relocation byte (wildcard
    in the diff), 0 = real code byte (must match orig byte-for-byte)."""
    mask = bytearray(size)
    for off, sz in relocs:
        end = min(off + sz, size)
        for i in range(off, end):
            if 0 <= i < size:
                mask[i] = 1
    return mask


def _yaml_size_override(rva: int, yaml_path: Path) -> int | None:
    """Look up the per-function `size:` field in the work-pool YAML by
    streaming the file (avoids a full pyyaml parse on a ~100k-entry
    file). Returns the size as an int (decoding 0x.. literals) or None
    if the YAML doesn't exist, doesn't have an entry for `rva`, or
    doesn't have a `size:` key.

    The YAML is the human-curated source of truth and may correct
    Ghidra's auto-detected size when Ghidra under-counts (e.g. when a
    function ends with XOR EAX,EAX/RET followed by INT3 padding)."""
    if not yaml_path.exists():
        return None
    target = f"- rva: {rva:#010x}"
    in_entry = False
    for line in yaml_path.read_text().splitlines():
        if line.startswith("- rva:"):
            in_entry = (line.split("#", 1)[0].rstrip() == target)
            continue
        if not in_entry:
            continue
        stripped = line.strip()
        if stripped.startswith("size:"):
            value = stripped.split(":", 1)[1].split("#", 1)[0].strip()
            try:
                return int(value, 0)
            except ValueError:
                return None
    return None


def _hex_dump(b: bytes, width: int = 16) -> list[str]:
    out: list[str] = []
    for i in range(0, len(b), width):
        chunk = b[i : i + width]
        out.append(" ".join(f"{x:02x}" for x in chunk))
    return out


def _side_by_side(orig: bytes, ours: bytes, mask: bytearray, width: int = 16) -> str:
    """Side-by-side diff. Markers:
      `.`  bytes match (or both ends of the row are past EOF)
      `~`  bytes differ but the position is a known relocation —
           expected (linker fixup, .obj has zeros, .exe has the value)
      `*`  bytes differ AND the position is real code — actual
           structural diff that the contributor needs to fix
      `?`  one side is past EOF (size mismatch)
    """
    rows: list[str] = []
    rows.append(f"  {'offset':>6}  {'orig (binary)':<{width*3}}  ours (cl.exe)")
    n = max(len(orig), len(ours))
    for i in range(0, n, width):
        o_chunk = orig[i : i + width]
        u_chunk = ours[i : i + width]
        o_hex = _hex_dump(o_chunk)[0] if o_chunk else ""
        u_hex = _hex_dump(u_chunk)[0] if u_chunk else ""
        marker_chars = []
        for j in range(width):
            o = o_chunk[j] if j < len(o_chunk) else None
            u = u_chunk[j] if j < len(u_chunk) else None
            pos = i + j
            if o is None or u is None:
                marker_chars.append("?")
            elif o != u:
                if pos < len(mask) and mask[pos]:
                    marker_chars.append("~")
                else:
                    marker_chars.append("*")
            else:
                marker_chars.append(".")
        marker = " ".join(marker_chars[: max(len(o_chunk), len(u_chunk))])
        rows.append(f"  {i:>06x}  {o_hex:<{width*3}}  {u_hex}")
        rows.append(f"  {'':>6}  {marker:<{width*3}}")
    return "\n".join(rows)


def _verdict(orig: bytes, ours: bytes, mask: bytearray) -> tuple[str, int, int, int]:
    """Return (verdict, structural_diffs, reloc_diffs, total_orig_bytes).

    Verdicts:
      `GREEN`        no structural diffs (relocation diffs are expected)
      `PARTIAL`      sizes match but >= 1 structural byte differs
      `MISMATCH`     sizes differ — usually a missing/extra instruction
    """
    if len(orig) != len(ours):
        return ("MISMATCH", -1, -1, len(orig))
    structural = 0
    reloc = 0
    for i, (a, b) in enumerate(zip(orig, ours)):
        if a == b:
            continue
        if i < len(mask) and mask[i]:
            reloc += 1
        else:
            structural += 1
    if structural == 0:
        return ("GREEN", 0, reloc, len(orig))
    return ("PARTIAL", structural, reloc, len(orig))


def _first_mismatch(orig: bytes, ours: bytes, mask: bytearray) -> int | None:
    """First position where orig != ours AND the position is NOT a
    known relocation. Returns None if there are no structural diffs."""
    for i, (a, b) in enumerate(zip(orig, ours)):
        if a == b:
            continue
        if i < len(mask) and mask[i]:
            continue
        return i
    if len(orig) != len(ours):
        return min(len(orig), len(ours))
    return None


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("args", nargs="*", help="FUNC=<name>  [BINARY=<bin>.exe]")
    parsed = ap.parse_args()

    func_name: str | None = None
    binary_stem = "ffxivgame"
    for a in parsed.args:
        v = _parse_kv_arg(a, "FUNC")
        if v:
            func_name = v
            continue
        v = _parse_kv_arg(a, "BINARY")
        if v:
            binary_stem = v.replace(".exe", "")
            continue
        # Bare FUN_xxxx (legacy)
        if a.startswith("FUN_"):
            func_name = a

    if not func_name:
        print("usage: tools/compare.py FUNC=<symbol-name> [BINARY=<bin>.exe]", file=sys.stderr)
        return 3

    abs_addr = _abs_from_fun_name(func_name)
    if abs_addr is None:
        print(f"error: only FUN_<address> names supported today, got {func_name!r}", file=sys.stderr)
        return 3

    orig_pe, pe_layout_path, obj_root = _binary_paths(binary_stem)
    image_base, _ = _read_pe_layout(pe_layout_path)
    rva = abs_addr - image_base

    sym_path = CONFIG_DIR / f"{binary_stem}.symbols.json"
    if not sym_path.exists():
        print(f"error: {sym_path} missing — run `make split BINARY={binary_stem}.exe`", file=sys.stderr)
        return 3
    syms = json.loads(sym_path.read_text())
    sym = next((s for s in syms if s["rva"] == rva), None)
    if sym is None:
        print(f"error: rva {rva:#x} not in symbols.json (out of date?)", file=sys.stderr)
        return 3

    # Prefer the YAML's `size:` over symbols.json — Ghidra occasionally
    # under-counts (e.g. trailing XOR/RET + INT3 padding). The YAML is
    # human-curated and more reliable for known-corrected functions.
    yaml_path = CONFIG_DIR / f"{binary_stem}.yaml"
    yaml_size = _yaml_size_override(rva, yaml_path)
    sym_size = int(sym["size"])
    if yaml_size is not None and yaml_size != sym_size:
        size = yaml_size
        size_source = f"YAML ({yaml_size} B; symbols.json had {sym_size} B)"
    else:
        size = sym_size
        size_source = "symbols.json"

    obj_path = obj_root / f"{func_name}.obj"
    if not obj_path.exists():
        print(f"error: {obj_path} missing — run `make rosetta BINARY={binary_stem}.exe` first", file=sys.stderr)
        return 3

    orig = _read_orig_bytes(rva, size, orig_pe, pe_layout_path)
    ours = _coff_text_bytes(obj_path)
    relocs = _coff_text_relocs(obj_path)
    mask = _build_reloc_mask(relocs, max(len(orig), len(ours)))
    verdict, structural, reloc, total = _verdict(orig, ours, mask)

    print(f"=== {func_name} [{binary_stem}] (rva {rva:#x}, size {size} B from {size_source}) ===")
    print(f"  orig: {len(orig)} bytes from {orig_pe.relative_to(REPO_ROOT)}")
    print(f"  ours: {len(ours)} bytes from {obj_path.relative_to(REPO_ROOT)}")
    if relocs:
        print(f"  reloc-masked positions: {sum(1 for m in mask[:total] if m)} of {total} bytes")
    print()

    if verdict == "GREEN":
        if reloc:
            print(f"  ✅ GREEN — byte-identical modulo {reloc} relocation byte"
                  f"{'s' if reloc != 1 else ''} (linker-filled, expected)")
        else:
            print(f"  ✅ GREEN — byte-identical ({total} of {total} bytes match)")
        return 0

    if verdict == "MISMATCH":
        print(f"  ❌ MISMATCH — sizes differ (orig={len(orig)}, ours={len(ours)})")
        delta = len(ours) - len(orig)
        print(f"     ours is {abs(delta)} byte{'s' if abs(delta)!=1 else ''} {'longer' if delta>0 else 'shorter'}")
    else:
        matched = total - structural - reloc
        pct = 100.0 * matched / total if total else 0.0
        # The structural number is what matters for "is this a real diff"
        print(f"  🟡 PARTIAL — {matched + reloc} of {total} bytes match modulo relocations "
              f"({structural} structural diff{'s' if structural != 1 else ''}, "
              f"{reloc} reloc diff{'s' if reloc != 1 else ''}; {pct:.1f}% raw match)")

    fm = _first_mismatch(orig, ours, mask)
    if fm is not None:
        print(f"  first STRUCTURAL mismatch at offset {fm:#x} (file rva {rva + fm:#x})")
    print()
    print(_side_by_side(orig, ours, mask))
    print()
    print("Diff markers:  .  match  |  ~  reloc (linker fixup, expected)  |  *  STRUCTURAL diff")
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
