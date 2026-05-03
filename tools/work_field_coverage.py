#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Coverage report: garlemald's SetActorProperty path table vs. the
client-side work-table fields garlemald MUST populate.

Closes the spec triangle into a 4-way cross-reference:

  client engine API (cpp_bindings)
        ↓
  state-fields the API reads (work_field_inventory)
        ↓
  garlemald's property-path writers (THIS TOOL'S INPUT)
        ↓
  garlemald scripts that consume the values (garlemald_lua_coverage)

Methodology
-----------

1. Parse garlemald's Rust source for property-path string literals
   (both dot-separated `"playerWork.tribe"` and slash-separated
   `"playerWork/tribe"` forms — the slash form is murmur2-hashed
   to a 32-bit wire-ID per `docs/murmur2.md`).

2. Load `build/wire/work_field_inventory.json` produced by
   `extract_work_fields.py` (the per-table field inventory the
   client's scripts actually read).

3. For each inventory field `<table>.<field>`, check garlemald's
   path table for any path that starts with that prefix. A garlemald
   path like `charaWork.parameterSave.hp[0]` SATISFIES the
   inventory's `charaWork.parameterSave` requirement (since the
   parent path is being written somewhere).

4. Report per-table:
   - **Covered**: field has at least one garlemald writer
   - **Uncovered**: field is read by client scripts but garlemald
     has no writer

Output:
  build/wire/work_field_coverage.json
  build/wire/work_field_coverage.md
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from collections import defaultdict
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

# Known top-level work-table prefixes (from work_field_inventory.json)
WORK_TABLES = {
    "playerWork", "charaWork", "npcWork", "questDirectorWork",
    "directorWork", "desktopWidgetWork", "progDebugWork",
    "guildleveWork", "aetheryteWork", "widgetWork", "instanceRaidWork",
    "normalItemWork", "syncItemWork", "askWork", "areaWork",
    "battleSave", "battleTemp", "battleParameter", "battleStateForSelf",
    "battleCommon", "generalParameter", "questWork", "itemWork",
    "itemPackageWork",
}

# Match property-path string literals in Rust source.
# Dot form: "playerWork.tribe" / "charaWork.parameterSave.hp[0]" / "playerWork.questScenario[i]"
# Slash form: "playerWork/journal" / "charaWork/exp"
DOT_PATH_RE = re.compile(
    r'"((?:playerWork|charaWork|npcWork|guildleveWork|aetheryteWork'
    r'|directorWork|areaWork|widgetWork|instanceRaidWork|normalItemWork'
    r'|askWork|questWork|questDirectorWork|battleSave|battleTemp'
    r'|battleParameter|battleStateForSelf|battleCommon|generalParameter'
    r'|desktopWidgetWork|progDebugWork|syncItemWork|itemWork|itemPackageWork)'
    r'\.[A-Za-z_0-9.\[\]]+)"'
)
SLASH_PATH_RE = re.compile(
    r'"((?:playerWork|charaWork|npcWork|guildleveWork|aetheryteWork'
    r'|directorWork|areaWork|widgetWork|instanceRaidWork|normalItemWork'
    r'|askWork|questWork|questDirectorWork|battleSave|battleTemp'
    r'|battleParameter|battleStateForSelf|battleCommon|generalParameter'
    r'|desktopWidgetWork|progDebugWork|syncItemWork|itemWork|itemPackageWork)'
    r'/[A-Za-z_0-9/]+)"'
)


def find_garlemald_paths(garlemald_root: Path) -> dict[str, dict]:
    """Walk garlemald's Rust source for property-path literals.

    Returns: {path_string: {"style": "dot"|"slash", "files": [paths]}}
    """
    paths: dict[str, dict] = {}
    for src_dir in (garlemald_root / "common" / "src",
                    garlemald_root / "map-server" / "src",
                    garlemald_root / "world-server" / "src",
                    garlemald_root / "lobby-server" / "src"):
        if not src_dir.exists():
            continue
        for root, _, files in os.walk(src_dir):
            for f in files:
                if not f.endswith(".rs"):
                    continue
                p = Path(root) / f
                try:
                    text = p.read_text(errors="replace")
                except Exception:
                    continue
                rel = str(p.relative_to(garlemald_root))
                for m in DOT_PATH_RE.finditer(text):
                    path = m.group(1)
                    e = paths.setdefault(path, {"style": "dot", "files": set()})
                    e["files"].add(rel)
                for m in SLASH_PATH_RE.finditer(text):
                    path = m.group(1)
                    e = paths.setdefault(path, {"style": "slash", "files": set()})
                    e["files"].add(rel)
    # Convert sets to sorted lists for JSON serialization
    for v in paths.values():
        v["files"] = sorted(v["files"])
    return paths


def index_garlemald_paths_by_field(
    paths: dict[str, dict]
) -> dict[str, dict[str, list[str]]]:
    """Build {table → {top_field → [full paths covering it]}}.

    A garlemald path like "charaWork.parameterSave.hp[0]" indexes
    under (table=charaWork, top_field=parameterSave) — the top
    field is the second segment of the dotted/slashed path.
    """
    index: dict[str, dict[str, list[str]]] = defaultdict(lambda: defaultdict(list))
    for path, info in paths.items():
        sep = "/" if info["style"] == "slash" else "."
        # Split the head off
        parts = re.split(r"[./]", path, maxsplit=2)
        if len(parts) < 2:
            continue
        table, top_field = parts[0], parts[1]
        # Strip array-indexing suffix like [0] / [i] from top_field
        top_field = re.sub(r"\[.*?\]", "", top_field)
        index[table][top_field].append(path)
    return {t: dict(d) for t, d in index.items()}


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--garlemald-root", type=Path,
                    default=REPO_ROOT.parent / "garlemald-server")
    ap.add_argument("--inventory", type=Path,
                    default=REPO_ROOT / "build" / "wire" / "work_field_inventory.json")
    ap.add_argument("--out-dir", type=Path,
                    default=REPO_ROOT / "build" / "wire")
    args = ap.parse_args()

    if not args.garlemald_root.exists():
        print(f"error: --garlemald-root {args.garlemald_root} not found",
              file=sys.stderr)
        return 1
    if not args.inventory.exists():
        print(f"error: --inventory {args.inventory} not found — "
              f"run 'make extract-work-fields' first", file=sys.stderr)
        return 1
    args.out_dir.mkdir(parents=True, exist_ok=True)

    inventory = json.loads(args.inventory.read_text())
    inventory_fields = inventory["fields"]   # {table: {field: {access_count, ...}}}

    garlemald_paths = find_garlemald_paths(args.garlemald_root)
    garlemald_index = index_garlemald_paths_by_field(garlemald_paths)

    coverage: dict[str, dict] = {}
    total_inventory_fields = 0
    total_covered = 0

    # Walk every inventory table+field, check coverage
    for table in sorted(inventory_fields):
        fields = inventory_fields[table]
        per_field: dict[str, dict] = {}
        for field in sorted(fields, key=lambda f: -fields[f]["access_count"]):
            access_count = fields[field]["access_count"]
            covering_paths = garlemald_index.get(table, {}).get(field, [])
            per_field[field] = {
                "access_count": access_count,
                "covered": len(covering_paths) > 0,
                "garlemald_paths": covering_paths,
            }
            total_inventory_fields += 1
            if covering_paths:
                total_covered += 1
        n_fields = len(per_field)
        n_cov = sum(1 for v in per_field.values() if v["covered"])
        coverage[table] = {
            "field_count": n_fields,
            "covered_count": n_cov,
            "uncovered_count": n_fields - n_cov,
            "fields": per_field,
        }

    # Also collect garlemald paths that DON'T match any inventory field
    # (potential dead writers or undocumented client fields)
    inventory_keys = {(t, f) for t, fl in inventory_fields.items() for f in fl}
    orphan_paths: list[dict] = []
    for path, info in garlemald_paths.items():
        parts = re.split(r"[./]", path, maxsplit=2)
        if len(parts) < 2: continue
        table = parts[0]
        top = re.sub(r"\[.*?\]", "", parts[1])
        if (table, top) not in inventory_keys:
            orphan_paths.append({
                "path": path,
                "style": info["style"],
                "files": info["files"],
            })
    orphan_paths.sort(key=lambda x: x["path"])

    # Write JSON
    out_json = args.out_dir / "work_field_coverage.json"
    out_json.write_text(json.dumps({
        "summary": {
            "inventory_fields_total": total_inventory_fields,
            "inventory_fields_covered": total_covered,
            "inventory_fields_uncovered": total_inventory_fields - total_covered,
            "garlemald_total_paths": len(garlemald_paths),
            "garlemald_orphan_paths": len(orphan_paths),
        },
        "coverage": coverage,
        "orphan_garlemald_paths": orphan_paths,
    }, indent=2))

    # Write Markdown
    out_md = args.out_dir / "work_field_coverage.md"
    with out_md.open("w") as f:
        f.write("# Work-field coverage report — garlemald writers vs client readers\n\n")
        f.write("Auto-generated by `tools/work_field_coverage.py`. Closes the\n")
        f.write("spec triangle into a 4-way cross-reference (engine API → state\n")
        f.write("fields → garlemald writers → garlemald scripts).\n\n")

        f.write("## Summary\n\n")
        f.write(f"- **Inventory fields total** (from work_field_inventory.json): "
                f"{total_inventory_fields}\n")
        f.write(f"- **Inventory fields COVERED by a garlemald writer**: "
                f"**{total_covered}** ({100*total_covered/total_inventory_fields:.1f}%)\n")
        f.write(f"- **Inventory fields UNCOVERED**: "
                f"**{total_inventory_fields - total_covered}** "
                f"({100*(total_inventory_fields - total_covered)/total_inventory_fields:.1f}%)\n")
        f.write(f"- **Garlemald property paths total**: {len(garlemald_paths)}\n")
        f.write(f"- **Garlemald orphan paths** (write but no inventory entry): "
                f"{len(orphan_paths)}\n\n")

        f.write("## Per-table coverage\n\n")
        f.write("| Work table | Inventory fields | Covered | Uncovered |\n")
        f.write("|---|---:|---:|---:|\n")
        for table in sorted(coverage, key=lambda t: -coverage[t]["uncovered_count"]):
            c = coverage[table]
            uncov_marker = (f"**{c['uncovered_count']}**"
                            if c['uncovered_count'] else "0")
            f.write(f"| `{table}` | {c['field_count']} | {c['covered_count']} | "
                    f"{uncov_marker} |\n")
        f.write("\n")

        f.write("## UNCOVERED fields (priority — each needs a garlemald writer)\n\n")
        f.write("These fields are read by the client's scripts but garlemald has\n")
        f.write("no SetActorProperty writer for them. Each missing writer means\n")
        f.write("the client sees default/uninitialised values for the corresponding\n")
        f.write("UI / behaviour.\n\n")
        any_unc = False
        for table in sorted(coverage, key=lambda t: -coverage[t]["uncovered_count"]):
            c = coverage[table]
            unc = [(name, v) for name, v in c["fields"].items() if not v["covered"]]
            if not unc:
                continue
            any_unc = True
            f.write(f"### `{table}` — {len(unc)} uncovered\n\n")
            f.write("| Field | Access count |\n|---|---:|\n")
            unc.sort(key=lambda x: -x[1]["access_count"])
            for name, v in unc:
                f.write(f"| `{name}` | {v['access_count']} |\n")
            f.write("\n")
        if not any_unc:
            f.write("*(none — every inventory field has a garlemald writer)*\n\n")

        f.write("## COVERED fields (already populated by garlemald)\n\n")
        f.write("<details>\n<summary>Expand</summary>\n\n")
        for table in sorted(coverage):
            c = coverage[table]
            cov = [(name, v) for name, v in c["fields"].items() if v["covered"]]
            if not cov:
                continue
            f.write(f"### `{table}` — {len(cov)} covered\n\n")
            for name, v in cov:
                paths = v["garlemald_paths"]
                f.write(f"  - `{name}` → {', '.join(f'`{p}`' for p in paths)}\n")
            f.write("\n")
        f.write("</details>\n\n")

        if orphan_paths:
            f.write("## Orphan garlemald paths (writes with no inventory entry)\n\n")
            f.write("These garlemald paths target fields the inventory tool didn't\n")
            f.write("capture. Three explanations:\n")
            f.write("1. The field IS used by client scripts, but the inventory tool's\n")
            f.write("   regex missed it (inline-access pattern not yet supported).\n")
            f.write("2. The field is purely server-side bookkeeping (e.g., wire-only\n")
            f.write("   handshake fields).\n")
            f.write("3. The path targets a deeper sub-path that the inventory only\n")
            f.write("   captured at the parent level (e.g., `charaWork.parameterSave.hp`\n")
            f.write("   → inventory captured `parameterSave` but not `hp`).\n\n")
            f.write("<details>\n<summary>Expand</summary>\n\n")
            for o in orphan_paths:
                f.write(f"  - `{o['path']}` ({o['style']})\n")
            f.write("\n</details>\n\n")

    print(f"Wrote {out_json.relative_to(REPO_ROOT)}", file=sys.stderr)
    print(f"Wrote {out_md.relative_to(REPO_ROOT)}", file=sys.stderr)
    print(f"Summary: {total_inventory_fields} inventory fields, "
          f"{total_covered} covered ({100*total_covered/total_inventory_fields:.1f}%), "
          f"{total_inventory_fields - total_covered} uncovered, "
          f"{len(orphan_paths)} orphan garlemald paths")
    return 0


if __name__ == "__main__":
    sys.exit(main())
