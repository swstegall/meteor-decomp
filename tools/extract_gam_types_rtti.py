#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Extract per-(ns, id) GAM types directly from RTTI template-parameter names.

The mangled `Component::GAM::CompileTimeParameter<id, &PARAMNAME, TYPE,
DecoratorSimpleAssign<TYPE>>` template instantiation embeds the property's
TYPE as its 3rd template argument. That's ground truth from the binary,
unlike the heuristic .data descriptor parsing in `extract_gam_params.py`
which has systematic off-by-one bugs on array sizes (e.g. reports
`int[3]` for `Array<int,4>`).

Empirically, comparing the two on the 192 ClientSelectData / Player /
PlayerPlayer / etc. CTPs, ~80 of the existing extractor's array sizes
disagree with RTTI by exactly one (it under-counts by 1) and a handful
of scalar types render differently (`unsigned __int64` vs
`unsigned___int64` — cosmetic). RTTI is the authoritative source.

Output:
  config/<binary>.gam_types_rtti.json   (ns, id, type) records
  config/<binary>.gam_params.json       enriched in-place with `rtti_type`
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CONFIG = REPO_ROOT / "config"
WIRE = REPO_ROOT / "build" / "wire"

CTP_PATTERN = re.compile(
    r"^Component::GAM::CompileTimeParameter<"
    r"(\d+),"                                           # 1: id
    r"&char\*_([A-Za-z_:][\w:]*)::PARAMNAME_\d+,"       # 2: ns (owning Data class)
    r"(.*?),"                                           # 3: type (lazy)
    r"class_Component::GAM::DecoratorSimpleAssign"
)


def normalize_type(rtti_type: str) -> str:
    """Render the RTTI-mangled type as a more conventional C++ string.

    RTTI demangles `unsigned __int64` as `unsigned___int64` and prefixes
    aggregates with `class_` / `struct_`. This unwinds those so the output
    looks like the original C++ source.
    """
    t = rtti_type
    t = t.replace("___", " __")             # unsigned___int64 → unsigned __int64
    t = re.sub(r"\b(class|struct)_", "", t) # strip kind prefix
    t = t.replace("_", " ")                 # signed_char → signed char
    # Render Component::GAM::Array<X,N> → X[N]
    m = re.match(r"^Component::GAM::Array<(.+?),(\d+)>(.*)$", t)
    if m:
        inner, count, tail = m.group(1), m.group(2), m.group(3)
        t = f"{inner.strip()}[{count}]{tail}"
    return t.strip()


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", default="ffxivgame", nargs="?")
    args = ap.parse_args()
    stem = args.binary.replace(".exe", "")

    handlers_path = WIRE / f"{stem}.net_handlers.json"
    if not handlers_path.exists():
        print(f"error: missing {handlers_path}; run extract_net_vtables.py first", file=sys.stderr)
        return 1
    handlers = json.loads(handlers_path.read_text())

    # De-dup CTP class names (every slot of every CTP repeats the class name).
    ctp_classes: set[str] = set()
    for e in handlers:
        cls = e.get("class", "")
        if cls.startswith("Component::GAM::CompileTimeParameter<") and "PARAMNAME" in cls:
            ctp_classes.add(cls)

    rows: list[dict] = []
    for cls in ctp_classes:
        m = CTP_PATTERN.match(cls)
        if not m:
            continue
        id_, ns, ty = int(m.group(1)), m.group(2), m.group(3)
        rows.append({
            "id": id_,
            "ns": ns,
            "rtti_type_raw": ty,
            "rtti_type": normalize_type(ty),
        })
    rows.sort(key=lambda r: (r["ns"], r["id"]))

    out_rtti = CONFIG / f"{stem}.gam_types_rtti.json"
    out_rtti.write_text(json.dumps(rows, indent=2) + "\n")
    print(f"wrote: {out_rtti.relative_to(REPO_ROOT)}  ({len(rows)} entries)")

    # Enrich existing gam_params.json with `rtti_type` field where RTTI
    # has a match. Surfaces disagreements per-row so downstream tools can
    # prefer rtti_type when present.
    gp_path = CONFIG / f"{stem}.gam_params.json"
    if gp_path.exists():
        gp = json.loads(gp_path.read_text())
        by_key = {(r["ns"], r["id"]): r for r in rows}
        agree = disagree = 0
        for entry in gp:
            key = (entry.get("ns"), entry.get("id"))
            r = by_key.get(key)
            if r:
                entry["rtti_type"] = r["rtti_type"]
                if entry.get("type", "").strip() == r["rtti_type"].strip():
                    agree += 1
                else:
                    disagree += 1
        gp_path.write_text(json.dumps(gp, indent=2) + "\n")
        print(f"  enriched {gp_path.name}: {agree} agree, {disagree} differ from existing extractor")

    return 0


if __name__ == "__main__":
    sys.exit(main())
