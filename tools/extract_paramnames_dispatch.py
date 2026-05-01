#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Recover the actual PARAMNAME_<id> property strings by walking the
binary's per-Data-class "param-name dispatcher" function — the
vtable slot that takes a GAM id (e.g. 100..125) and returns the
property's display string.

Until we ran this dispatcher discovery, the GAM extractor only had
generic `IntData.Value0` placeholders. The dispatcher confirms that
each Data class HAS real property names; they're just not present
as standalone strings in `.rdata` (they're loaded from `.data`
indirectly by the per-class dispatcher's jump table).

Methodology:
  - For each known dispatcher RVA (currently only CharaMakeData's
    MetadataProvider slot 2 = RVA 0x001ad010), scan the asm for
    `PUSH <imm32>` instructions whose immediate is a `.data`
    address. Those are the per-id string pointers in jump-table
    order.
  - The jump table at the beginning of the function (`JMP dword
    ptr [EAX*0x4 + <table>]`) maps `id - <base>` to which case
    runs; we extract the base from the `ADD EAX, -<base>` prologue.
  - Read the C string at each pointed-to `.data` address.

Reads:
  orig/<binary>.exe  (PE, the bytes themselves)

Writes:
  config/<binary>.paramnames_resolved.json
  build/wire/<binary>.paramnames.md
  config/<binary>.gam_params.json  (in-place enrichment with the
                                    `paramname` field for each
                                    matched (id, ns))
"""

from __future__ import annotations

import argparse
import json
import re
import struct
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
ORIG = REPO_ROOT / "orig"
CONFIG = REPO_ROOT / "config"
ASM_ROOT = REPO_ROOT / "asm"
WIRE = REPO_ROOT / "build" / "wire"

# Each dispatcher entry tells us: "this asm function takes a GAM id,
# subtracts <base>, and returns a string pointer for one of the
# CharaMakeData / Player / etc. properties." Discovered by manual
# inspection of the largest MetadataProvider slots.
DISPATCHERS: dict[str, dict] = {
    "CharaMakeData": {
        "rva": 0x001ad010,
        "ns": "Application::Network::GameAttributeManager::Data::CharaMakeData",
        "id_base": 100,  # ADD EAX, -0x64 in prologue
        "id_count": 26,  # CMP EAX, 0x19 = 25 → 0..25 inclusive
    },
    # Add other dispatchers as we identify them — each Data class has
    # one in its MetadataProvider vtable. ClientSelectData / Player /
    # PlayerPlayer / ZoneInitData TBD.
}


def _section_for_va(sections: list[tuple], va: int) -> tuple[str | None, int]:
    """Return (section_name, file_offset) for a given VA, or (None, 0)."""
    rva = va - 0x400000
    for name, vaddr, raddr, vsize, rsize in sections:
        if vaddr <= rva < vaddr + max(vsize, rsize):
            return name, raddr + (rva - vaddr)
    return None, 0


def _parse_pe(path: Path) -> tuple[bytes, list[tuple]]:
    data = path.read_bytes()
    e_lfanew = struct.unpack_from("<I", data, 0x3c)[0]
    n = struct.unpack_from("<H", data, e_lfanew + 6)[0]
    size_opt = struct.unpack_from("<H", data, e_lfanew + 20)[0]
    sec_off = e_lfanew + 24 + size_opt
    sections = []
    for i in range(n):
        base = sec_off + i * 40
        name = data[base : base + 8].rstrip(b"\0").decode("latin-1")
        vsize = struct.unpack_from("<I", data, base + 8)[0]
        vaddr = struct.unpack_from("<I", data, base + 12)[0]
        rsize = struct.unpack_from("<I", data, base + 16)[0]
        raddr = struct.unpack_from("<I", data, base + 20)[0]
        sections.append((name, vaddr, raddr, vsize, rsize))
    return data, sections


def _read_cstr(data: bytes, file_off: int, max_len: int = 256) -> str:
    blob = data[file_off : file_off + max_len]
    nul = blob.find(b"\0")
    return blob[: nul if nul >= 0 else max_len].decode("latin-1", errors="replace")


def extract_dispatcher(
    pe_data: bytes,
    sections: list[tuple],
    asm_path: Path,
    name: str,
    info: dict,
) -> list[dict]:
    """Walk a dispatcher's asm, extract its 26-entry string-pointer
    table from `PUSH <imm32>` instructions in case-handler order."""
    asm = asm_path.read_text()

    # Match `PUSH 0x<hex>` lines — extract the immediates that look
    # like .data addresses (>= 0x012... typically lands in .data).
    pushes: list[int] = []
    for m in re.finditer(r"^\s*[0-9a-f]+:\s+(?:[0-9a-f][0-9a-f] )+\s+PUSH (0x[0-9a-fA-F]+)$", asm, re.MULTILINE):
        va = int(m.group(1), 16)
        sec, _foff = _section_for_va(sections, va)
        if sec == ".data":
            pushes.append(va)

    expected = info["id_count"]
    rows: list[dict] = []
    for i, va in enumerate(pushes[:expected]):
        sec, foff = _section_for_va(sections, va)
        if sec is None:
            continue
        s = _read_cstr(pe_data, foff)
        rows.append({
            "id": info["id_base"] + i,
            "ns": info["ns"],
            "paramname": s,
            "ptr_va": f"0x{va:08x}",
            "ptr_section": sec,
            "ptr_file_off": foff,
        })
    return rows


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", default="ffxivgame", nargs="?")
    args = ap.parse_args()
    stem = args.binary.replace(".exe", "")

    pe_path = ORIG / f"{stem}.exe"
    if not pe_path.exists():
        print(f"error: missing {pe_path}; run tools/symlink_orig.sh", file=sys.stderr)
        return 1
    pe_data, sections = _parse_pe(pe_path)

    all_rows: list[dict] = []
    asm_dir = ASM_ROOT / stem
    for cls, info in DISPATCHERS.items():
        glob_prefix = f"{info['rva']:08x}_"
        matches = list(asm_dir.glob(f"{glob_prefix}*.s"))
        if not matches:
            print(f"warning: dispatcher asm missing for {cls} at rva {info['rva']:#x}", file=sys.stderr)
            continue
        rows = extract_dispatcher(pe_data, sections, matches[0], cls, info)
        all_rows.extend(rows)
        print(f"  {cls}: {len(rows)} of {info['id_count']} resolved")

    # JSON dump (machine-readable).
    out_json = CONFIG / f"{stem}.paramnames_resolved.json"
    out_json.write_text(json.dumps(all_rows, indent=2))

    # Markdown report.
    WIRE.mkdir(parents=True, exist_ok=True)
    out_md = WIRE / f"{stem}.paramnames.md"
    by_ns: dict[str, list[dict]] = {}
    for r in all_rows:
        by_ns.setdefault(r["ns"], []).append(r)
    with out_md.open("w") as f:
        f.write(f"# {stem}.exe — resolved GAM PARAMNAME strings\n\n")
        f.write(f"Auto-generated by `tools/extract_paramnames_dispatch.py`.\n\n")
        f.write(f"Each Data class has a vtable slot in its `MetadataProvider`\n")
        f.write(f"that maps a GAM id (e.g. 100..125 for CharaMakeData) to a\n")
        f.write(f"property name string in `.data`. We walk that dispatcher's\n")
        f.write(f"asm, extract the per-id string pointers from `PUSH <imm32>`\n")
        f.write(f"instructions, and dereference them.\n\n")
        for ns, rows in sorted(by_ns.items()):
            cls = ns.split("::")[-1]
            f.write(f"## `{cls}` ({len(rows)} entries) — `{ns}`\n\n")
            f.write("| id | name |\n|---:|:---|\n")
            for r in rows:
                f.write(f"| {r['id']} | `{r['paramname']}` |\n")
            f.write("\n")

    # Enrich gam_params.json in-place with `paramname` field.
    gam_path = CONFIG / f"{stem}.gam_params.json"
    if gam_path.exists():
        gam = json.loads(gam_path.read_text())
        lookup = {(r["id"], r["ns"]): r["paramname"] for r in all_rows}
        enriched = 0
        for entry in gam:
            key = (entry["id"], entry["ns"])
            if key in lookup:
                entry["paramname"] = lookup[key]
                enriched += 1
        gam_path.write_text(json.dumps(gam, indent=2))
        print(f"  enriched {enriched} gam_params entries with resolved paramnames")

    print(f"wrote: {out_json.relative_to(REPO_ROOT)}")
    print(f"       {out_md.relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
