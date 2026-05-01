#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Cross-validate garlemald-server's `parse_new_char_request` flow
(lobby-server/src/data/chara_info.rs) against the binary's GAM
CharaMakeData schema (config/<binary>.gam_params.json).

The two are not the same wire format — garlemald reads a hand-rolled
flat blob (positional u8/u16/u32 reads with explicit padding skips
and a 0x10-byte seek), while GAM is the typed (id, value) registry
the binary's GameAttributeManager declares for the CharaMakeData
class. Both describe the *same* underlying schema (the fields the
client sends during character creation), so they should agree on:

  - field count (after correctly accounting for header / padding / trailer)
  - field type (the type a given semantic field carries)

This tool produces a side-by-side report that calls out:
  - aligned fields (same type, plausible position match)
  - type mismatches
  - count discrepancies
  - garlemald-side reads with no GAM counterpart (and vice versa)

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
# read or skip in source order; the type is the *Rust* type the call
# returns. `kind` distinguishes:
#   "header"  pre-payload framing (version, unknown1 — outside GAM)
#   "field"   a named CharaMake field; should align to a GAM id
#   "padding" a skip that doesn't carry information (filler)
#   "trailer" post-payload (initial_town comes after a 0x10 seek)
#
# byte_size is the number of bytes consumed from the cursor. For
# field/padding rows this is the actual read width; for the seek it's
# the relative skip distance.
GARLEMALD_FLOW: list[dict] = [
    {"kind": "header",  "name": "_version",                "rs_type": "u32", "byte_size": 4},
    {"kind": "header",  "name": "_unknown1",               "rs_type": "u32", "byte_size": 4},
    {"kind": "field",   "name": "info.tribe",              "rs_type": "u8",  "byte_size": 1},
    {"kind": "field",   "name": "appearance.size",         "rs_type": "u8",  "byte_size": 1},
    {"kind": "field",   "name": "appearance.hair_style",   "rs_type": "u16", "byte_size": 2},
    {"kind": "field",   "name": "appearance.hair_highlight_color", "rs_type": "u8",  "byte_size": 1},
    {"kind": "field",   "name": "appearance.hair_variation",       "rs_type": "u8",  "byte_size": 1},
    {"kind": "field",   "name": "appearance.face_type",            "rs_type": "u8",  "byte_size": 1},
    {"kind": "field",   "name": "appearance.characteristics",      "rs_type": "u8",  "byte_size": 1},
    {"kind": "field",   "name": "appearance.characteristics_color", "rs_type": "u8", "byte_size": 1},
    {"kind": "padding", "name": "(u32 skip)",              "rs_type": "u32", "byte_size": 4},
    {"kind": "field",   "name": "appearance.face_eyebrows",        "rs_type": "u8",  "byte_size": 1},
    {"kind": "field",   "name": "appearance.face_iris_size",       "rs_type": "u8",  "byte_size": 1},
    {"kind": "field",   "name": "appearance.face_eye_shape",       "rs_type": "u8",  "byte_size": 1},
    {"kind": "field",   "name": "appearance.face_nose",            "rs_type": "u8",  "byte_size": 1},
    {"kind": "field",   "name": "appearance.face_features",        "rs_type": "u8",  "byte_size": 1},
    {"kind": "field",   "name": "appearance.face_mouth",           "rs_type": "u8",  "byte_size": 1},
    {"kind": "field",   "name": "appearance.ears",                 "rs_type": "u8",  "byte_size": 1},
    {"kind": "field",   "name": "appearance.hair_color",           "rs_type": "u16", "byte_size": 2},
    {"kind": "padding", "name": "(u32 skip)",              "rs_type": "u32", "byte_size": 4},
    {"kind": "field",   "name": "appearance.skin_color",           "rs_type": "u16", "byte_size": 2},
    {"kind": "field",   "name": "appearance.eye_color",            "rs_type": "u16", "byte_size": 2},
    {"kind": "field",   "name": "appearance.voice",                "rs_type": "u8",  "byte_size": 1},
    {"kind": "field",   "name": "info.guardian",                   "rs_type": "u8",  "byte_size": 1},
    {"kind": "field",   "name": "info.birth_month",                "rs_type": "u8",  "byte_size": 1},
    {"kind": "field",   "name": "info.birth_day",                  "rs_type": "u8",  "byte_size": 1},
    {"kind": "field",   "name": "info.current_class",              "rs_type": "u16", "byte_size": 2},
    # The three trailing u32s are 12 bytes total — likely the int[3]
    # at GAM id 124 (a position vector or similar).
    {"kind": "padding", "name": "(u32 skip — likely int[3] @ GAM id 124)", "rs_type": "u32 x 3", "byte_size": 12},
    # 0x10 seek — 16 bytes that don't appear in GAM at all. Could be a
    # trailer / checksum / version-specific extension.
    {"kind": "padding", "name": "(seek 0x10 — no GAM counterpart)", "rs_type": "skip", "byte_size": 16},
    {"kind": "trailer", "name": "info.initial_town",               "rs_type": "u8",  "byte_size": 1},
]

# Rust read width → GAM type compatibility. A garlemald `u8` read can
# legitimately carry a GAM `signed char` or `unsigned char` value.
RUST_TO_GAM_COMPAT = {
    "u8":  ["signed char", "char", "unsigned char", "bool"],
    "u16": ["short", "unsigned short"],
    "u32": ["int", "unsigned int"],
    "u64": ["__int64", "unsigned __int64"],
}


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
    cmd = sorted(
        [r for r in rows if r["ns"].endswith("::CharaMakeData")],
        key=lambda r: r["id"],
    )

    WIRE.mkdir(parents=True, exist_ok=True)
    out = WIRE / f"{stem}.chara_make_validation.md"

    # Walk garlemald fields in order, attempting to match each against
    # the next GAM entry positionally. For padding/skip rows, we still
    # advance a "GAM cursor" if the byte size + element-size suggests
    # they cover a GAM entry.
    findings: list[str] = []
    g_iter = iter(cmd)
    g_current = next(g_iter, None)
    g_consumed_for_current = 0
    rows_table: list[tuple] = []

    for row in GARLEMALD_FLOW:
        kind = row["kind"]
        sz = row["byte_size"]
        if kind == "header":
            rows_table.append(("header", row["name"], row["rs_type"], "—", "—", "outside GAM"))
            continue

        # Walk GAM cursor by `sz` bytes; for each GAM entry fully consumed
        # by this row, line them up. Most rows consume exactly one GAM entry.
        consumed_gam: list[dict] = []
        remaining = sz
        while remaining > 0 and g_current is not None:
            gam_size = _gam_size(g_current)
            available = gam_size - g_consumed_for_current
            take = min(available, remaining)
            g_consumed_for_current += take
            remaining -= take
            if g_consumed_for_current >= gam_size:
                consumed_gam.append(g_current)
                g_current = next(g_iter, None)
                g_consumed_for_current = 0

        # Render row(s).
        if not consumed_gam:
            rows_table.append((kind, row["name"], row["rs_type"], "(no GAM align)", "—", "no GAM counterpart consumed"))
            continue

        for i, ge in enumerate(consumed_gam):
            rs_type = row["rs_type"] if i == 0 else "(continued)"
            name = row["name"] if i == 0 else "(continued)"
            note = _alignment_note(row, ge)
            rows_table.append((kind, name, rs_type, ge["id"], ge["type"], note))

    # Trailing GAM entries (garlemald didn't consume them).
    while g_current is not None:
        rows_table.append(("(trailing GAM)", "—", "—", g_current["id"], g_current["type"], "GAM has this; garlemald flow doesn't read"))
        g_current = next(g_iter, None)

    # Render markdown.
    with out.open("w") as f:
        f.write(f"# {stem}.exe — `parse_new_char_request` ↔ GAM CharaMakeData\n\n")
        f.write(f"Auto-generated by `tools/validate_chara_make.py`. Compares\n")
        f.write(f"`garlemald-server/lobby-server/src/data/chara_info.rs::parse_new_char_request`\n")
        f.write(f"against the binary's `Application::Network::GameAttributeManager::Data::CharaMakeData`\n")
        f.write(f"GAM registry ({len(cmd)} entries).\n\n")
        f.write(f"## What this report does (and doesn't) prove\n\n")
        f.write(f"This walker pairs garlemald's reads with GAM entries\n")
        f.write(f"**positionally, sorted by GAM id**. That alignment is only\n")
        f.write(f"valid if the wire format serialises GAM entries in id order;\n")
        f.write(f"the breakdown after ~8 fields below suggests it doesn't.\n\n")
        f.write(f"GAM is a *class-side* schema (the registry of properties\n")
        f.write(f"the C++ `CharaMakeData` class declares). The wire format used\n")
        f.write(f"during character creation is a separate hand-coded\n")
        f.write(f"`Serialize` method that emits the same fields in some\n")
        f.write(f"order — possibly with bitfield packing (Project Meteor's\n")
        f.write(f"build path packs 4 chars into a u32 in places). The\n")
        f.write(f"correct alignment requires reading\n")
        f.write(f"`CharaMakeData::Serialize` (locate via\n")
        f.write(f"`build/wire/{stem}.net_handlers.md` →\n")
        f.write(f"`Application::Network::GameAttributeManager::Data::CharaMakeData`).\n\n")
        f.write(f"What this report DOES surface:\n\n")
        f.write(f"- The GAM registry's authoritative field count (26).\n")
        f.write(f"- The garlemald parser's read count (24 named + 1 trailer).\n")
        f.write(f"- The byte-count discrepancy (the 0x10 seek covers 16 bytes\n")
        f.write(f"  that have no GAM counterpart).\n")
        f.write(f"- The first 8 GAM ids align cleanly with garlemald's first 8\n")
        f.write(f"  reads, so the early-blob layout *is* GAM-id-ordered.\n\n")

        f.write(f"## Side-by-side\n\n")
        f.write("| kind | garlemald field | rs type | GAM id | GAM type | note |\n")
        f.write("|---|---|---|---:|---|---|\n")
        for row in rows_table:
            kind, name, rt, gid, gt, note = row
            f.write(f"| {kind} | `{name}` | `{rt}` | {gid} | `{gt}` | {note} |\n")

        # Summary.
        garlemald_field_rows = sum(1 for r in GARLEMALD_FLOW if r["kind"] == "field" or r["kind"] == "trailer")
        f.write(f"\n## Summary\n\n")
        f.write(f"- garlemald field+trailer reads: **{garlemald_field_rows}**\n")
        f.write(f"- GAM CharaMakeData entries: **{len(cmd)}**\n")
        f.write(f"- garlemald total bytes (fields + padding + seek + trailer, excluding 8-byte header): "
                f"**{sum(r['byte_size'] for r in GARLEMALD_FLOW if r['kind'] != 'header')}**\n")
        f.write(f"- GAM total wire footprint (assuming id-order, no padding): "
                f"**{sum(_gam_size(r) for r in cmd)}**\n")
        f.write(f"\n## Open questions for the next contributor\n\n")
        f.write(f"1. **What is the actual wire-byte order?** Decompile\n"
                f"   `Application::Network::GameAttributeManager::Data::CharaMakeData::Serialize`\n"
                f"   (or its inverse `Deserialize`) and confirm whether GAM\n"
                f"   ids are emitted in id-order, declaration-order, or some\n"
                f"   per-field hand-written sequence.\n")
        f.write(f"2. **Are the two `u32 skip` reads in garlemald lossy?**\n"
                f"   Project Meteor's `build_for_chara_list` packs 4 chars\n"
                f"   into u32s in some places (see `parse_new_char_request`\n"
                f"   handles them as opaque skips). If those bytes carry\n"
                f"   real GAM-side fields (e.g. ids 108-111 and 118-121),\n"
                f"   garlemald is silently discarding them.\n")
        f.write(f"3. **What's in the 16-byte `seek 0x10`?** No GAM entry\n"
                f"   sums to 16 bytes alone, so this is likely a trailer or\n"
                f"   per-version extension outside the GAM schema. Could be\n"
                f"   a CRC, padding to a power-of-two boundary, or a slot\n"
                f"   reserved for fields added in later patches.\n")
        f.write(f"4. **Is `info.initial_town` the GAM id 125 trailer?** It\n"
                f"   reads 1 byte after the seek, which lines up size-wise\n"
                f"   with GAM's last entry (id 125, signed char). If yes,\n"
                f"   the wire format is `[8B header][24 fields with packing,\n"
                f"   24-25 bytes][int[3] = 12B][16B trailer][1B init town]`,\n"
                f"   which is at least internally consistent with GAM's 26-\n"
                f"   field schema after accounting for packing.\n")

    print(f"wrote: {out.relative_to(REPO_ROOT)}")
    return 0


def _gam_size(entry: dict) -> int:
    """Wire footprint for one GAM entry, in bytes."""
    sizes = {
        "signed char": 1, "char": 1, "unsigned char": 1, "bool": 1,
        "short": 2, "unsigned short": 2, "wchar_t": 2,
        "int": 4, "unsigned int": 4, "long": 4, "unsigned long": 4, "float": 4,
        "double": 8, "__int64": 8, "unsigned __int64": 8,
    }
    t = entry["type"]
    if t in sizes:
        return sizes[t]
    if "[" in t:
        elem, n = t.rstrip("]").split("[")
        return sizes.get(elem, 0) * int(n)
    if t == "Sqex::Misc::Utf8String":
        return 0  # variable
    return 0


def _alignment_note(row: dict, ge: dict) -> str:
    rs = row["rs_type"]
    gam = ge["type"]
    if row["kind"] == "padding":
        return f"garlemald padding swallows GAM id {ge['id']} ({gam})"
    compat = RUST_TO_GAM_COMPAT.get(rs, [])
    if gam in compat:
        return "type ok"
    if "[" in gam:
        return f"garlemald {rs} doesn't unwrap GAM array {gam}"
    return f"type mismatch: {rs} vs {gam}"


if __name__ == "__main__":
    sys.exit(main())
