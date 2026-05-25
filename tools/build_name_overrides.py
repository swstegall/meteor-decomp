#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Merge the recovered-name sidecars into one authoritative name-override
layer that the work pool and contributors consult.

Names recovered from the Yokimitsuro repos live in two sidecars:
  config/<binary>.legacy_symbols.json     (FFXIVLegacyClientStructs ctor names)
  config/<binary>.ffxivdecomp_symbols.json (ffxivDecomp opcode/binding/receiver names)

Both key a real RVA that the binary still carries as FUN_xxxxxxxx in
config/<binary>.symbols.json. This tool folds them into:

  config/<binary>.name_overrides.json   [{rva, rva_hex, name, source,
                                          current_symbol}]

It is intentionally **non-destructive** — it does NOT rewrite
symbols.json (which is a Ghidra dump; rewriting it would force a
work-pool / asm re-split and disturb in-flight matches). Instead this
override layer is a documented work-pool input (see AGENTS.md): when a
contributor is matching a FUN_xxxxxxxx that appears here, use this name.
Only RVAs that still resolve to FUN_/thunk_/LAB_/SUB_ in symbols.json
are emitted (already-named functions are skipped, conflicts reported).

Usage: python3 tools/build_name_overrides.py [--binary ffxivgame]
"""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
AUTO = re.compile(r"(FUN_|thunk_FUN_|LAB_|SUB_)")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--binary", default="ffxivgame")
    args = ap.parse_args()
    cfg = REPO_ROOT / "config"

    syms = {s["rva"]: s["name"] for s in json.loads((cfg / f"{args.binary}.symbols.json").read_text())}

    sidecars = [
        (f"{args.binary}.legacy_symbols.json", "FFXIVLegacyClientStructs"),
        (f"{args.binary}.ffxivdecomp_symbols.json", "ffxivDecomp"),
    ]
    rows: dict[int, dict] = {}
    conflicts, skipped_named = [], 0
    for fname, src in sidecars:
        p = cfg / fname
        if not p.exists():
            continue
        for e in json.loads(p.read_text()):
            rva = e["rva"]
            name = e["name"]
            cur = syms.get(rva)
            if cur is None:
                continue  # no function there (already cross-checked upstream)
            if not AUTO.match(cur):
                skipped_named += 1  # already has a human name; don't override
                continue
            if rva in rows and rows[rva]["name"] != name:
                conflicts.append((rva, rows[rva]["name"], name, src))
                continue
            rows[rva] = {
                "rva": rva, "rva_hex": f"0x{rva:08x}", "name": name,
                "source": src, "current_symbol": cur,
            }

    out = cfg / f"{args.binary}.name_overrides.json"
    merged = sorted(rows.values(), key=lambda r: r["rva"])
    out.write_text(json.dumps(merged, indent=1), encoding="utf-8")

    print(f"wrote config/{out.name}: {len(merged)} name overrides")
    print(f"  (skipped {skipped_named} already-human-named; {len(conflicts)} conflicts)")
    for rva, a, b, src in conflicts:
        print(f"  CONFLICT 0x{rva:08x}: {a!r} vs {b!r} (from {src})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
