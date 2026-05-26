#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Propose `Class::vfunc<slot>` names for still-unnamed vtable methods.

`config/<bin>.vtable_slots.jsonl` (from DumpRtti.java) maps every RTTI vtable
slot to the function it points at: {class, slot, fn_rva, fn_name}. This turns
that into a names file in the `name_overrides` schema so ApplyKnownNames.java
can push `Class::vfunc<slot>` onto functions that are still `FUN_xxx` — making
the disassembly navigable by owning class + slot even when we don't yet know
what the method does.

Scope + safety (conservative on purpose):
  * Only classes confidently joined to FFXIVLegacyClientStructs
    (config/<bin>.struct_layouts.json) — i.e. the game's own class vocabulary,
    not std::/Sqwt template noise. Pass --all-rtti to widen to every game
    class (those with a "::" and not starting with std).
  * Only functions still named FUN_/thunk_FUN_/SUB_/LAB_ in symbols.json —
    never overwrite a real name.
  * Only functions that appear in EXACTLY ONE class's vtable. A function shared
    across vtables is an inherited method whose owner is ambiguous, so we skip
    it rather than mis-attribute.

Reads:
  config/<bin>.vtable_slots.jsonl
  config/<bin>.struct_layouts.json   (the FFXIVLECS×RTTI join; unless --all-rtti)
  config/<bin>.symbols.json          (current names — skip already-named)

Writes:
  config/<bin>.vtable_method_names.json   name_overrides schema, source
                                          "vtable-slot"; feed to ApplyKnownNames
                                          via APPLY_NAMES_JSON (kept separate
                                          from name_overrides.json so it stays
                                          opt-in / re-derivable).
"""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CONFIG = REPO_ROOT / "config"
IMAGE_BASE = 0x400000
_UNNAMED = re.compile(r"^(FUN_|thunk_FUN_|SUB_|LAB_)")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("binary", nargs="?", default="ffxivgame")
    ap.add_argument("--all-rtti", action="store_true",
                    help="widen scope to every game class, not just FFXIVLECS-joined")
    args = ap.parse_args()
    b = args.binary

    slots_path = CONFIG / f"{b}.vtable_slots.jsonl"
    if not slots_path.exists():
        raise SystemExit(f"missing {slots_path.relative_to(REPO_ROOT)} "
                         "(run the Ghidra import / DumpRtti first)")
    slots = [json.loads(line) for line in slots_path.read_text().splitlines() if line.strip()]

    # current names, to skip already-named functions
    name_by_rva = {e["rva"]: e["name"] for e in
                   json.loads((CONFIG / f"{b}.symbols.json").read_text())}

    # scope: which classes are eligible
    if args.all_rtti:
        eligible = None  # any game class (filtered below)
    else:
        sl = json.loads((CONFIG / f"{b}.struct_layouts.json").read_text())
        eligible = {c["name"] for c in sl["classes"]}

    def in_scope(cls: str) -> bool:
        if eligible is not None:
            return cls in eligible
        return "::" in cls and not cls.startswith("std")

    # count how many distinct classes each function is a vtable member of
    fn_classes: dict[int, set] = {}
    for s in slots:
        fn_classes.setdefault(s["fn_rva"], set()).add(s["class"])

    out, ambiguous, already, out_of_scope = [], 0, 0, 0
    seen = set()
    for s in slots:
        cls, slot, rva = s["class"], s["slot"], s["fn_rva"]
        if not in_scope(cls):
            out_of_scope += 1
            continue
        cur = name_by_rva.get(rva, s.get("fn_name", ""))
        if not _UNNAMED.match(cur or ""):
            already += 1
            continue
        if len(fn_classes.get(rva, ())) != 1:
            ambiguous += 1
            continue
        if rva in seen:
            continue
        seen.add(rva)
        out.append({
            "rva": rva,
            "rva_hex": f"0x{rva:08x}",
            "name": f"{cls}::vfunc{slot}",
            "source": "vtable-slot",
            "current_symbol": cur,
        })

    out.sort(key=lambda r: r["rva"])
    dest = CONFIG / f"{b}.vtable_method_names.json"
    dest.write_text(json.dumps(out, indent=2) + "\n")
    print(f"vtable-method-names [{b}]:")
    print(f"  slots: {len(slots)} | scope: "
          f"{'all-rtti' if args.all_rtti else 'FFXIVLECS-joined'}")
    print(f"  proposed names: {len(out)}  "
          f"(skipped: {already} already-named, {ambiguous} ambiguous/inherited, "
          f"{out_of_scope} out-of-scope)")
    print(f"  wrote {dest.relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
