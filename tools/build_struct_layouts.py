#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Join FFXIVLegacyClientStructs class layouts onto our binary's RTTI vtables.

Background
----------
`config/ffxivgame.legacy_structs.json` is the imported FFXIVLegacyClientStructs
(FFXIVLECS) struct database — class names, sizes, constructor VAs and field
layouts. Its *addresses* are from a DIFFERENT client build, so its `ctor_va`
values do NOT line up with our 1.23b `ffxivgame.exe` (only ~7/2572 coincide).

But the struct *layouts* are valid for our binary — verified independently:
FFXIVLECS `CharaActor.PositionX @ 0x154` matches the offset we confirmed by
hand in `include/actor/chara_actor.h`. So the usable join is by **RTTI vtable
name**, not by address.

This tool normalises FFXIVLECS's C#-style namespace
(`FFXIVClientStructs.FFXIV.Application.Scene.Actor.Chara`) into the demangled
C++ form our `config/ffxivgame.rtti.json` records
(`Application::Scene::Actor::Chara::CharaActor`) and joins on it.

Reads
-----
  config/<binary>.legacy_structs.json   (FFXIVLECS import)
  config/<binary>.rtti.json             (DumpRtti.java — vtable class index)

Writes
------
  config/<binary>.struct_layouts.json   one row per confidently-joined class:
                                        {name, vtable_rvas, slot_count, size,
                                         fields:[{offset,name,type}]}
  include/structs/<binary>/<Class>.h    generated struct header for every class
                                        with >= MIN_FIELDS named fields (the
                                        field-rich gameplay structs)
  docs/struct_layouts.md                human-readable join summary

The JSON is the durable artifact a future Ghidra session can import to apply
these layouts wholesale (auto-annotating raw `*(T*)(this+off)` accesses).
"""

from __future__ import annotations

import argparse
import datetime as _dt
import json
import re
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CONFIG = REPO_ROOT / "config"
INCLUDE = REPO_ROOT / "include" / "structs"
DOCS = REPO_ROOT / "docs"

# Classes with at least this many named fields get a generated header — below
# this a class contributes only its vtable pointer, so a header adds no value.
MIN_FIELDS = 2


def normalise_full_name(c: dict) -> str:
    """FFXIVLECS class -> demangled C++ name in our rtti.json's format."""
    cpp = c.get("cpp_name_comment", "") or ""
    # When the import already resolved a clean C++ name, trust it.
    if "::" in cpp and not cpp.startswith("FFXIVClientStructs"):
        return cpp
    ns = c.get("namespace", "") or ""
    ns = re.sub(r"^FFXIVClientStructs\.FFXIV\.", "", ns)
    ns = re.sub(r"^FFXIVClientStructs\.", "", ns)
    ns = ns.replace(".", "::")
    return f"{ns}::{c['name']}" if ns else c["name"]


def sanitise(name: str) -> str:
    """Full C++ name -> filesystem-safe header stem."""
    return re.sub(r"[^A-Za-z0-9]+", "_", name).strip("_")


def load(path: Path):
    if not path.exists():
        raise SystemExit(f"missing input: {path.relative_to(REPO_ROOT)}")
    return json.loads(path.read_text())


def build(binary: str) -> dict:
    classes = load(CONFIG / f"{binary}.legacy_structs.json")
    rtti = load(CONFIG / f"{binary}.rtti.json")

    # rtti class name -> list of vtable entries (a class can own several
    # sub-object vtables under multiple inheritance, hence a list).
    rtti_by_name: dict[str, list] = {}
    for e in rtti:
        cn = e.get("class")
        if cn:
            rtti_by_name.setdefault(cn, []).append(e)

    joined = []
    ambiguous = 0
    for c in classes:
        full = normalise_full_name(c)
        entries = rtti_by_name.get(full)
        if not entries:
            continue
        if len(entries) > 1:
            ambiguous += 1
        fields = [
            {
                "offset": (f.get("offset") if isinstance(f.get("offset"), str)
                           else hex(f["offset"]) if isinstance(f.get("offset"), int)
                           else None),
                "name": f.get("name"),
                "type": f.get("type", ""),
            }
            for f in c.get("fields", [])
            if f.get("name")
        ]
        joined.append({
            "name": full,
            "vtable_rvas": [e.get("rva_hex") or hex(e["rva"]) for e in entries],
            "slot_count": max((e.get("slot_count", 0) for e in entries), default=0),
            "size": c.get("size"),
            "n_fields": len(fields),
            "fields": fields,
        })

    joined.sort(key=lambda r: (-r["n_fields"], r["name"]))
    return {
        "generated": _dt.date.today().isoformat(),
        "binary": binary,
        "source": (
            "config/{b}.legacy_structs.json (FFXIVLegacyClientStructs, MIT) "
            "joined to config/{b}.rtti.json by demangled vtable name".format(b=binary)
        ),
        "note": (
            "FFXIVLECS addresses are from a different client build and do NOT "
            "align with this binary; join is by RTTI vtable name. Field layouts "
            "validated independently (CharaActor.PositionX @ 0x154)."
        ),
        "stats": {
            "lecs_classes": len(classes),
            "joined": len(joined),
            "ambiguous_multi_vtable": ambiguous,
            "field_rich": sum(1 for r in joined if r["n_fields"] >= MIN_FIELDS),
            "total_named_fields": sum(r["n_fields"] for r in joined),
        },
        "classes": joined,
    }


HEADER_TMPL = """\
// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// GENERATED by tools/build_struct_layouts.py — do not edit by hand.
// Field layout sourced from FFXIVLegacyClientStructs (MIT), joined to this
// binary by RTTI vtable name. Addresses there are from a different build, so
// only the *layout* (offsets/names/types) is authoritative here. Fields are
// offset-sorted with explicit padding so member offsets are exact (x86, 4-byte
// pointers); undocumented gaps are `_padNN` filler. Fields the source places at
// an already-occupied offset are emitted as comments to keep the layout valid.
//
// Class:   {name}
// Size:    {size} (0x{size_hex})
// vtable:  {vtables}
#pragma once
#include <cstdint>

// {name}
struct {stem} {{
{members}
}};
"""

# C# type name -> (C++ type, size in bytes) for an x86 (32-bit pointer) target.
_CSHARP_TYPES = {
    "nint": ("uint32_t", 4), "nuint": ("uint32_t", 4),
    "bool": ("uint8_t", 1), "byte": ("uint8_t", 1), "sbyte": ("int8_t", 1),
    "char": ("uint16_t", 2), "short": ("int16_t", 2), "ushort": ("uint16_t", 2),
    "int": ("int32_t", 4), "uint": ("uint32_t", 4), "float": ("float", 4),
    "long": ("int64_t", 8), "ulong": ("uint64_t", 8), "double": ("double", 8),
}


def _cpp_type(t: str) -> tuple[str, int, str]:
    """(cpp_type, total_size_bytes, suffix) for a FFXIVLECS field type string.

    Unknown / non-primitive types are treated as an x86 pointer (4 bytes) since
    on this binary they're almost always object pointers.
    """
    t = (t or "").strip()
    m = re.match(r"^([A-Za-z_][\w:<>]*)\s*\[\s*(\d+)\s*\]$", t)
    if m:  # array T[N]
        base, n = m.group(1), int(m.group(2))
        cpp, sz = _CSHARP_TYPES.get(base, ("uint32_t", 4))
        return cpp, sz * n, f"[{n}]"
    cpp, sz = _CSHARP_TYPES.get(t, ("uint32_t", 4))
    return cpp, sz, ""


def emit_headers(data: dict, binary: str) -> list[str]:
    out_dir = INCLUDE / binary
    written = []
    for row in data["classes"]:
        if row["n_fields"] < MIN_FIELDS:
            continue
        stem = sanitise(row["name"])
        # offset-sort; drop fields with no parseable offset
        flds = []
        for f in row["fields"]:
            try:
                off = int(f["offset"], 16) if isinstance(f["offset"], str) else int(f["offset"])
            except (TypeError, ValueError):
                continue
            flds.append((off, f))
        flds.sort(key=lambda x: x[0])

        lines, cur, pad_n = [], 0, 0
        for off, f in flds:
            cpp, sz, suffix = _cpp_type(f["type"])
            if off < cur:  # overlaps an already-placed field — annotate, don't break layout
                lines.append(
                    f"    // @0x{off:x} {f['type'] or '?'} {f['name']}  "
                    f"(overlaps prior field; not emitted)"
                )
                continue
            if off > cur:  # undocumented gap -> filler
                lines.append(f"    uint8_t _pad{pad_n}[0x{off - cur:x}];")
                pad_n += 1
            lines.append(
                f"    /* 0x{off:<5x} */ {cpp.ljust(10)} {f['name']}{suffix};"
            )
            cur = off + sz
        size = row["size"] or 0
        if size and size > cur:  # tail filler to declared size
            lines.append(f"    uint8_t _pad{pad_n}[0x{size - cur:x}];")

        hdr = HEADER_TMPL.format(
            name=row["name"], stem=stem, size=size, size_hex=f"{size:x}",
            vtables=", ".join(row["vtable_rvas"]), members="\n".join(lines),
        )
        out_dir.mkdir(parents=True, exist_ok=True)
        path = out_dir / f"{stem}.h"
        path.write_text(hdr)
        written.append(str(path.relative_to(REPO_ROOT)))
    return written


def emit_doc(data: dict, headers: list[str], binary: str) -> str:
    s = data["stats"]
    rich = [r for r in data["classes"] if r["n_fields"] >= MIN_FIELDS]
    lines = [
        f"# Struct layouts — {binary} (FFXIVLegacyClientStructs × RTTI)",
        "",
        f"> Generated {data['generated']} by `tools/build_struct_layouts.py`. "
        "Re-run via `make struct-layouts`.",
        "",
        data["note"],
        "",
        "## Summary",
        "",
        f"- FFXIVLECS classes imported: **{s['lecs_classes']}**",
        f"- Confidently joined to a binary RTTI vtable (by demangled name): "
        f"**{s['joined']}**",
        f"  - of which appear in >1 vtable (multi-inheritance / dupes): "
        f"{s['ambiguous_multi_vtable']}",
        f"- Field-rich classes (≥ {MIN_FIELDS} named fields, header generated): "
        f"**{s['field_rich']}**",
        f"- Total named field definitions: **{s['total_named_fields']}**",
        "",
        "The join gives every one of those classes a concrete vtable RVA in "
        "this binary, so their virtual methods can be named by class; the "
        "field-rich classes additionally get usable struct layouts.",
        "",
        "## Field-rich classes (struct headers under "
        f"`include/structs/{binary}/`)",
        "",
        "| Class | Fields | Size | vtable RVA(s) |",
        "|---|---:|---:|---|",
    ]
    for r in rich:
        lines.append(
            f"| `{r['name']}` | {r['n_fields']} | "
            f"{r['size']} | {', '.join(r['vtable_rvas'])} |"
        )
    lines += [
        "",
        f"Generated {len(headers)} header(s). The full join (all "
        f"{s['joined']} classes, with vtable RVAs + sizes) is in "
        f"`config/{binary}.struct_layouts.json` — the durable artifact a future "
        "Ghidra session can import to apply these layouts wholesale.",
        "",
    ]
    return "\n".join(lines)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("binary", nargs="?", default="ffxivgame")
    args = ap.parse_args()
    binary = args.binary

    data = build(binary)
    (CONFIG / f"{binary}.struct_layouts.json").write_text(
        json.dumps(data, indent=2) + "\n"
    )
    headers = emit_headers(data, binary)
    (DOCS / "struct_layouts.md").write_text(emit_doc(data, headers, binary))

    s = data["stats"]
    print(f"struct-layouts [{binary}]:")
    print(f"  joined {s['joined']}/{s['lecs_classes']} classes "
          f"({s['ambiguous_multi_vtable']} multi-vtable)")
    print(f"  field-rich (>= {MIN_FIELDS}): {s['field_rich']} "
          f"-> {len(headers)} headers; {s['total_named_fields']} total fields")
    print(f"  wrote config/{binary}.struct_layouts.json, docs/struct_layouts.md")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
