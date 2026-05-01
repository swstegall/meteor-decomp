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
            divergences.append(f"`{w['name']}` ({w['rs_type']}) vs GAM id {gid} `{r.get('paramname','?')}`: `{r['type']}`")

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
        f.write("field (where one exists) and flags type/width DIVERGENCES.\n")
        f.write("Divergences are NOT bugs by default: the GAM schema describes\n")
        f.write("the binary's *in-memory* type, while the wire format may\n")
        f.write("legitimately serialise a different shape (e.g. transport a\n")
        f.write("resolved string where memory holds a short id). Promoting a\n")
        f.write("divergence to a confirmed bug requires decomp evidence from\n")
        f.write("`CharacterListPacket::Deserialize`.\n\n")
        f.write("## Live-client status (as of 2026-05-01)\n\n")
        f.write("`fresh-start-{limsa,gridania,uldah}.sh` drive a successful\n")
        f.write("chara-select handoff against the live 1.23b client. Empirically\n")
        f.write("the current wire shape is *accepted* by the binary's\n")
        f.write("deserializer; the chara-select screen renders, the player\n")
        f.write("is selectable, and zone-in proceeds. Any divergence below\n")
        f.write("must explain how the current shape passes deserialisation\n")
        f.write("before it can be classified as a bug.\n\n")
        f.write("## Schema divergences (unconfirmed; need decomp)\n\n")
        if divergences:
            for d in divergences:
                f.write(f"- {d}\n")
            f.write("\n### Detail\n\n")
            f.write(
                "Each item below pairs the on-the-wire Rust shape with the GAM\n"
                "in-memory type. None are confirmed bugs given the live-client\n"
                "status above. Decompile `CharacterListPacket::Deserialize` to\n"
                "convert these notes into definitive findings.\n\n"
                "1. **`current_level: u16` vs GAM `mainSkillLevel: signed char` (1 byte).**\n"
                "   Wire writes 2 bytes; GAM in-memory holds 1. If the deserializer\n"
                "   reads only 1 byte, the second byte would leak into whatever\n"
                "   follows. The fact that the live client doesn't disconnect at\n"
                "   chara-select suggests either (a) the wire schema actually\n"
                "   reads u16 here and clips to the i8 storage, or (b) the\n"
                "   deserializer reads u16 and the GAM schema diverges from the\n"
                "   wire shape for this field.\n\n"
                "2. **`tribe: u8` vs GAM `tribe: Sqex::Misc::Utf8String`.**\n"
                "   GAM declares the in-memory representation as a length-prefixed\n"
                "   UTF-8 string (e.g. \"Hyur Midlander\"). Garlemald writes 1\n"
                "   numeric byte. Either the wire transports the byte and the\n"
                "   client resolves to the display string at render time, or the\n"
                "   GAM Utf8String type is for a *different* code path (in-memory\n"
                "   cache, not wire) and the wire genuinely takes the raw id. The\n"
                "   raw `tribe_model` u32 a few writes earlier covers the model\n"
                "   id; this byte may be a redundant short tribe enum.\n\n"
                "3. **`location1: length-prefixed string` vs GAM `zoneName: signed char`.**\n"
                "   GAM in-memory holds a 1-byte zone id; garlemald writes\n"
                "   `\"prv0Inn01\\0\"` length-prefixed. The wire likely *does*\n"
                "   transport the string (resolved zone name for client-side\n"
                "   display) — this matches Project Meteor's RE — and the GAM\n"
                "   schema describes the post-resolve in-memory storage.\n\n"
                "4. **`location2: length-prefixed string` vs GAM `territoryName: signed char`.**\n"
                "   Same shape as #3. Same likely explanation.\n\n"
                "5. **`initial_town: u32 (twice)` vs GAM `initialTown: short` (one field).**\n"
                "   Two notes:\n"
                "   - Width: u32 wire vs i16 GAM. The deserializer may read u32\n"
                "     and downcast, or the GAM schema is the in-memory storage\n"
                "     after a width-cast at parse time.\n"
                "   - Two consecutive 4-byte writes mapped to the same GAM id:\n"
                "     this could be (a) two separate semantic fields the wire\n"
                "     format carries that GAM doesn't expose (favourite aetheryte,\n"
                "     fallback town, etc.), or (b) a Project Meteor port artifact\n"
                "     where the second slot is intentionally left equal to the\n"
                "     first because the original author didn't yet know what it\n"
                "     should hold. Decomp would tell us.\n"
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

        f.write("\n## Decomp follow-up\n\n")
        f.write("To convert the divergences above into definitive findings,\n")
        f.write("decompile `CharacterListPacket::Deserialize` and record the\n")
        f.write("byte-by-byte read sequence. The candidate is reachable from\n")
        f.write("the lobby Down dispatcher (see `config/{0}.opcodes.json`).\n".format(stem))
        f.write("Until then, **do not speculatively patch** `build_for_chara_list`:\n")
        f.write("the live client accepts the current shape, and changing the\n")
        f.write("wire layout based only on a schema-level divergence has a\n")
        f.write("real risk of breaking the working chara-select handoff.\n\n")
        f.write("Speculative patches kept in version-control history at\n")
        f.write("commits prior to 2026-05-01 if needed for reference.\n")

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
        "divergence": "DIVERGENCE — wire shape differs from GAM in-memory type (unconfirmed; needs decomp)",
    }.get(kind, "?")


if __name__ == "__main__":
    sys.exit(main())
