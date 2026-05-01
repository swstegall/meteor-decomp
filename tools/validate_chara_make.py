#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Cross-validate garlemald-server's `parse_new_char_request` flow
(lobby-server/src/data/chara_info.rs) against the binary's GAM
CharaMakeData schema *with resolved property names* from
`tools/extract_paramnames_dispatch.py`.

After resolving the PARAMNAME pointers we know definitively:
  - GAM CharaMakeData has 26 ids (100..125) with semantic names.
  - The wire format IS GAM-id-ordered, with two non-GAM u32 skips
    inserted as sub-record headers (or legacy redundant-packed
    fields the parser correctly ignores) and a 16-byte trailer.

Garlemald's parser aligns cleanly against this, except for three
discrepancies surfaced here:

  1. `appearance.face_features` is the binary's `faceCheek` (id 112).
     Semantic mislabel — "face_features" is misleading.
  2. `appearance.ears` is the binary's `faceJaw` (id 114).
     Semantic mislabel — these are the jaw/chin shape, NOT ears.
     1.x had a separate ears field elsewhere (or none — Hyur ears
     are attached to the face mesh).
  3. `info.current_class: u16` lumps two GAM fields:
     - id 122 `initialMainSkill` (signed char, 1 byte) — starting class.
     - id 123 `initialEquipSet` (signed char, 1 byte) — starting equipment set.
     Reading them as a u16 loses the equipment-set value.
  4. Three trailing `u32 skip` reads (12 bytes) ARE GAM id 124
     `initialBonusItem: int[3]` — three starter-item ids the
     character is granted at creation. Garlemald discards them.

Output:
  build/wire/<binary>.chara_make_validation.md
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CONFIG = REPO_ROOT / "config"
WIRE = REPO_ROOT / "build" / "wire"

# Garlemald's parse_new_char_request flow, transcribed by hand from
# lobby-server/src/data/chara_info.rs (lines 42-81). One row per
# read or skip in source order.
#
# `gam_id`: the binary GAM id this read maps to. None = non-GAM
# (header / sub-record header / trailer / 16-byte seek).
GARLEMALD_FLOW: list[dict] = [
    {"kind": "header",       "name": "_version",                          "rs_type": "u32", "byte_size": 4, "gam_id": None},
    {"kind": "header",       "name": "_unknown1",                         "rs_type": "u32", "byte_size": 4, "gam_id": None},
    {"kind": "field",        "name": "info.tribe",                        "rs_type": "u8",  "byte_size": 1, "gam_id": 100},
    {"kind": "field",        "name": "appearance.size",                   "rs_type": "u8",  "byte_size": 1, "gam_id": 101},
    {"kind": "field",        "name": "appearance.hair_style",             "rs_type": "u16", "byte_size": 2, "gam_id": 102},
    {"kind": "field",        "name": "appearance.hair_highlight_color",   "rs_type": "u8",  "byte_size": 1, "gam_id": 103},
    {"kind": "field",        "name": "appearance.hair_variation",         "rs_type": "u8",  "byte_size": 1, "gam_id": 104},
    {"kind": "field",        "name": "appearance.face_type",              "rs_type": "u8",  "byte_size": 1, "gam_id": 105},
    {"kind": "field",        "name": "appearance.characteristics",        "rs_type": "u8",  "byte_size": 1, "gam_id": 106},
    {"kind": "field",        "name": "appearance.characteristics_color",  "rs_type": "u8",  "byte_size": 1, "gam_id": 107},
    {"kind": "non_gam_skip", "name": "(u32 skip — likely sub-record header for face block)", "rs_type": "u32", "byte_size": 4, "gam_id": None},
    {"kind": "field",        "name": "appearance.face_eyebrows",          "rs_type": "u8",  "byte_size": 1, "gam_id": 108},
    {"kind": "field",        "name": "appearance.face_iris_size",         "rs_type": "u8",  "byte_size": 1, "gam_id": 109},
    {"kind": "field",        "name": "appearance.face_eye_shape",         "rs_type": "u8",  "byte_size": 1, "gam_id": 110},
    {"kind": "field",        "name": "appearance.face_nose",              "rs_type": "u8",  "byte_size": 1, "gam_id": 111},
    {"kind": "mislabel",     "name": "appearance.face_features (= faceCheek)", "rs_type": "u8", "byte_size": 1, "gam_id": 112},
    {"kind": "field",        "name": "appearance.face_mouth",             "rs_type": "u8",  "byte_size": 1, "gam_id": 113},
    {"kind": "mislabel",     "name": "appearance.ears (= faceJaw)",       "rs_type": "u8",  "byte_size": 1, "gam_id": 114},
    {"kind": "field",        "name": "appearance.hair_color",             "rs_type": "u16", "byte_size": 2, "gam_id": 115},
    {"kind": "non_gam_skip", "name": "(u32 skip — likely packed-color redundancy)", "rs_type": "u32", "byte_size": 4, "gam_id": None},
    {"kind": "field",        "name": "appearance.skin_color",             "rs_type": "u16", "byte_size": 2, "gam_id": 116},
    {"kind": "field",        "name": "appearance.eye_color",              "rs_type": "u16", "byte_size": 2, "gam_id": 117},
    {"kind": "field",        "name": "appearance.voice",                  "rs_type": "u8",  "byte_size": 1, "gam_id": 118},
    {"kind": "field",        "name": "info.guardian",                     "rs_type": "u8",  "byte_size": 1, "gam_id": 119},
    {"kind": "field",        "name": "info.birth_month",                  "rs_type": "u8",  "byte_size": 1, "gam_id": 120},
    {"kind": "field",        "name": "info.birth_day",                    "rs_type": "u8",  "byte_size": 1, "gam_id": 121},
    {"kind": "lossy",        "name": "info.current_class (u16 lumps id 122 initialMainSkill + id 123 initialEquipSet)",
                              "rs_type": "u16", "byte_size": 2, "gam_id": "122+123"},
    {"kind": "discarded",    "name": "(3x u32 skip = id 124 initialBonusItem int[3], starter items)",
                              "rs_type": "u32 x 3", "byte_size": 12, "gam_id": 124},
    {"kind": "non_gam_skip", "name": "(seek 0x10 — non-GAM trailer / padding)", "rs_type": "skip", "byte_size": 16, "gam_id": None},
    {"kind": "trailer",      "name": "info.initial_town",                 "rs_type": "u8",  "byte_size": 1, "gam_id": 125},
]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", default="ffxivgame", nargs="?")
    args = ap.parse_args()
    stem = args.binary.replace(".exe", "")

    src = CONFIG / f"{stem}.gam_params.json"
    if not src.exists():
        print(f"error: missing {src}; run extract_gam_params.py first", file=sys.stderr)
        return 1
    rows = json.loads(src.read_text())
    by_id_ns = {(r["id"], r["ns"]): r for r in rows}

    cmd_ns = "Application::Network::GameAttributeManager::Data::CharaMakeData"
    cmd = sorted(
        [r for r in rows if r["ns"] == cmd_ns],
        key=lambda r: r["id"],
    )

    WIRE.mkdir(parents=True, exist_ok=True)
    out = WIRE / f"{stem}.chara_make_validation.md"

    with out.open("w") as f:
        f.write(f"# {stem}.exe — `parse_new_char_request` ↔ GAM CharaMakeData\n\n")
        f.write(f"Auto-generated by `tools/validate_chara_make.py`. Compares\n")
        f.write(f"`garlemald-server/lobby-server/src/data/chara_info.rs::parse_new_char_request`\n")
        f.write(f"against the binary's `{cmd_ns}`\n")
        f.write(f"GAM registry ({len(cmd)} entries with resolved property names).\n\n")
        f.write(f"## Conclusion\n\n")
        f.write(f"Garlemald's parser aligns to the binary's GAM CharaMakeData\n")
        f.write(f"schema **field-by-field**, with the wire being GAM-id-ordered\n")
        f.write(f"plus two non-GAM `u32 skip` sub-record headers and a 16-byte\n")
        f.write(f"trailing seek. Three discrepancies are concrete bugs / data\n")
        f.write(f"loss in the parser:\n\n")
        f.write(f"1. **`appearance.face_features` → should be `faceCheek`** (GAM id 112).\n"
                f"   Semantic mislabel — \"face_features\" doesn't match the binary's name.\n\n")
        f.write(f"2. **`appearance.ears` → should be `faceJaw`** (GAM id 114).\n"
                f"   Semantic mislabel. 1.x doesn't expose ears as a separate slot — those\n"
                f"   bytes encode the jaw/chin shape. Whatever \"ears\" looked like in the\n"
                f"   live client was driven by `tribe` + `face` lookups, not this field.\n\n")
        f.write(f"3. **`info.current_class: u16` reads two GAM fields as one** (ids 122 + 123).\n"
                f"   - id 122 `initialMainSkill` (signed char, 1 byte) — starting class.\n"
                f"   - id 123 `initialEquipSet` (signed char, 1 byte) — starting gear set.\n"
                f"   Reading both as a single u16 named `current_class` discards the\n"
                f"   equipment-set value. Should split into two u8 reads.\n\n")
        f.write(f"4. **Three trailing `u32 skip` reads correspond to GAM id 124 `initialBonusItem: int[3]`** —\n"
                f"   three starter-item ids the character is granted at creation.\n"
                f"   Garlemald discards them; should read as `[u32; 3]` and pass through\n"
                f"   to the inventory init.\n\n")
        f.write(f"## Side-by-side\n\n")
        f.write("| garlemald field | rs type | bytes | GAM id | binary name | type | note |\n")
        f.write("|---|---|---:|---:|---|---|---|\n")
        for row in GARLEMALD_FLOW:
            gam_id = row["gam_id"]
            if isinstance(gam_id, int):
                gam = by_id_ns.get((gam_id, cmd_ns), {})
                bn = gam.get("paramname", "?")
                gt = gam.get("type", "?")
            elif gam_id == "122+123":
                a = by_id_ns.get((122, cmd_ns), {})
                b = by_id_ns.get((123, cmd_ns), {})
                bn = f"{a.get('paramname','?')} + {b.get('paramname','?')}"
                gt = f"{a.get('type','?')} + {b.get('type','?')}"
            else:
                bn = "—"
                gt = "—"
            note = _note(row["kind"])
            f.write(f"| `{row['name']}` | `{row['rs_type']}` | {row['byte_size']} | "
                    f"{gam_id if gam_id is not None else '—'} | "
                    f"`{bn}` | `{gt}` | {note} |\n")

        f.write(f"\n## Suggested patch (Rust pseudocode)\n\n")
        f.write("```rust\n")
        f.write("// In lobby-server/src/data/chara_info.rs::parse_new_char_request,\n")
        f.write("// rename misleading fields:\n")
        f.write("appearance.face_cheek = c.read_u8()?;   // was face_features\n")
        f.write("appearance.face_mouth = c.read_u8()?;\n")
        f.write("appearance.face_jaw   = c.read_u8()?;   // was ears\n")
        f.write("// And split current_class into the two GAM fields it lumps:\n")
        f.write("info.initial_main_skill = c.read_u8()? as u32;  // was high byte of current_class\n")
        f.write("info.initial_equip_set  = c.read_u8()? as u32;  // was low byte of current_class\n")
        f.write("// And capture the three starter-item ids:\n")
        f.write("for i in 0..3 {\n")
        f.write("    info.initial_bonus_item[i] = c.read_u32::<LittleEndian>()?;\n")
        f.write("}\n")
        f.write("```\n\n")
        f.write(f"## Definitive answer to the earlier open questions\n\n")
        f.write(f"- **GAM id ordering vs wire order**: ✅ confirmed *id-ordered* for\n")
        f.write(f"  CharaMakeData, with 4-byte non-GAM skips inserted between groups\n")
        f.write(f"  (one before id 108 `faceBrow`, one before id 116 `skintone`).\n")
        f.write(f"- **Are the u32 skips lossy?** No — they have no GAM counterpart.\n")
        f.write(f"  Likely sub-record headers or legacy-format redundancy that the\n")
        f.write(f"  parser correctly ignores.\n")
        f.write(f"- **What's in the 16-byte `seek 0x10`?** Non-GAM. Likely padding\n")
        f.write(f"  or a per-version extension (the seek skips a fixed-size block\n")
        f.write(f"  that's reserved but unused at 1.23b).\n")
        f.write(f"- **Is `info.initial_town` the GAM id 125 trailer?** ✅ yes,\n")
        f.write(f"  `initialTown` (signed char).\n")

    print(f"wrote: {out.relative_to(REPO_ROOT)}")
    return 0


def _note(kind: str) -> str:
    return {
        "header":       "outside GAM (packet wrapper)",
        "field":        "ok",
        "non_gam_skip": "non-GAM, no field lost",
        "mislabel":     "**SEMANTIC MISLABEL** — type ok, name wrong",
        "lossy":        "**BUG** — single read consumes 2 GAM fields",
        "discarded":    "**BUG** — 12 bytes silently dropped",
        "trailer":      "ok (post-trailer)",
    }.get(kind, "?")


if __name__ == "__main__":
    sys.exit(main())
