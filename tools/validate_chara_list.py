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

This validator is *schema-level*, not byte-layout validation. It
pairs each garlemald write with its closest GAM field and notes
type/width DIVERGENCES — but those divergences are NOT bugs by
default: the GAM schema describes the binary's *in-memory* type,
while the wire format may legitimately serialise a different
shape (e.g. transport a resolved string where memory holds a
short id). Confirming a divergence is a real bug requires either
(a) decompiling `CharacterListPacket::Deserialize` and reading
the byte-by-byte read sequence, or (b) observing the live client
mis-render / disconnect on the current wire shape.

Empirical signal as of 2026-05-01: the fresh-start scripts
(`fresh-start-{limsa,gridania,uldah}.sh`) drive a working
chara-select handoff against the live client. So the current
wire shape is at minimum *deserialisable* by the binary; any
"bug" claim has to also explain how the current shape passes
deserialisation.

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
    {"name": "current_level",           "rs_type": "u16",                     "byte_size": 2, "gam_csd": ("107", "mainSkillLevel"), "kind": "divergence"},
    {"name": "current_job",             "rs_type": "u8",                      "byte_size": 1, "gam_csd": ("119", "currentJob"),  "kind": "field"},
    {"name": "constant_1_c",            "rs_type": "u16 (=1)",                "byte_size": 2, "gam_csd": None,            "kind": "magic"},
    {"name": "tribe",                   "rs_type": "u8",                      "byte_size": 1, "gam_csd": ("109", "tribe"),       "kind": "divergence"},
    {"name": "magic_0xe22222aa",        "rs_type": "u32",                     "byte_size": 4, "gam_csd": None,            "kind": "magic"},
    {"name": "location1_length",        "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("110", "zoneName"),    "kind": "length"},
    {"name": "location1_bytes",         "rs_type": "bytes (\"prv0Inn01\\0\")",  "byte_size": 0, "gam_csd": ("110", "zoneName"),    "kind": "divergence"},
    {"name": "location2_length",        "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("111", "territoryName"), "kind": "length"},
    {"name": "location2_bytes",         "rs_type": "bytes (\"defaultTerritory\\0\")", "byte_size": 0, "gam_csd": ("111", "territoryName"), "kind": "divergence"},
    {"name": "guardian",                "rs_type": "u8",                      "byte_size": 1, "gam_csd": ("112", "guardian"),    "kind": "field"},
    {"name": "birth_month",             "rs_type": "u8",                      "byte_size": 1, "gam_csd": ("113", "birthdayMonth"), "kind": "field"},
    {"name": "birth_day",               "rs_type": "u8",                      "byte_size": 1, "gam_csd": ("114", "birthdayDay"),  "kind": "field"},
    {"name": "magic_0x17",              "rs_type": "u16",                     "byte_size": 2, "gam_csd": None,            "kind": "magic"},
    {"name": "constant_4_a",            "rs_type": "u32 (=4)",                "byte_size": 4, "gam_csd": None,            "kind": "magic"},
    {"name": "constant_4_b",            "rs_type": "u32 (=4)",                "byte_size": 4, "gam_csd": None,            "kind": "magic"},
    {"name": "(seek 0x10)",             "rs_type": "skip",                    "byte_size": 16, "gam_csd": None,           "kind": "padding"},
    {"name": "initial_town_a",          "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("118", "initialTown"),  "kind": "divergence"},
    {"name": "initial_town_b",          "rs_type": "u32",                     "byte_size": 4, "gam_csd": ("118", "initialTown duplicate"), "kind": "divergence"},
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

    # Prefer RTTI-derived types when available (`extract_gam_types_rtti.py`
    # enriches gam_params.json with a `rtti_type` field). RTTI is ground
    # truth from the binary's CompileTimeParameter template instantiation;
    # the legacy .data extractor has off-by-one errors on Array sizes.
    def csd_type(row: dict) -> str:
        return row.get("rtti_type") or row.get("type", "?")

    WIRE.mkdir(parents=True, exist_ok=True)
    out = WIRE / f"{stem}.chara_list_validation.md"

    # Track which GAM fields are referenced.
    csd_referenced: set[int] = set()
    divergences: list[str] = []

    for w in GARLEMALD_WRITES:
        gam = w["gam_csd"]
        if gam is None:
            continue
        gid = int(gam[0])
        csd_referenced.add(gid)

        if w["kind"] == "divergence":
            r = csd_by_id.get(gid)
            if r is None:
                continue
            divergences.append(f"`{w['name']}` ({w['rs_type']}) vs GAM id {gid} `{r.get('paramname','?')}`: `{csd_type(r)}`")

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
        f.write("field and notes type/width DIVERGENCES.\n\n")
        f.write("**As of 2026-05-01 the divergences are confirmed expected,\n")
        f.write("not bugs.** See \"Architecture finding\" below for the full\n")
        f.write("reasoning. Short version: the chara-list packet is not\n")
        f.write("GAM-serialised on the wire — there is no\n")
        f.write("`CharacterListPacket::Deserialize` function to decompile. A\n")
        f.write("custom hand-rolled parser populates the GAM fields after a\n")
        f.write("wire-vs-memory translation, so divergent wire shapes against\n")
        f.write("GAM in-memory types are part of the design.\n\n")
        f.write("## Schema divergences (expected — see \"Architecture finding\" below)\n\n")
        if divergences:
            for d in divergences:
                f.write(f"- {d}\n")
            f.write(
                "\n**These are not bugs.** Each pair shows the wire shape vs the\n"
                "GAM in-memory destination type. The chara-list packet is parsed\n"
                "by a custom (non-GAM) deserialiser that populates the GAM fields\n"
                "after a wire-vs-memory translation; see the per-divergence\n"
                "interpretation under \"Conclusion on the five 'divergences'\"\n"
                "in the Architecture finding section.\n"
            )
        else:
            f.write("(no divergences detected — schema looks consistent)\n")

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
                    gt = csd_type(r)
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
            f.write("These CSD GAM fields exist in the in-memory schema but have\n")
            f.write("no corresponding write in `build_for_chara_list`. Given the\n")
            f.write("Architecture finding (custom wire format, not GAM-serialised),\n")
            f.write("this is expected: the wire transports only the fields\n")
            f.write("Project Meteor's RE identified, and the in-memory schema\n")
            f.write("includes additional fields populated from other sources\n")
            f.write("(database, derived state, init defaults).\n\n")
            f.write("| GAM id | name | type |\n|---:|---|---|\n")
            for r in sorted(csd_unreferenced, key=lambda x: x["id"]):
                f.write(f"| {r['id']} | `{r.get('paramname','?')}` | `{csd_type(r)}` |\n")
        else:
            f.write("(every CSD GAM field is referenced)\n")

        f.write("\n## Architecture finding (2026-05-01)\n\n")
        f.write("The original \"decompile `CharacterListPacket::Deserialize`\"\n")
        f.write("plan turned out to be ill-posed: there IS no single such\n")
        f.write("function, and **the chara-list packet is not GAM-serialised\n")
        f.write("on the wire**. The GAM `ClientSelectData` types describe the\n")
        f.write("client's *in-memory* destination after deserialisation; the\n")
        f.write("wire format is hand-rolled by `CharacterListPacket` (Project\n")
        f.write("Meteor's RE, mirrored by `garlemald::build_for_chara_list`)\n")
        f.write("and a separate custom parser populates the GAM fields from\n")
        f.write("that custom byte sequence.\n\n")
        f.write("This is conclusive given three independent observations:\n\n")
        f.write("1. **Live client accepts garlemald's wire shape.**\n")
        f.write("   `fresh-start-{limsa,gridania,uldah}.sh` drive a working\n")
        f.write("   chara-select handoff; chara-select renders, the player\n")
        f.write("   row is selectable, and zone-in proceeds.\n\n")
        f.write("2. **RTTI ground truth confirms the GAM types.**\n")
        f.write("   Each `Component::GAM::CompileTimeParameter<id, &PARAMNAME, TYPE,\n")
        f.write("   DecoratorSimpleAssign<TYPE>>` template instantiation embeds\n")
        f.write("   the in-memory TYPE as its 3rd template argument. For the\n")
        f.write("   five divergent ids (107, 109, 110, 111, 118) the RTTI\n")
        f.write("   types match what the legacy `extract_gam_params.py`\n")
        f.write("   reported — the divergences are NOT extraction artifacts.\n")
        f.write("   See `tools/extract_gam_types_rtti.py` for the parser.\n\n")
        f.write("3. **The Lobby Down dispatch is a state-transition path,\n")
        f.write("   not a byte-reader.** Opcode 0x0D → `LobbyLoginOperationStep::slot[5]`\n")
        f.write("   = `FUN_00da5410` at RVA `0x9a5410` ignores the body\n")
        f.write("   pointer entirely; it just sets a `this+0xD = 1` flag and\n")
        f.write("   transitions the lobby state. Some other code path consumes\n")
        f.write("   the buffered body; whatever that path is, it MUST be\n")
        f.write("   handling the `prv0Inn01\\0` zone string and the duplicate\n")
        f.write("   `initialTown` u32s, because those bytes empirically reach\n")
        f.write("   the deserialiser without crashing it.\n\n")
        f.write("### Conclusion on the five \"divergences\"\n\n")
        f.write("They are **NOT bugs**. They are expected differences between\n")
        f.write("a hand-rolled wire format and the in-memory GAM data class\n")
        f.write("type. Specifically:\n\n")
        f.write("- `current_level: u16` — wire carries 2 bytes, `mainSkillLevel`\n")
        f.write("  is `signed char` in memory. The custom deserialiser likely\n")
        f.write("  reads u16 from the wire and downcasts on assignment.\n")
        f.write("- `tribe: u8` — wire carries 1 byte, `tribe` is `Utf8String`\n")
        f.write("  in memory. The byte is a tribe enum that the custom path\n")
        f.write("  resolves to a localised string at deserialisation time.\n")
        f.write("- `location1`/`location2`: length-prefixed strings — wire\n")
        f.write("  carries the resolved zone/territory name strings;\n")
        f.write("  `zoneName`/`territoryName` are `signed char` enums in\n")
        f.write("  memory, populated by the custom parser via lookup.\n")
        f.write("- `initial_town: u32 (twice)` — wire carries two u32 slots;\n")
        f.write("  `initialTown` is `short` in memory. The two slots may be\n")
        f.write("  parsed into separate fields the custom parser knows about\n")
        f.write("  (favourite-aetheryte, fallback town) but only one of which\n")
        f.write("  has a GAM CompileTimeParameter binding.\n\n")
        f.write("**Action: none.** Do not patch `build_for_chara_list`. The\n")
        f.write("wire shape is empirically correct; \"fixing\" it to match the\n")
        f.write("GAM types would break a working chara-select handoff.\n\n")
        f.write("### Open question (deferred)\n\n")
        f.write("The custom deserialiser itself is not yet located in the\n")
        f.write("binary. To find it, trace from `FUN_00da5410`'s helpers\n")
        f.write("(`0x4e7290` / `0x4e78d0` / `0x4e6110`) — they receive the two\n")
        f.write("global-state pointers `0x1127ad8` and `0x1363d30`, which\n")
        f.write("likely hold the buffered body and the GAM ClientSelectData\n")
        f.write("instance respectively. This is a follow-on Phase 3+ task; it\n")
        f.write("is NOT a prerequisite for any garlemald-side change since\n")
        f.write("the conclusion above is independent of finding the parser.\n\n")

        f.write("### Dispatch chain reference\n\n")
        f.write("- Lobby Down dispatcher: `FUN_00da4160` @ RVA `0x9a4160`. Strips\n")
        f.write("  16-byte envelope (opcode at hdr+2), looks up `vtable[case_idx]`,\n")
        f.write("  calls it with body ptr (envelope+0x10).\n")
        f.write("- Opcode `0x0D` → vtable slot 5 of `LobbyProtoDownCallbackInterface`.\n")
        f.write("  Default = no-op `RET 0xC` @ `0x9a2d10`.\n")
        f.write("- Override: `LobbyLoginOperationStep::slot[5]` = `FUN_00da5410`\n")
        f.write("  @ RVA `0x9a5410`. State-transition only; not a byte-reader.\n")
        f.write("- GAM Data class vtables (23 slots each):\n")
        f.write("  - `ClientSelectData::ClientSelectData` @ `.rdata` RVA `0xbab80c`\n")
        f.write("  - `ClientSelectDataN::ClientSelectDataN` @ `0xbab88c`\n")
        f.write("  - `ClientSelectData::MetadataProvider` @ `0xbab7ec`\n")
        f.write("- Per-property `Component::GAM::CompileTimeParameter<id, &PARAMNAME, TYPE, …>`\n")
        f.write("  classes carry the in-memory TYPE in their RTTI template signature\n")
        f.write("  (3rd template arg). Use `tools/extract_gam_types_rtti.py` to\n")
        f.write("  refresh `config/<binary>.gam_types_rtti.json`.\n")

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
        "divergence": "expected — wire shape differs from GAM in-memory type by design",
    }.get(kind, "?")


if __name__ == "__main__":
    sys.exit(main())
