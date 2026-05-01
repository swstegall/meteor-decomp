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
    # Each entry: (dispatcher RVA, owning Data-class namespace, key kind).
    # The dispatcher is a vtable slot in the class's MetadataProvider
    # that maps a key (either the global GAM id or a local 0..N index)
    # to a property-name C-string in `.data` via a large unrolled switch.
    #
    # `key`:
    #   "global_id"     prologue normalises the global GAM id with
    #                   `ADD EAX, -<base>` then indexes a JT. The K-th
    #                   `.data` PUSH in source order corresponds to
    #                   the K-th real GAM id (sorted ascending). Gaps
    #                   in the id range share a single sentinel
    #                   handler whose PUSH targets `.rdata`, so they
    #                   collapse to one PUSH and don't shift alignment.
    #                   ✓ Validated for CharaMakeData (26/26) and
    #                   Player (92/92).
    #
    #   "local_index"   prologue has no `ADD EAX, -<base>`; it just
    #                   `CMP EAX, <count-1>` then `JMP [EAX*4 + JT]`.
    #                   The input is already a 0..N local index, NOT
    #                   the global GAM id. The mapping from global id
    #                   to local index lives in *another* slot (e.g.
    #                   PlayerPlayer slot 2). Without decompiling that
    #                   translator we don't know which name goes with
    #                   which GAM id, so we extract the names as an
    #                   ordered list keyed by local-index only and
    #                   skip the gam_params enrichment.
    "CharaMakeData": {
        "rva": 0x001ad010,
        "ns": "Application::Network::GameAttributeManager::Data::CharaMakeData",
        "key": "global_id",
    },
    "Player": {
        "rva": 0x001add90,
        "ns": "Application::Network::GameAttributeManager::Data::Player",
        "key": "global_id",
    },
    "PlayerPlayer": {
        # Slot 4 of MetadataProvider, 37-entry table, local-index keyed.
        # The 37 PUSHes recover the *names* (tribe, size, …,
        # actionSaveWork) but pairing them with GAM ids requires the
        # local-index → global-id mapping from slot 2 (RVA 0x001aee30,
        # 771 bytes — TBD).
        "rva": 0x001af270,
        "ns": "Application::Network::GameAttributeManager::Data::PlayerPlayer",
        "key": "local_index",
    },
    "ClientSelectData": {
        # Slot 2 of MetadataProvider — global-id keyed.
        # `ADD EAX, -0x64; CMP EAX, 0x13` → ids 100..119 (20-entry JT).
        # GAM has 17 ids in this range (gaps at 101, 105, 106 → sentinel).
        "rva": 0x001ad580,
        "ns": "Application::Network::GameAttributeManager::Data::ClientSelectData",
        "key": "global_id",
    },
    "ClientSelectDataN": {
        # Slot 2 of MetadataProvider — global-id keyed.
        # `ADD EAX, -0x64; CMP EAX, 0x10` → ids 100..116 (17-entry JT,
        # contiguous, no gaps).
        "rva": 0x001ad990,
        "ns": "Application::Network::GameAttributeManager::Data::ClientSelectDataN",
        "key": "global_id",
    },
    "ZoneInitData": {
        # Slot 2 of MetadataProvider — global-id keyed but with a
        # `SUB EAX, 0x64; JZ; SUB EAX, 1; JZ; SUB EAX, 1; JZ` chained-
        # compare cascade rather than a JT (more compact for 3 ids).
        # Case handlers still appear in id-sorted source order, so the
        # K-th `.data` PUSH still pairs with the K-th real GAM id.
        "rva": 0x001af640,
        "ns": "Application::Network::GameAttributeManager::Data::ZoneInitData",
        "key": "global_id",
    },
    # All known Data classes covered.
    # build/wire/<binary>.net_handlers.md under each ::MetadataProvider
    # section and pick the largest non-init slot, then check the
    # prologue to classify as "global_id" vs "local_index".
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
    real_ids: list[int],
) -> tuple[list[dict], list[dict]]:
    """Walk a dispatcher's asm, extract its `.data` string pointers from
    case-handler `PUSH <imm32>` instructions in source order. Returns
    `(by_id_rows, by_local_rows)`:

      by_id_rows   one entry per resolved (id, ns, paramname) — populated
                   only for global_id-keyed dispatchers where the K-th
                   PUSH reliably maps to the K-th sorted real GAM id.
      by_local_rows  one entry per local index, populated for both kinds
                   (local-index-keyed dispatchers populate ONLY this).
    """
    asm = asm_path.read_text()

    pushes: list[int] = []
    for m in re.finditer(r"^\s*[0-9a-f]+:\s+(?:[0-9a-f][0-9a-f] )+\s+PUSH (0x[0-9a-fA-F]+)$", asm, re.MULTILINE):
        va = int(m.group(1), 16)
        sec, _foff = _section_for_va(sections, va)
        if sec == ".data":
            pushes.append(va)

    by_id: list[dict] = []
    by_local: list[dict] = []
    for k, va in enumerate(pushes):
        sec, foff = _section_for_va(sections, va)
        if sec is None:
            continue
        s = _read_cstr(pe_data, foff)
        by_local.append({
            "ns": info["ns"],
            "local_index": k,
            "paramname": s,
            "ptr_va": f"0x{va:08x}",
        })

    if info["key"] == "global_id":
        if len(pushes) != len(real_ids):
            print(
                f"warning: {name} has {len(pushes)} .data PUSHes but {len(real_ids)} GAM ids — alignment may be off",
                file=sys.stderr,
            )
        for real_id, va in zip(real_ids, pushes):
            sec, foff = _section_for_va(sections, va)
            if sec is None:
                continue
            s = _read_cstr(pe_data, foff)
            by_id.append({
                "id": real_id,
                "ns": info["ns"],
                "paramname": s,
                "ptr_va": f"0x{va:08x}",
                "ptr_section": sec,
                "ptr_file_off": foff,
            })
    return (by_id, by_local)


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

    # Load gam_params.json so we can look up the real GAM ids per
    # namespace — the dispatcher's case handlers are sorted by id, and
    # we need to map the K-th .data PUSH to the K-th real id (skipping
    # gaps in the id sequence).
    gam_path = CONFIG / f"{stem}.gam_params.json"
    if not gam_path.exists():
        print(f"error: missing {gam_path}; run extract_gam_params.py first", file=sys.stderr)
        return 1
    gam = json.loads(gam_path.read_text())
    ids_by_ns: dict[str, list[int]] = {}
    for entry in gam:
        ids_by_ns.setdefault(entry["ns"], []).append(entry["id"])
    for ns_ids in ids_by_ns.values():
        ns_ids.sort()

    all_rows: list[dict] = []           # by GAM id (only global_id dispatchers)
    all_local_rows: list[dict] = []     # by local index (every dispatcher)
    asm_dir = ASM_ROOT / stem
    for cls, info in DISPATCHERS.items():
        glob_prefix = f"{info['rva']:08x}_"
        matches = list(asm_dir.glob(f"{glob_prefix}*.s"))
        if not matches:
            print(f"warning: dispatcher asm missing for {cls} at rva {info['rva']:#x}", file=sys.stderr)
            continue
        real_ids = ids_by_ns.get(info["ns"], [])
        if not real_ids:
            print(f"warning: no GAM ids found for namespace {info['ns']}", file=sys.stderr)
            continue
        by_id, by_local = extract_dispatcher(pe_data, sections, matches[0], cls, info, real_ids)
        all_rows.extend(by_id)
        all_local_rows.extend(by_local)
        if info["key"] == "global_id":
            print(f"  {cls}: {len(by_id)} of {len(real_ids)} GAM ids resolved (id-keyed)")
        else:
            print(f"  {cls}: {len(by_local)} names extracted (local-index-keyed; "
                  f"GAM-id pairing TBD — see slot 2 translator)")

    # JSON dump (machine-readable).
    out_json = CONFIG / f"{stem}.paramnames_resolved.json"
    out_json.write_text(json.dumps(all_rows, indent=2))

    # Markdown report.
    WIRE.mkdir(parents=True, exist_ok=True)
    out_md = WIRE / f"{stem}.paramnames.md"
    by_ns_id: dict[str, list[dict]] = {}
    for r in all_rows:
        by_ns_id.setdefault(r["ns"], []).append(r)
    by_ns_local: dict[str, list[dict]] = {}
    for r in all_local_rows:
        by_ns_local.setdefault(r["ns"], []).append(r)
    with out_md.open("w") as f:
        f.write(f"# {stem}.exe — resolved GAM PARAMNAME strings\n\n")
        f.write(f"Auto-generated by `tools/extract_paramnames_dispatch.py`.\n\n")
        f.write(f"Each Data class has a vtable slot in its `MetadataProvider`\n")
        f.write(f"that maps either a GAM id or a local 0..N index to a\n")
        f.write(f"property-name string in `.data`. We walk that dispatcher's\n")
        f.write(f"asm, extract the per-key string pointers from `PUSH <imm32>`\n")
        f.write(f"instructions, and dereference them.\n\n")
        f.write(f"## Id-keyed dispatchers (GAM id → name)\n\n")
        for ns, rows in sorted(by_ns_id.items()):
            cls = ns.split("::")[-1]
            f.write(f"### `{cls}` ({len(rows)} entries) — `{ns}`\n\n")
            f.write("| id | name |\n|---:|:---|\n")
            for r in rows:
                f.write(f"| {r['id']} | `{r['paramname']}` |\n")
            f.write("\n")
        f.write(f"## Local-index-keyed dispatchers (no GAM-id pairing yet)\n\n")
        f.write(f"These dispatchers take a 0..N local-index input rather\n")
        f.write(f"than a global GAM id. The local→global mapping lives in\n")
        f.write(f"a separate slot (typically slot 2 of the same\n")
        f.write(f"MetadataProvider) — TBD to decompile. For now, names are\n")
        f.write(f"listed in dispatcher source order without GAM-id pairing.\n\n")
        for ns, rows in sorted(by_ns_local.items()):
            if ns in by_ns_id:
                continue  # already shown id-keyed
            cls = ns.split("::")[-1]
            f.write(f"### `{cls}` ({len(rows)} entries) — `{ns}`\n\n")
            f.write("| local idx | name |\n|---:|:---|\n")
            for r in rows:
                f.write(f"| {r['local_index']} | `{r['paramname']}` |\n")
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
