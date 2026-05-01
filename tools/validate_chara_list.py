#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Cross-validate garlemald-server's `build_for_chara_list`
(lobby-server/src/data/chara_info.rs) against the binary's GAM
ClientSelectData / ClientSelectDataN registries.

UNLIKE `parse_new_char_request` (which decodes a GAM-serialised
chara-make request), `build_for_chara_list` is a hand-rolled flat
blob — Project Meteor reverse-engineered the CharacterListPacket
wire format and garlemald ports it. The wire format is not a
direct (id, value) GAM serialisation; it has its own packing
(face → u32 bitfield, hair → u32, color → u32, equipment → 21
sequential u32s) plus header / magic constants the binary expects.

So this validator is *schema-level*, not byte-layout validation:
  - Each garlemald write is paired with its semantic GAM field.
  - Type mismatches between the Rust write and the GAM type flag
    likely bugs (e.g. writing u16 where GAM says signed char).
  - GAM fields garlemald doesn't write are noted (could be
    intentional — the wire format may not include every GAM
    field).
  - Garlemald writes with no GAM counterpart are noted (mostly
    equipment slots, which are presumably packed into the
    `graphics: int[0]` variable-length array in the GAM schema).

The byte-layout question (does the binary's CharacterListPacket
deserializer accept this exact byte sequence?) requires reading
the binary's deserializer — TBD.

Output:
  build/wire/<binary>.chara_list_validation.md
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CONFIG = REPO_ROOT / "config"
WIRE = REPO_ROOT / "build" / "wire"

# Garlemald's build_for_chara_list write sequence, transcribed by
# hand from lobby-server/src/data/chara_info.rs (lines 116-195).
# `gam_csd` / `gam_csdn`: the CSD or CSDataN GAM id this write maps
# to semantically (None for header / packing / equipment / magic).
GARLEMALD_WRITES: list[dict] = [
    {"name": "header_magic_1",          "rs_type": "u32 (=0x4c0)",            "byte_size": 4, "gam_csd": None,            "kind": "magic"},
    {"name": "header_magic_2",          "rs_type": "u32 (=0x232327ea)",       "byte_size": 4, "gam_csd": None,            "kind": "magic"},
    {"name": "name_length",             "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("100", "displayName"), "kind": "length"},
    {"name": "name_bytes",              "rs_type": "bytes (UTF-8 + NUL)",     "byte_size": 0, "gam_csd": ("100", "displayName"), "kind": "field"},
    {"name": "constant_0x1c",           "rs_type": "u32 (=0x1c)",             "byte_size": 4, "gam_csd": None,            "kind": "magic"},
    {"name": "constant_0x04",           "rs_type": "u32 (=0x04)",             "byte_size": 4, "gam_csd": None,            "kind": "magic"},
    {"name": "tribe_model",             "rs_type": "u32 (get_tribe_model)",   "byte_size": 4, "gam_csd": None,            "kind": "packed"},
    {"name": "size",                    "rs_type": "u32",                     "byte_size": 4, "gam_csd": None,            "kind": "field"},
    {"name": "color_val",               "rs_type": "u32 (skin|hair<<10|eye<<20)", "byte_size": 4, "gam_csd": None,        "kind": "packed"},
    {"name": "face.to_u32()",           "rs_type": "u32 (FaceInfo bitfield)", "byte_size": 4, "gam_csd": None,            "kind": "packed"},
    {"name": "hair_val",                "rs_type": "u32 (hl|var<<5|sty<<10)", "byte_size": 4, "gam_csd": None,            "kind": "packed"},
    {"name": "voice",                   "rs_type": "u32",                     "byte_size": 4, "gam_csd": None,            "kind": "field"},
    # Equipment slots — 22 u32s. Likely correspond to GAM `graphics: int[0]`
    # (variable-length array). Garlemald writes them as fixed positional
    # slots, which the binary's deserializer presumably re-serialises
    # back into the GAM `graphics` field.
    {"name": "main_hand",               "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("102", "graphics[]"), "kind": "graphics"},
    {"name": "off_hand",                "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("102", "graphics[]"), "kind": "graphics"},
    {"name": "5x zero",                 "rs_type": "u32 x 5 (=0)",            "byte_size": 20, "gam_csd": ("102", "graphics[]"), "kind": "graphics"},
    {"name": "head",                    "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("102", "graphics[]"), "kind": "graphics"},
    {"name": "body",                    "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("102", "graphics[]"), "kind": "graphics"},
    {"name": "legs",                    "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("102", "graphics[]"), "kind": "graphics"},
    {"name": "hands",                   "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("102", "graphics[]"), "kind": "graphics"},
    {"name": "feet",                    "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("102", "graphics[]"), "kind": "graphics"},
    {"name": "waist",                   "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("102", "graphics[]"), "kind": "graphics"},
    {"name": "neck",                    "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("102", "graphics[]"), "kind": "graphics"},
    {"name": "right_ear",               "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("102", "graphics[]"), "kind": "graphics"},
    {"name": "left_ear",                "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("102", "graphics[]"), "kind": "graphics"},
    {"name": "right_index",             "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("102", "graphics[]"), "kind": "graphics"},
    {"name": "left_index",              "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("102", "graphics[]"), "kind": "graphics"},
    {"name": "right_finger",            "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("102", "graphics[]"), "kind": "graphics"},
    {"name": "left_finger",             "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("102", "graphics[]"), "kind": "graphics"},
    {"name": "8x zero byte",            "rs_type": "u8 x 8 (=0)",             "byte_size": 8, "gam_csd": None,            "kind": "padding"},
    {"name": "constant_1_a",            "rs_type": "u32 (=1)",                "byte_size": 4, "gam_csd": ("103", "loginFlag (low half?)"), "kind": "loginFlag"},
    {"name": "constant_1_b",            "rs_type": "u32 (=1)",                "byte_size": 4, "gam_csd": ("103", "loginFlag (high half?)"), "kind": "loginFlag"},
    {"name": "current_class",           "rs_type": "u8",                      "byte_size": 1, "gam_csd": ("104", "mainSkill"),   "kind": "field"},
    {"name": "current_level",           "rs_type": "u16",                     "byte_size": 2, "gam_csd": ("107", "mainSkillLevel"), "kind": "type_mismatch"},
    {"name": "current_job",             "rs_type": "u8",                      "byte_size": 1, "gam_csd": ("119", "currentJob"),  "kind": "field"},
    {"name": "constant_1_c",            "rs_type": "u16 (=1)",                "byte_size": 2, "gam_csd": None,            "kind": "magic"},
    {"name": "tribe",                   "rs_type": "u8",                      "byte_size": 1, "gam_csd": ("109", "tribe"),       "kind": "type_mismatch"},
    {"name": "magic_0xe22222aa",        "rs_type": "u32",                     "byte_size": 4, "gam_csd": None,            "kind": "magic"},
    {"name": "location1_length",        "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("110", "zoneName"),    "kind": "length"},
    {"name": "location1_bytes",         "rs_type": "bytes (\"prv0Inn01\\0\")",  "byte_size": 0, "gam_csd": ("110", "zoneName"),    "kind": "type_mismatch"},
    {"name": "location2_length",        "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("111", "territoryName"), "kind": "length"},
    {"name": "location2_bytes",         "rs_type": "bytes (\"defaultTerritory\\0\")", "byte_size": 0, "gam_csd": ("111", "territoryName"), "kind": "type_mismatch"},
    {"name": "guardian",                "rs_type": "u8",                      "byte_size": 1, "gam_csd": ("112", "guardian"),    "kind": "field"},
    {"name": "birth_month",             "rs_type": "u8",                      "byte_size": 1, "gam_csd": ("113", "birthdayMonth"), "kind": "field"},
    {"name": "birth_day",               "rs_type": "u8",                      "byte_size": 1, "gam_csd": ("114", "birthdayDay"),  "kind": "field"},
    {"name": "magic_0x17",              "rs_type": "u16",                     "byte_size": 2, "gam_csd": None,            "kind": "magic"},
    {"name": "constant_4_a",            "rs_type": "u32 (=4)",                "byte_size": 4, "gam_csd": None,            "kind": "magic"},
    {"name": "constant_4_b",            "rs_type": "u32 (=4)",                "byte_size": 4, "gam_csd": None,            "kind": "magic"},
    {"name": "(seek 0x10)",             "rs_type": "skip",                    "byte_size": 16, "gam_csd": None,           "kind": "padding"},
    {"name": "initial_town_a",          "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("118", "initialTown"),  "kind": "type_mismatch"},
    {"name": "initial_town_b",          "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("118", "initialTown duplicate"), "kind": "duplicate"},
]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", default="ffxivgame", nargs="?")
    args = ap.parse_args()
    stem = args.binary.replace(".exe", "")

    src = CONFIG / f"{stem}.gam_params.json"
    if not src.exists():
        print(f"error: missing {src}; run extract_gam_params.py + extract_paramnames_dispatch.py first", file=sys.stderr)
        return 1
    rows = json.loads(src.read_text())
    csd_ns = "Application::Network::GameAttributeManager::Data::ClientSelectData"
    csd_by_id = {r["id"]: r for r in rows if r["ns"] == csd_ns}

    WIRE.mkdir(parents=True, exist_ok=True)
    out = WIRE / f"{stem}.chara_list_validation.md"

    # Track which GAM fields are referenced.
    csd_referenced: set[int] = set()
    bugs: list[str] = []

    for w in GARLEMALD_WRITES:
        gam = w["gam_csd"]
        if gam is None:
            continue
        gid = int(gam[0])
        csd_referenced.add(gid)

        if w["kind"] == "type_mismatch":
            r = csd_by_id.get(gid)
            if r is None:
                continue
            bugs.append(f"`{w['name']}` ({w['rs_type']}) vs GAM id {gid} `{r.get('paramname','?')}`: `{r['type']}`")

    csd_unreferenced = [r for r in csd_by_id.values() if r["id"] not in csd_referenced]

    with out.open("w") as f:
        f.write(f"# {stem}.exe — `build_for_chara_list` ↔ GAM ClientSelectData\n\n")
        f.write("Auto-generated by `tools/validate_chara_list.py`. Compares\n")
        f.write("`garlemald-server/lobby-server/src/data/chara_info.rs::build_for_chara_list`\n")
        f.write(f"against the binary's `{csd_ns}` GAM registry.\n\n")
        f.write("## What this report does (and doesn't) prove\n\n")
        f.write("The chara-list response is a HAND-ROLLED wire format —\n")
        f.write("Project Meteor reverse-engineered `CharacterListPacket`\n")
        f.write("with magic headers / packed bitfields / fixed equipment\n")
        f.write("slots and garlemald ports it directly. It is *not* a GAM\n")
        f.write("(id, value) self-describing serialisation.\n\n")
        f.write("So this validator is **schema-level**, not byte-layout\n")
        f.write("validation. It pairs each Rust write with its closest GAM\n")
        f.write("field (where one exists) and flags type mismatches that\n")
        f.write("are likely bugs. The byte-layout question (does the\n")
        f.write("binary's `CharacterListPacket` deserializer actually\n")
        f.write("accept this byte sequence?) requires decompiling the\n")
        f.write("deserializer — TBD.\n\n")
        f.write("## Likely bugs\n\n")
        if bugs:
            for b in bugs:
                f.write(f"- {b}\n")
            f.write("\n### Detail\n\n")
            f.write(
                "1. **`current_level: u16` vs GAM `mainSkillLevel: signed char` (1 byte).**\n"
                "   Garlemald writes 2 bytes where the GAM schema declares 1. If the\n"
                "   binary deserializer reads exactly 1 byte, the second byte of\n"
                "   garlemald's u16 leaks into whatever follows (`current_job` etc.),\n"
                "   shifting the rest of the blob by 1.\n\n"
                "2. **`tribe: u8` vs GAM `tribe: Sqex::Misc::Utf8String`.**\n"
                "   GAM declares the wire format as a UTF-8 string (length-prefixed\n"
                "   localised display name like \"Hyur Midlander\"), garlemald writes\n"
                "   a 1-byte numeric tribe id. Likely the chara-list packet expects\n"
                "   the display string here, not the raw id (the raw id was already\n"
                "   sent earlier via `tribe_model` u32). If so, the chara-select\n"
                "   screen may render the tribe row blank or as garbled text.\n\n"
                "3. **`location1: length-prefixed string` vs GAM `zoneName: signed char`.**\n"
                "   GAM declares zoneName as a 1-byte zone id; garlemald writes the\n"
                "   full ASCII string \"prv0Inn01\\0\" length-prefixed. **Suspect this\n"
                "   one less** — it's possible Project Meteor reverse-engineered\n"
                "   correctly that the chara-list response uses raw strings here\n"
                "   (the binary's GAM schema for in-memory use is a 1-byte id, but\n"
                "   the WIRE may transport the resolved string for client-side\n"
                "   display). Decompiling `CharacterListPacket::Deserialize` would\n"
                "   confirm.\n\n"
                "4. **`location2: length-prefixed string` vs GAM `territoryName: signed char`.**\n"
                "   Same shape as #3. Same caveat.\n\n"
                "5. **`initial_town: u32 (twice)` vs GAM `initialTown: short` (2 bytes, once).**\n"
                "   Two issues compounded:\n"
                "   - Type width: u32 vs i16. 2× over-write.\n"
                "   - Duplicate write: garlemald writes `initial_town` twice in a row\n"
                "     (lines 194-195). Probably one of these is a separate field\n"
                "     entirely (e.g. `favourite_aetheryte` or a fall-back town id)\n"
                "     and the duplicate is a port bug from the C# original.\n"
            )
        else:
            f.write("(no type mismatches detected — schema looks consistent)\n")

        f.write("\n## Side-by-side\n\n")
        f.write("| garlemald write | rs type | bytes | GAM id | binary name | type | status |\n")
        f.write("|---|---|---:|---:|---|---|---|\n")
        for w in GARLEMALD_WRITES:
            gam = w["gam_csd"]
            if gam is not None and gam[0].isdigit():
                gid = int(gam[0])
                r = csd_by_id.get(gid)
                if r is not None:
                    bn = r.get("paramname", "?")
                    gt = r["type"]
                    gid_str = str(gid)
                else:
                    bn = gam[1]; gt = "—"; gid_str = gam[0]
            else:
                bn = "—"; gt = "—"; gid_str = "—"
            status = _status(w["kind"])
            f.write(f"| `{w['name']}` | `{w['rs_type']}` | {w['byte_size']} | "
                    f"{gid_str} | `{bn}` | `{gt}` | {status} |\n")

        f.write("\n## GAM ClientSelectData fields garlemald doesn't write\n\n")
        if csd_unreferenced:
            f.write("These CSD GAM fields appear in the binary's schema but have no\n")
            f.write("corresponding Rust write in `build_for_chara_list`. May be\n")
            f.write("intentional (the wire format may not include every GAM field),\n")
            f.write("or may be missing data. Decompile `CharacterListPacket::Deserialize`\n")
            f.write("to confirm.\n\n")
            f.write("| GAM id | name | type |\n|---:|---|---|\n")
            for r in sorted(csd_unreferenced, key=lambda x: x["id"]):
                f.write(f"| {r['id']} | `{r.get('paramname','?')}` | `{r['type']}` |\n")
        else:
            f.write("(every CSD GAM field is referenced)\n")

        f.write("\n## Suggested patch (Rust pseudocode)\n\n")
        f.write("```rust\n")
        f.write("// In build_for_chara_list:\n\n")
        f.write("// (1) current_level should be u8, not u16:\n")
        f.write("c.write_u8(chara.current_level as u8).unwrap();\n\n")
        f.write("// (2) the trailing duplicate initial_town write is suspect — the\n")
        f.write("//     second u32 likely corresponds to a different field (favourite\n")
        f.write("//     aetheryte? legacy fallback?). Until the deserializer is\n")
        f.write("//     decompiled, leave the write but flag with a TODO comment:\n")
        f.write("c.write_u32::<LittleEndian>(chara.initial_town as u32).unwrap();\n")
        f.write("// TODO(meteor-decomp): second initial_town write is a duplicate\n")
        f.write("// in Project Meteor's C#; decompile CharacterListPacket::Deserialize\n")
        f.write("// to confirm whether this slot is a separate field.\n")
        f.write("c.write_u32::<LittleEndian>(chara.initial_town as u32).unwrap();\n\n")
        f.write("// (3) tribe — GAM declares it as Utf8String. The wire may\n")
        f.write("//     correctly transport the raw byte; or it may need a localised\n")
        f.write("//     string. Project Meteor's C# also writes the byte form, so\n")
        f.write("//     leave it for now but flag for verification.\n")
        f.write("```\n")

    print(f"wrote: {out.relative_to(REPO_ROOT)}")
    return 0


def _status(kind: str) -> str:
    return {
        "magic": "outside GAM (header/magic)",
        "field": "ok",
        "length": "length prefix (Utf8String wire form)",
        "packed": "outside GAM (packed bitfield, not 1:1)",
        "graphics": "presumably part of `graphics` (int[0])",
        "padding": "non-GAM padding",
        "loginFlag": "u32+u32 = u64 (matches GAM `loginFlag`)",
        "type_mismatch": "**LIKELY BUG** — width/type doesn't match GAM",
        "duplicate": "**LIKELY BUG** — garlemald writes initial_town twice",
    }.get(kind, "?")


if __name__ == "__main__":
    sys.exit(main())
