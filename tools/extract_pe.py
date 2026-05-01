#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Dump structural metadata from each PE in `orig/`. Pure-Python, no
dependencies — runs on a fresh checkout.

Outputs:
  - prints a per-binary summary to stdout
  - writes `build/pe-layout/<name>.json` with the full structure
  - writes `build/pe-layout/<name>/<section>.bin` raw section dumps
    when --extract-sections is given
"""

from __future__ import annotations

import argparse
import datetime
import json
import os
import struct
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
ORIG_DIR = REPO_ROOT / "orig"
OUT_DIR = REPO_ROOT / "build" / "pe-layout"

DLL_CHARACTERISTICS = {
    0x0020: "HIGH_ENTROPY_VA",
    0x0040: "DYNAMIC_BASE",
    0x0080: "FORCE_INTEGRITY",
    0x0100: "NX_COMPAT",
    0x0200: "NO_ISOLATION",
    0x0400: "NO_SEH",
    0x0800: "NO_BIND",
    0x1000: "APPCONTAINER",
    0x2000: "WDM_DRIVER",
    0x4000: "GUARD_CF",
    0x8000: "TERMINAL_SERVER_AWARE",
}

SECTION_FLAGS = {
    0x00000020: "CNT_CODE",
    0x00000040: "CNT_INITIALIZED_DATA",
    0x00000080: "CNT_UNINITIALIZED_DATA",
    0x20000000: "MEM_EXECUTE",
    0x40000000: "MEM_READ",
    0x80000000: "MEM_WRITE",
    0x10000000: "MEM_SHARED",
}


def decode_section_flags(flags: int) -> list[str]:
    return [name for bit, name in SECTION_FLAGS.items() if flags & bit]


def parse_pe(path: Path) -> dict:
    data = path.read_bytes()
    if data[:2] != b"MZ":
        raise ValueError(f"{path}: not an MZ image")
    e_lfanew = struct.unpack_from("<I", data, 0x3C)[0]
    if data[e_lfanew : e_lfanew + 4] != b"PE\0\0":
        raise ValueError(f"{path}: PE signature missing")

    coff = e_lfanew + 4
    machine, n_sections, timestamp, _, _, size_opt, characteristics = struct.unpack_from(
        "<HHIIIHH", data, coff
    )

    opt = coff + 20
    magic = struct.unpack_from("<H", data, opt)[0]
    if magic != 0x10B:
        raise ValueError(f"{path}: not PE32 (magic={magic:#x})")

    (
        magic,
        major_link,
        minor_link,
        size_code,
        size_init_data,
        size_uninit_data,
        addr_entry,
        base_code,
        base_data,
        image_base,
        sect_align,
        file_align,
        major_os,
        minor_os,
        major_image,
        minor_image,
        major_subsys,
        minor_subsys,
        _reserved,
        size_image,
        size_headers,
        checksum,
        subsystem,
        dll_char,
        stack_reserve,
        stack_commit,
        heap_reserve,
        heap_commit,
        loader_flags,
        n_rva_sizes,
    ) = struct.unpack_from("<HBBIIIIIIIIIHHHHHHIIIIHHIIIIII", data, opt)

    sec_off = opt + size_opt
    sections = []
    for i in range(n_sections):
        base = sec_off + i * 40
        name = data[base : base + 8].rstrip(b"\0").decode("latin-1", errors="replace")
        vsize = struct.unpack_from("<I", data, base + 8)[0]
        vaddr = struct.unpack_from("<I", data, base + 12)[0]
        rsize = struct.unpack_from("<I", data, base + 16)[0]
        raddr = struct.unpack_from("<I", data, base + 20)[0]
        chars = struct.unpack_from("<I", data, base + 36)[0]
        sections.append(
            {
                "name": name,
                "virtual_size": vsize,
                "virtual_address": vaddr,
                "raw_size": rsize,
                "raw_pointer": raddr,
                "characteristics": chars,
                "characteristics_decoded": decode_section_flags(chars),
            }
        )

    # Look for hallmarks the docs hint at.
    rdata_blob = b""
    rdata_va = None
    for s in sections:
        if s["name"] == ".rdata":
            rdata_blob = data[s["raw_pointer"] : s["raw_pointer"] + s["raw_size"]]
            rdata_va = s["virtual_address"]
            break

    has_security_cookie = b"__security_cookie" in rdata_blob
    has_msvcr_import = b"MSVCR80" in data or b"MSVCR90" in data
    has_msvcp_import = b"MSVCP80" in data or b"MSVCP90" in data
    has_d3d9_import = b"d3d9.dll" in data.lower() or b"d3d9.dll".lower() in data
    has_dinput8 = b"dinput8.dll" in data
    has_ws2_32 = b"WS2_32.dll" in data or b"ws2_32.dll" in data
    has_lua = b"lua_pcall" in rdata_blob or b"luaL_register" in rdata_blob

    return {
        "path": str(path),
        "size": len(data),
        "machine": f"{machine:#x}",
        "n_sections": n_sections,
        "timestamp": timestamp,
        "timestamp_iso": datetime.datetime.fromtimestamp(timestamp, datetime.timezone.utc).isoformat().replace("+00:00", "Z"),
        "characteristics": f"{characteristics:#x}",
        "linker_version": f"{major_link}.{minor_link}",
        "image_base": f"{image_base:#x}",
        "entry_rva": f"{addr_entry:#x}",
        "size_code": size_code,
        "size_init_data": size_init_data,
        "size_uninit_data": size_uninit_data,
        "section_alignment": f"{sect_align:#x}",
        "file_alignment": f"{file_align:#x}",
        "subsystem": subsystem,
        "dll_characteristics": f"{dll_char:#x}",
        "stack_reserve": stack_reserve,
        "stack_commit": stack_commit,
        "heap_reserve": heap_reserve,
        "heap_commit": heap_commit,
        "sections": sections,
        "fingerprints": {
            "has_security_cookie": has_security_cookie,
            "has_msvcr_import": has_msvcr_import,
            "has_msvcp_import": has_msvcp_import,
            "has_d3d9_import": has_d3d9_import,
            "has_dinput8_import": has_dinput8,
            "has_ws2_32_import": has_ws2_32,
            "has_lua_imports": has_lua,
        },
    }


def write_summary(name: str, info: dict) -> None:
    print(f"=== {name} ===")
    print(f"  size:       {info['size']:>12,} bytes")
    print(f"  built:      {info['timestamp_iso']}")
    print(f"  linker:     {info['linker_version']}  (8.0 → MSVC VS 2005)")
    print(f"  base:       {info['image_base']}")
    print(f"  entry:      {info['entry_rva']}")
    print(f"  sections:   {info['n_sections']}")
    for s in info["sections"]:
        flags = "|".join(s["characteristics_decoded"]) or "?"
        print(
            f"    {s['name']:10s}  va={s['virtual_address']:#010x}  "
            f"vsize={s['virtual_size']:#010x}  rsize={s['raw_size']:#010x}  [{flags}]"
        )
    fp = info["fingerprints"]
    on = lambda b: "Y" if b else "."
    print(
        f"  fingerprints: cookie={on(fp['has_security_cookie'])} "
        f"msvcr={on(fp['has_msvcr_import'])} msvcp={on(fp['has_msvcp_import'])} "
        f"d3d9={on(fp['has_d3d9_import'])} dinput8={on(fp['has_dinput8_import'])} "
        f"ws2_32={on(fp['has_ws2_32_import'])} lua={on(fp['has_lua_imports'])}"
    )


def extract_sections(name: str, info: dict) -> None:
    src = Path(info["path"])
    raw = src.read_bytes()
    out = OUT_DIR / name
    out.mkdir(parents=True, exist_ok=True)
    for s in info["sections"]:
        sec_name = s["name"].lstrip(".") or "noname"
        if not sec_name.replace("_", "").isalnum():
            sec_name = sec_name.encode("latin-1").hex()
        target = out / f"{sec_name}.bin"
        target.write_bytes(raw[s["raw_pointer"] : s["raw_pointer"] + s["raw_size"]])


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument(
        "--extract-sections",
        action="store_true",
        help="dump each section's raw bytes to build/pe-layout/<name>/",
    )
    ap.add_argument(
        "--orig-dir",
        type=Path,
        default=ORIG_DIR,
        help=f"directory holding the .exe symlinks (default: {ORIG_DIR})",
    )
    args = ap.parse_args()

    OUT_DIR.mkdir(parents=True, exist_ok=True)

    if not args.orig_dir.exists():
        print(f"error: {args.orig_dir} does not exist; run tools/symlink_orig.sh first", file=sys.stderr)
        return 1

    exes = sorted(args.orig_dir.glob("*.exe"))
    if not exes:
        print(f"error: no .exe files in {args.orig_dir}; run tools/symlink_orig.sh", file=sys.stderr)
        return 1

    for exe in exes:
        try:
            info = parse_pe(exe)
        except ValueError as e:
            print(f"warning: skipping {exe}: {e}", file=sys.stderr)
            continue
        write_summary(exe.name, info)
        out_json = OUT_DIR / f"{exe.stem}.json"
        out_json.write_text(json.dumps(info, indent=2))
        if args.extract_sections:
            extract_sections(exe.stem, info)
    return 0


if __name__ == "__main__":
    sys.exit(main())
