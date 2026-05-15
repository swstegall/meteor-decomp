#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Post-link patcher: copy a curated set of PE header fields from orig
into our re-linked PE, so byte-level diff goes to zero.

Without this, a freshly-linked PE diverges from orig in:
  - PE timestamp (link.exe writes "now")
  - PE checksum (link.exe doesn't bother for /DEBUG-less builds)
  - AddressOfEntryPoint (we use a stub symbol; orig points elsewhere)
  - SizeOfImage / SizeOfHeaders / per-section virtual sizes (link.exe
    uses its own alignment + layout policy)
  - Subsystem version, OS version, image version
  - DllCharacteristics
  - LoaderFlags, NumberOfRvaAndSizes, NumberOfSymbols (typically 0)
  - Section table entries (raw_size, virtual_size, etc.)

This tool reads `orig/<bin>.exe`, parses both PE headers, and rewrites
the corresponding fields in `build/link/<bin>.exe`.

Idempotent. Run after every link.

Usage:
  tools/postlink_patch.py ffxivlogin
  tools/postlink_patch.py ffxivlogin --dry-run

Caveats:
  - This is a "match orig at the byte level" tool, not a "make the PE
    runnable on a different OS" tool. The patched fields are ONLY
    valid for the orig binary's intended runtime environment.
  - We DO NOT touch section CONTENT — that's the job of the upstream
    section emitter (`tools/emit_data_sections.py --include-text`).
"""

from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
ORIG = REPO_ROOT / "orig"
LINK = REPO_ROOT / "build" / "link"


# Offsets in the optional header (relative to start of optional header).
# Layout per Microsoft PE/COFF spec; PE32 (i386) variant.
OPT_OFF = {
    "magic":                  0x00,  # u16
    "major_linker_ver":       0x02,  # u8
    "minor_linker_ver":       0x03,  # u8
    "size_code":              0x04,  # u32
    "size_init_data":         0x08,  # u32
    "size_uninit_data":       0x0c,  # u32
    "entry_rva":              0x10,  # u32
    "base_code":              0x14,  # u32
    "base_data":              0x18,  # u32
    "image_base":             0x1c,  # u32
    "section_alignment":      0x20,  # u32
    "file_alignment":         0x24,  # u32
    "major_os_ver":           0x28,  # u16
    "minor_os_ver":           0x2a,  # u16
    "major_image_ver":        0x2c,  # u16
    "minor_image_ver":        0x2e,  # u16
    "major_subsys_ver":       0x30,  # u16
    "minor_subsys_ver":       0x32,  # u16
    "win32_version":          0x34,  # u32 (reserved, must be 0)
    "size_image":             0x38,  # u32
    "size_headers":           0x3c,  # u32
    "checksum":               0x40,  # u32
    "subsystem":              0x44,  # u16
    "dll_characteristics":    0x46,  # u16
    "size_stack_reserve":     0x48,  # u32
    "size_stack_commit":      0x4c,  # u32
    "size_heap_reserve":      0x50,  # u32
    "size_heap_commit":       0x54,  # u32
    "loader_flags":           0x58,  # u32
    "n_rva_sizes":            0x5c,  # u32
    "data_directories":       0x60,  # 16 entries × 8 bytes each
}

# Fields we copy from orig → ours, with their u-size (u8/u16/u32).
FIELDS_TO_COPY = [
    ("major_linker_ver", "B"),
    ("minor_linker_ver", "B"),
    ("size_code", "I"),
    ("size_init_data", "I"),
    ("size_uninit_data", "I"),
    ("entry_rva", "I"),
    ("base_code", "I"),
    ("base_data", "I"),
    ("major_os_ver", "H"),
    ("minor_os_ver", "H"),
    ("major_image_ver", "H"),
    ("minor_image_ver", "H"),
    ("major_subsys_ver", "H"),
    ("minor_subsys_ver", "H"),
    ("size_image", "I"),
    ("size_headers", "I"),
    ("dll_characteristics", "H"),
    ("size_stack_reserve", "I"),
    ("size_stack_commit", "I"),
    ("size_heap_reserve", "I"),
    ("size_heap_commit", "I"),
    ("loader_flags", "I"),
    # checksum is recomputed from final bytes — see _recompute_checksum
]

# All 16 PE data-directory entries (RVA + size each, 8 bytes per entry).
# Index → (name, offset from data_directories[0]).
DATA_DIRECTORIES = [
    "ExportTable",        # 0
    "ImportTable",        # 1
    "ResourceTable",      # 2
    "ExceptionTable",     # 3
    "CertificateTable",   # 4
    "BaseRelocTable",     # 5
    "DebugDirectory",     # 6
    "ArchSpecific",       # 7
    "GlobalPointer",      # 8
    "TLSTable",           # 9
    "LoadConfigTable",    # 10
    "BoundImports",       # 11
    "IAT",                # 12
    "DelayImports",       # 13
    "CLRRuntime",         # 14
    "Reserved",           # 15
]

# Section-table fields we copy per-section (orig wins on each).
# Layout of an IMAGE_SECTION_HEADER:
#   0x00 Name (8 B)
#   0x08 VirtualSize (u32)
#   0x0c VirtualAddress (u32)
#   0x10 SizeOfRawData (u32)
#   0x14 PointerToRawData (u32)
#   0x18 PointerToRelocations (u32)
#   0x1c PointerToLinenumbers (u32)
#   0x20 NumberOfRelocations (u16)
#   0x22 NumberOfLinenumbers (u16)
#   0x24 Characteristics (u32)
SECTION_FIELDS_TO_COPY = [
    (0x08, "I"),  # VirtualSize
    (0x0c, "I"),  # VirtualAddress
    (0x10, "I"),  # SizeOfRawData
    (0x24, "I"),  # Characteristics
    # PointerToRawData (0x14) intentionally NOT copied — moving the
    # file offset requires actually relocating the section bytes,
    # which we don't do (and don't need to: the loader uses RVAs at
    # runtime, file offsets are only consulted at load).
]


def _pe_off(data: bytes) -> int:
    return struct.unpack_from("<I", data, 0x3c)[0]


def _opt_off(data: bytes) -> int:
    return _pe_off(data) + 0x18


def _section_table_off(data: bytes) -> int:
    pe_off = _pe_off(data)
    n_sections = struct.unpack_from("<H", data, pe_off + 6)[0]
    opt_size = struct.unpack_from("<H", data, pe_off + 0x14)[0]
    return pe_off + 0x18 + opt_size, n_sections


def _read_pe_timestamp(data: bytes) -> int:
    """Read the COFF file header's TimeDateStamp (4 bytes at PE+8)."""
    pe_off = _pe_off(data)
    return struct.unpack_from("<I", data, pe_off + 8)[0]


def _patch_field(target: bytearray, opt_off: int, field: str,
                 u_code: str, value: int) -> bool:
    """Patch `target` at the optional-header field. Returns True if value changed."""
    field_off = opt_off + OPT_OFF[field]
    fmt = f"<{u_code}"
    cur = struct.unpack_from(fmt, target, field_off)[0]
    if cur == value:
        return False
    struct.pack_into(fmt, target, field_off, value)
    return True


def _patch_section_field(target: bytearray, sec_off: int, idx: int,
                         field_off: int, u_code: str, value: int) -> bool:
    abs_off = sec_off + idx * 40 + field_off
    fmt = f"<{u_code}"
    cur = struct.unpack_from(fmt, target, abs_off)[0]
    if cur == value:
        return False
    struct.pack_into(fmt, target, abs_off, value)
    return True


def _recompute_checksum(data: bytearray, file_size: int) -> int:
    """Compute the PE checksum exactly as imagehlp.CheckSumMappedFile does:
    16-bit unsigned add of every WORD in the file (with the 4 checksum
    bytes treated as zero), end-around-carry to 16 bits, then add the
    file size. The result is a u32."""
    pe_off = _pe_off(data)
    opt_off = pe_off + 0x18
    csum_off = opt_off + OPT_OFF["checksum"]
    # Sum all u16 words, treating checksum bytes as zero.
    n_words = file_size // 2
    sum_ = 0
    for i in range(n_words):
        off = i * 2
        if off == csum_off or off == csum_off + 2:
            continue
        w = struct.unpack_from("<H", data, off)[0]
        sum_ = (sum_ + w) & 0xFFFFFFFF
        # End-around carry
        sum_ = (sum_ & 0xFFFF) + (sum_ >> 16)
    # Final carry
    sum_ = (sum_ & 0xFFFF) + (sum_ >> 16)
    sum_ &= 0xFFFF
    if file_size & 1:
        # Tail byte (rare for our PEs).
        sum_ += data[file_size - 1]
        sum_ = (sum_ & 0xFFFF) + (sum_ >> 16)
        sum_ &= 0xFFFF
    return (sum_ + file_size) & 0xFFFFFFFF


def patch(binary: str, dry_run: bool) -> dict:
    orig_path = ORIG / f"{binary}.exe"
    ours_path = LINK / f"{binary}.exe"
    orig = orig_path.read_bytes()
    ours = bytearray(ours_path.read_bytes())

    o_pe_off = _pe_off(orig)
    u_pe_off = _pe_off(ours)
    o_opt = _opt_off(orig)
    u_opt = _opt_off(ours)

    n_changed = 0
    n_unchanged = 0

    # 0. DOS header + DOS stub. Layout differs between MSVC linker
    # versions — orig binaries from VS 2005 have a longer Rich header
    # tail in the DOS stub region (bytes between e_lfanew=0x3c+4 and
    # the start of the NT headers). The Rich header encodes the
    # toolchain identity and is part of the original .exe identity
    # we want to preserve.
    #
    # Strategy:
    #   - Copy orig's bytes [0..o_pe_off] verbatim into ours' DOS region
    #     (this gives us orig's e_lfanew + DOS stub + Rich header).
    #   - If our pe_off differs from orig's, RELOCATE our NT headers
    #     to land at the orig pe_off. Section data lives at file
    #     offsets >= file_alignment (typically 0x1000) which is past
    #     both PE-header positions, so this relocation is safe.
    nt_size = u_opt + 0x60 + 16 * 8 + 40 * struct.unpack_from("<H", ours, u_pe_off + 6)[0] - u_pe_off
    nt_block = bytes(ours[u_pe_off:u_pe_off + nt_size])
    # Make sure orig's pe_off + our nt_size doesn't overrun the
    # padding region that ends at the first section's PointerToRawData.
    file_align = struct.unpack_from("<I", ours, u_opt + OPT_OFF["file_alignment"])[0]
    if o_pe_off + nt_size > file_align:
        raise ValueError(
            f"Cannot relocate NT headers to pe_off=0x{o_pe_off:x}: "
            f"would overlap section data at 0x{file_align:x}"
        )
    # Build the new prefix: orig DOS + zero pad to o_pe_off + our NT block + zero pad to file_align.
    new_prefix = bytearray(file_align)
    new_prefix[:o_pe_off] = orig[:o_pe_off]
    new_prefix[o_pe_off:o_pe_off + nt_size] = nt_block
    # Patch e_lfanew in the new prefix to point at o_pe_off (orig
    # already did this if we copied its [0..o_pe_off]; double-check).
    struct.pack_into("<I", new_prefix, 0x3c, o_pe_off)
    # Replace ours[:file_align] with new_prefix.
    if bytes(ours[:file_align]) != bytes(new_prefix):
        ours[:file_align] = new_prefix
        n_changed += 1
    else:
        n_unchanged += 1
    # Re-compute offsets after relocation.
    u_pe_off = _pe_off(ours)
    u_opt = _opt_off(ours)

    # 1. COFF timestamp.
    o_ts = _read_pe_timestamp(orig)
    u_ts = _read_pe_timestamp(ours)
    if o_ts != u_ts:
        ts_off = _pe_off(ours) + 8
        struct.pack_into("<I", ours, ts_off, o_ts)
        n_changed += 1
    else:
        n_unchanged += 1

    # 2. Optional-header fields.
    for field, u_code in FIELDS_TO_COPY:
        cur = struct.unpack_from(f"<{u_code}", orig, o_opt + OPT_OFF[field])[0]
        if _patch_field(ours, u_opt, field, u_code, cur):
            n_changed += 1
        else:
            n_unchanged += 1

    # 3. Section-table fields (per-section).
    o_sec_off, o_n_sec = _section_table_off(orig)
    u_sec_off, u_n_sec = _section_table_off(ours)
    # Match by section name.
    o_secs = []
    for i in range(o_n_sec):
        name = orig[o_sec_off + i * 40:o_sec_off + i * 40 + 8].rstrip(b"\0")
        o_secs.append((name, i))
    u_secs = {}
    for i in range(u_n_sec):
        name = ours[u_sec_off + i * 40:u_sec_off + i * 40 + 8].rstrip(b"\0")
        u_secs[bytes(name)] = i
    for name, o_idx in o_secs:
        if name not in u_secs:
            continue
        u_idx = u_secs[name]
        for field_off, u_code in SECTION_FIELDS_TO_COPY:
            cur = struct.unpack_from(f"<{u_code}", orig, o_sec_off + o_idx * 40 + field_off)[0]
            if _patch_section_field(ours, u_sec_off, u_idx, field_off, u_code, cur):
                n_changed += 1
            else:
                n_unchanged += 1

    # 3.5 Reorder section data to match orig's file layout when section
    # ORDER differs (e.g. ffxivboot has MSSMIXER between `.text` and
    # `.rdata` in orig; link.exe places it after `.tls`). Each section's
    # CONTENT is correct (we verified before this patcher); we just need
    # to move the bytes to the file offsets orig expects.
    #
    # Strategy:
    #   1. Read each section's bytes from OUR current PointerToRawData.
    #   2. Build a new file: header (already orig-aligned) + sections in
    #      ORIG's layout (orig's PointerToRawData per section).
    #   3. Rewrite the section table entries to match orig's order +
    #      PointerToRawData values.
    o_secs_by_name: dict[bytes, dict] = {}
    for i in range(o_n_sec):
        b = o_sec_off + i * 40
        name = bytes(orig[b:b + 8].rstrip(b"\0"))
        o_secs_by_name[name] = {
            "vaddr": struct.unpack_from("<I", orig, b + 12)[0],
            "rsize": struct.unpack_from("<I", orig, b + 16)[0],
            "rptr":  struct.unpack_from("<I", orig, b + 20)[0],
            "chars": struct.unpack_from("<I", orig, b + 36)[0],
            "vsize": struct.unpack_from("<I", orig, b + 8)[0],
            "header": bytes(orig[b:b + 40]),  # full section table entry
        }
    u_sec_by_name: dict[bytes, tuple[int, int]] = {}  # name → (cur_rptr, cur_rsize)
    for i in range(u_n_sec):
        b = u_sec_off + i * 40
        name = bytes(ours[b:b + 8].rstrip(b"\0"))
        rsize = struct.unpack_from("<I", ours, b + 16)[0]
        rptr = struct.unpack_from("<I", ours, b + 20)[0]
        u_sec_by_name[name] = (rptr, rsize)

    # Compute the maximum file offset orig sections occupy.
    max_orig_end = max(s["rptr"] + s["rsize"] for s in o_secs_by_name.values())
    # Pad ours up to that size if needed.
    if len(ours) < max_orig_end:
        ours += bytes(max_orig_end - len(ours))

    # Build a fresh layout: zero-pad past the headers, then place each
    # orig section at its orig file offset using OUR section bytes.
    file_align = struct.unpack_from("<I", ours, u_opt + OPT_OFF["file_alignment"])[0]
    new_body = bytearray(max(len(ours), max_orig_end))
    new_body[:file_align] = ours[:file_align]   # PE header region
    n_layout_changes = 0
    for name, o_sec in o_secs_by_name.items():
        if name not in u_sec_by_name:
            continue
        cur_rptr, cur_rsize = u_sec_by_name[name]
        if cur_rsize == 0:
            continue
        if cur_rptr == o_sec["rptr"] and bytes(ours[cur_rptr:cur_rptr + cur_rsize]) == \
                bytes(new_body[o_sec["rptr"]:o_sec["rptr"] + cur_rsize]):
            # Already aligned to orig.
            new_body[o_sec["rptr"]:o_sec["rptr"] + cur_rsize] = ours[cur_rptr:cur_rptr + cur_rsize]
            continue
        new_body[o_sec["rptr"]:o_sec["rptr"] + cur_rsize] = ours[cur_rptr:cur_rptr + cur_rsize]
        n_layout_changes += 1
    if n_layout_changes:
        # Replace section table entries with orig's verbatim — both
        # order and per-entry fields. This blasts our entries in
        # favour of orig's full 40-byte headers.
        for i, (name, sec) in enumerate(o_secs_by_name.items()):
            target_off = u_sec_off + i * 40
            new_body[target_off:target_off + 40] = sec["header"]
        # NumberOfSections in COFF header (PE_off+6) — copy from orig.
        new_body[u_pe_off + 6] = o_n_sec & 0xFF
        new_body[u_pe_off + 7] = (o_n_sec >> 8) & 0xFF
        # Replace ours buffer.
        ours = new_body
        n_changed += 1
    else:
        n_unchanged += 1
    # Re-fetch offsets after potential layout shuffle.
    u_pe_off = _pe_off(ours)
    u_opt = _opt_off(ours)

    # 4. Data directories — copy all 16 entries verbatim.
    o_data_dir = o_opt + OPT_OFF["data_directories"]
    u_data_dir = u_opt + OPT_OFF["data_directories"]
    for i in range(16):
        o_rva = struct.unpack_from("<I", orig, o_data_dir + i * 8)[0]
        o_sz = struct.unpack_from("<I", orig, o_data_dir + i * 8 + 4)[0]
        u_rva = struct.unpack_from("<I", ours, u_data_dir + i * 8)[0]
        u_sz = struct.unpack_from("<I", ours, u_data_dir + i * 8 + 4)[0]
        if o_rva != u_rva or o_sz != u_sz:
            struct.pack_into("<II", ours, u_data_dir + i * 8, o_rva, o_sz)
            n_changed += 1
        else:
            n_unchanged += 1

    # 4.5 Authenticode certificate (data directory index 4). Stored
    # OUTSIDE any PE section, appended at file offset = directory
    # entry's "RVA" (which is actually a FILE OFFSET for cert tables —
    # an exception to the "RVA" naming, see PE spec §5.3). Copy the
    # certificate bytes from orig into ours so the file is identical
    # even in the signed-attestation region.
    cert_rva = struct.unpack_from("<I", orig, o_data_dir + 4 * 8)[0]
    cert_size = struct.unpack_from("<I", orig, o_data_dir + 4 * 8 + 4)[0]
    if cert_rva and cert_size:
        cert_bytes = orig[cert_rva:cert_rva + cert_size]
        # Pad ours to cert_rva (orig's file offset) then append.
        if len(ours) < cert_rva:
            ours += bytes(cert_rva - len(ours))
        # Splice (overwrite) the cert region with orig bytes.
        end = cert_rva + cert_size
        if len(ours) < end:
            ours += bytes(end - len(ours))
        ours[cert_rva:end] = cert_bytes
        n_changed += 1

    # 5. Recompute checksum (last, after all other patches are in).
    new_csum = _recompute_checksum(ours, len(ours))
    csum_off = u_opt + OPT_OFF["checksum"]
    cur_csum = struct.unpack_from("<I", ours, csum_off)[0]
    if cur_csum != new_csum:
        struct.pack_into("<I", ours, csum_off, new_csum)
        n_changed += 1
    else:
        n_unchanged += 1

    if not dry_run and n_changed:
        ours_path.write_bytes(bytes(ours))

    return {
        "binary": binary,
        "fields_changed": n_changed,
        "fields_unchanged": n_unchanged,
        "orig_timestamp": f"0x{o_ts:08x}",
        "ours_old_timestamp": f"0x{u_ts:08x}",
        "checksum": f"0x{new_csum:08x}",
    }


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", help="binary stem (e.g. ffxivlogin)")
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()
    print(f"=== postlink_patch (dry-run={args.dry_run}) ===")
    s = patch(args.binary, args.dry_run)
    for k, v in s.items():
        print(f"  {k}: {v}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
