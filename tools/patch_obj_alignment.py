#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Patch every `_passthrough/*.obj`'s `.text$X<rva>` (and similar
`.<sec>$X<rva>`) section to set 1-byte alignment.

Why: cl.exe 2005's `#pragma code_seg(...)` doesn't expose alignment;
it defaults to 16-byte for `.text` and 8-byte for `.rdata`/`.data`.
When link.exe concatenates `.text$XNNNNNNNN` siblings into the merged
`.text`, it pads each subsection up to its alignment boundary. With
default 16-byte alignment, an 8-byte function followed by an 8-byte
gap subsection becomes [8B][8B padding][8B], not [8B][8B], breaking
RVA fidelity.

This tool rewrites the COFF section characteristics field
(IMAGE_SECTION_HEADER.Characteristics, bits 20-23 = alignment exponent
encoding) to `0x1` = 1-byte alignment, so subsections concatenate
without padding and land at exactly orig RVA.

In-place edit. Idempotent. Doesn't touch sections that aren't
`.<sec>$X<8-hex-digit>` named (preserves `.drectve`, `.debug$S`,
etc. with their default alignment).

Usage:
  tools/patch_obj_alignment.py ffxivlogin
  tools/patch_obj_alignment.py ffxivlogin --dry-run
  tools/patch_obj_alignment.py            # all five binaries
"""

from __future__ import annotations

import argparse
import re
import struct
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
OBJ_ROOT = REPO_ROOT / "build" / "obj" / "_passthrough"

ALL_BINARIES = ("ffxivboot", "ffxivconfig", "ffxivgame", "ffxivlogin", "ffxivupdater")

ALIGN_MASK = 0x00f00000      # bits 20..23 hold alignment exponent
ALIGN_1_BYTE = 0x00100000    # = (1 << 20)

# Match `.text$X<rva-hex>` and `.rdata$X<rva-hex>` etc.
RE_SUBSECTION = re.compile(r"^\.[a-zA-Z]+\$X[0-9a-fA-F]{8}$")


def _resolve_section_name(data: bytes, raw_name: bytes,
                          str_table_off: int) -> str:
    name = raw_name.rstrip(b"\0").decode("ascii", errors="replace")
    if name.startswith("/"):
        try:
            offset = int(name[1:])
        except ValueError:
            return name
        end = data.find(b"\0", str_table_off + offset)
        if end >= 0:
            return data[str_table_off + offset:end].decode("ascii", errors="replace")
    return name


def _parse_obj_layout(raw: bytes) -> tuple[int, int, int]:
    """Return (n_sections, sec_off, str_table_off) for either standard
    COFF (20-byte header) or /bigobj ANON object (56-byte header).

    Symbol record size differs between the two: standard COFF uses
    IMAGE_SYMBOL (18 bytes) while /bigobj uses IMAGE_SYMBOL_EX
    (20 bytes — SectionNumber is widened from WORD to DWORD).
    """
    sig1, sig2 = struct.unpack_from("<HH", raw, 0)
    if sig1 == 0x0000 and sig2 == 0xFFFF:
        # /bigobj ANON_OBJECT_HEADER_BIGOBJ. Layout (Microsoft):
        #   0x00 Sig1 (UInt16) = 0x0000
        #   0x02 Sig2 (UInt16) = 0xFFFF
        #   0x04 Version (UInt16) = 2
        #   0x06 Machine (UInt16) = 0x014C (i386)
        #   0x08 TimeDateStamp (UInt32)
        #   0x0c ClassID (16 bytes GUID)
        #   0x1c SizeOfData (UInt32)
        #   0x20 Flags (UInt32)
        #   0x24 MetaDataSize (UInt32)
        #   0x28 MetaDataOffset (UInt32)
        #   0x2c NumberOfSections (UInt32)
        #   0x30 PointerToSymbolTable (UInt32)
        #   0x34 NumberOfSymbols (UInt32)
        #   0x38 → start of section table (40 bytes each)
        n_sections = struct.unpack_from("<I", raw, 0x2c)[0]
        sym_off = struct.unpack_from("<I", raw, 0x30)[0]
        n_syms = struct.unpack_from("<I", raw, 0x34)[0]
        sec_off = 0x38
        sym_size = 20  # IMAGE_SYMBOL_EX
    else:
        # Standard COFF
        n_sections = struct.unpack_from("<H", raw, 2)[0]
        sym_off = struct.unpack_from("<I", raw, 8)[0]
        n_syms = struct.unpack_from("<I", raw, 12)[0]
        opt_size = struct.unpack_from("<H", raw, 16)[0]
        sec_off = 20 + opt_size
        sym_size = 18  # IMAGE_SYMBOL
    str_table_off = sym_off + n_syms * sym_size
    return n_sections, sec_off, str_table_off


def patch_obj(obj_path: Path, dry_run: bool) -> dict:
    raw = bytearray(obj_path.read_bytes())
    n_sections, sec_off, str_table_off = _parse_obj_layout(raw)
    n_patched = 0
    n_already = 0
    for i in range(n_sections):
        base = sec_off + i * 40
        name = _resolve_section_name(raw, bytes(raw[base:base + 8]), str_table_off)
        if not RE_SUBSECTION.match(name):
            continue
        chars_off = base + 36
        chars = struct.unpack_from("<I", raw, chars_off)[0]
        align = chars & ALIGN_MASK
        if align == ALIGN_1_BYTE:
            n_already += 1
            continue
        new_chars = (chars & ~ALIGN_MASK) | ALIGN_1_BYTE
        struct.pack_into("<I", raw, chars_off, new_chars)
        n_patched += 1
    if n_patched and not dry_run:
        obj_path.write_bytes(raw)
    return {"patched": n_patched, "already": n_already}


def process_binary(binary: str, dry_run: bool) -> dict:
    obj_dir = OBJ_ROOT / binary
    if not obj_dir.is_dir():
        return {"binary": binary, "skipped": "obj dir missing"}
    n_files = n_patched_files = total_patched = total_already = 0
    for obj_path in sorted(obj_dir.glob("*.obj")):
        n_files += 1
        s = patch_obj(obj_path, dry_run)
        if s["patched"]:
            n_patched_files += 1
        total_patched += s["patched"]
        total_already += s["already"]
    return {
        "binary": binary,
        "obj_files": n_files,
        "files_with_patches": n_patched_files,
        "subsections_patched": total_patched,
        "subsections_already_align1": total_already,
    }


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", nargs="?", help="binary stem (default: all five)")
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()
    binaries = (args.binary,) if args.binary else ALL_BINARIES
    print(f"=== patch_obj_alignment (dry-run={args.dry_run}) ===")
    for stem in binaries:
        s = process_binary(stem, args.dry_run)
        if "skipped" in s:
            print(f"  {stem}: SKIPPED ({s['skipped']})")
            continue
        print(f"  {stem}: obj_files={s['obj_files']:>5} "
              f"files_patched={s['files_with_patches']:>5} "
              f"subsections_patched={s['subsections_patched']:>6} "
              f"already_align1={s['subsections_already_align1']:>4}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
