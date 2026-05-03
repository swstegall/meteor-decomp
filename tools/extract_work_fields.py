#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Enumerate the per-class state-field inventory used by the engine's
client-side scripts.

Per `docs/player_base_decomp.md`, every C++-bound method on
PlayerBaseClass / CharaBaseClass / etc. implicitly references some
work-table field (`playerWork.guildleveId`, `charaWork.parameterSave`,
etc.). This tool walks the decompiled corpus and produces the
canonical inventory: per work-table, what fields are accessed.

For garlemald, this is the **state-field requirement spec** — every
field listed here must be populated correctly via SetActorProperty
/ work-sync packets for the client's scripts to behave.

Approach
--------
unluac decompiles `self.playerWork.guildleveId` as TWO consecutive
lines:

    L1_2 = A0_2.playerWork              -- load work table into local
    L1_2 = L1_2.guildleveId             -- field access on the local

So we track local-variable bindings: any `L? = X.<known_workTable>`
binds the local to that work-table; subsequent `L?2 = L?.<field>`
reads on that local capture the field name.

Method-vs-field filter
----------------------
Some matches are actually METHOD-pointer loads (the `L? = L?.X` is
followed by a call `L?(...)`). We detect this by peeking at the next
line for a call pattern. Filtered methods are reported separately
under "ambiguous_methods" so the user can audit.

Output:
  build/wire/work_field_inventory.json
  build/wire/work_field_inventory.md
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from collections import defaultdict, Counter
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

# Confirmed work-table identifiers (data tables, not methods). The
# threshold for inclusion: appears with field-access patterns in
# multiple classes / scripts.
KNOWN_WORK_TABLES = {
    "playerWork", "charaWork", "npcWork", "questDirectorWork",
    "directorWork", "desktopWidgetWork", "progDebugWork",
    "guildleveWork", "aetheryteWork", "widgetWork", "instanceRaidWork",
    "normalItemWork", "syncItemWork", "askWork", "areaWork",
    "battleSave", "battleTemp", "battleParameter", "battleStateForSelf",
    "battleCommon", "generalParameter", "questWork", "itemWork",
    "itemPackageWork",
}

FIELD_RE = re.compile(r"^\s*(L\d+_\d+)\s*=\s*(L\d+_\d+)\.(\w+)\s*$")
LOAD_RE = re.compile(r"^\s*(L\d+_\d+)\s*=\s*(\S+)\.(\w+)\s*$")
CALL_RE = re.compile(r"^\s*(?:L\d+_\d+\s*=\s*)?(L\d+_\d+)\s*\(")


def looks_like_method_name(name: str) -> bool:
    """Heuristic: identifiers starting with `_` are usually methods,
    not fields. This isn't bullet-proof — some engine fields legitimately
    start with `_` (e.g., `_temp`, `_sync`, `_tag` are work-table
    sub-fields per `docs/director_quest_decomp.md`) — so we only flag
    names that look like verb-prefixed methods.
    """
    if not name.startswith("_"):
        return False
    # Engine work-table sub-fields like _temp, _sync, _tag, _onInit are
    # legitimate FIELDS (or method names that double as field markers).
    # Don't treat short identifiers as suspicious.
    if len(name) <= 5:
        return False
    # Common verb prefixes after `_` suggest methods
    verb_prefixes = (
        "_get", "_set", "_is", "_has", "_can", "_do", "_call", "_break",
        "_cancel", "_reset", "_init", "_load", "_unload", "_save",
        "_run", "_wait", "_find", "_count", "_print", "_send",
        "_fade", "_lock", "_unlock", "_force", "_create",
        "_delete", "_clear", "_append", "_execute", "_turn",
        "_lookAt", "_cancel", "_aim", "_transform", "_setup",
    )
    for p in verb_prefixes:
        if name.startswith(p):
            return True
    return False


def scan_file(path: Path) -> tuple[dict, dict, set]:
    """Returns (fields_per_table, ambiguous_methods, observed_tables).

    fields_per_table: {table_name: Counter(field → count)}
    ambiguous_methods: {table_name: Counter(method-like-name → count)}
    observed_tables: set of work-table names actually seen in this file
    """
    fields_per_table = defaultdict(Counter)
    ambiguous = defaultdict(Counter)
    observed_tables = set()

    try:
        lines = path.read_text(errors="replace").splitlines()
    except Exception:
        return dict(fields_per_table), dict(ambiguous), observed_tables

    local_to_table: dict[str, str] = {}

    for i, line in enumerate(lines):
        # Check field-access first (most specific)
        m = FIELD_RE.match(line)
        if m:
            dest, src, ident = m.group(1), m.group(2), m.group(3)
            if src in local_to_table:
                table = local_to_table[src]
                # Peek next line for call pattern
                next_line = lines[i + 1] if i + 1 < len(lines) else ""
                next_call = CALL_RE.match(next_line)
                is_method = (
                    looks_like_method_name(ident)
                    or (next_call is not None and next_call.group(1) == dest)
                )
                if is_method:
                    ambiguous[table][ident] += 1
                else:
                    fields_per_table[table][ident] += 1
            # Reassignment: dest no longer holds the table
            local_to_table.pop(dest, None)
            continue

        # Then check load
        m = LOAD_RE.match(line)
        if m:
            lvar, source, ident = m.group(1), m.group(2), m.group(3)
            if source.startswith("L") and source in local_to_table:
                # This is actually a field-access we already missed
                continue
            if ident in KNOWN_WORK_TABLES:
                local_to_table[lvar] = ident
                observed_tables.add(ident)
            else:
                local_to_table.pop(lvar, None)

    return dict(fields_per_table), dict(ambiguous), observed_tables


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--lua-dir", type=Path, default=REPO_ROOT / "build" / "lua",
                    help="directory of decompiled .lua files (default: build/lua)")
    ap.add_argument("--out-dir", type=Path, default=REPO_ROOT / "build" / "wire",
                    help="output dir (default: build/wire)")
    args = ap.parse_args()

    if not args.lua_dir.exists():
        print(f"error: {args.lua_dir} not found — run 'make decompile-lpb' first",
              file=sys.stderr)
        return 1
    args.out_dir.mkdir(parents=True, exist_ok=True)

    # Aggregate
    global_fields: dict[str, Counter] = defaultdict(Counter)
    global_ambiguous: dict[str, Counter] = defaultdict(Counter)
    files_per_field: dict[str, dict[str, set]] = defaultdict(lambda: defaultdict(set))
    n_files = 0

    for root, _, files in os.walk(args.lua_dir):
        for f in files:
            if not f.endswith(".lua"):
                continue
            n_files += 1
            path = Path(root) / f
            rel = str(path.relative_to(args.lua_dir))
            fpt, amb, _ = scan_file(path)
            for table, fields in fpt.items():
                for field, count in fields.items():
                    global_fields[table][field] += count
                    files_per_field[table][field].add(rel)
            for table, methods in amb.items():
                for method, count in methods.items():
                    global_ambiguous[table][method] += count

    # JSON output
    out_json = args.out_dir / "work_field_inventory.json"
    out_data = {
        "summary": {
            "files_scanned": n_files,
            "work_tables": len(global_fields),
            "total_distinct_fields": sum(len(v) for v in global_fields.values()),
            "total_field_accesses": sum(sum(v.values()) for v in global_fields.values()),
        },
        "fields": {
            table: {
                field: {
                    "access_count": global_fields[table][field],
                    "files": sorted(files_per_field[table][field])[:5],
                    "file_count": len(files_per_field[table][field]),
                }
                for field in sorted(global_fields[table],
                                    key=lambda f: -global_fields[table][f])
            }
            for table in sorted(global_fields)
        },
        "ambiguous_methods_called_on_work_tables": {
            table: dict(global_ambiguous[table].most_common())
            for table in sorted(global_ambiguous)
        },
    }
    out_json.write_text(json.dumps(out_data, indent=2))

    # Markdown
    out_md = args.out_dir / "work_field_inventory.md"
    with out_md.open("w") as f:
        f.write("# Work-table state-field inventory\n\n")
        f.write("Auto-generated by `tools/extract_work_fields.py`. Re-run after\n")
        f.write("`make decompile-lpb` if the install changes.\n\n")

        f.write("## Summary\n\n")
        f.write(f"- **Files scanned**: {n_files}\n")
        f.write(f"- **Work tables observed**: {len(global_fields)}\n")
        total_fields = sum(len(v) for v in global_fields.values())
        total_accesses = sum(sum(v.values()) for v in global_fields.values())
        f.write(f"- **Total distinct fields**: {total_fields}\n")
        f.write(f"- **Total field accesses**: {total_accesses}\n\n")

        f.write("## Per-work-table totals\n\n")
        f.write("| Work table | Distinct fields | Total accesses |\n")
        f.write("|---|---:|---:|\n")
        for table in sorted(global_fields, key=lambda t: -sum(global_fields[t].values())):
            n_fields = len(global_fields[table])
            n_acc = sum(global_fields[table].values())
            f.write(f"| `{table}` | {n_fields} | {n_acc} |\n")
        f.write("\n")

        f.write("## Per-table field inventories\n\n")
        for table in sorted(global_fields, key=lambda t: -sum(global_fields[t].values())):
            n_fields = len(global_fields[table])
            n_acc = sum(global_fields[table].values())
            f.write(f"### `{table}` ({n_fields} fields, {n_acc} accesses)\n\n")
            f.write("| Field | Accesses | Files |\n|---|---:|---:|\n")
            for field, count in sorted(global_fields[table].items(),
                                       key=lambda x: -x[1]):
                fc = len(files_per_field[table][field])
                f.write(f"| `{field}` | {count} | {fc} |\n")
            f.write("\n")

        if global_ambiguous:
            f.write("## Ambiguous: method-like names accessed on work tables\n\n")
            f.write("These names were accessed on a work-table local but look like\n")
            f.write("METHODS (verb prefixes like `_get`, `_set`, `_load`, etc., or\n")
            f.write("followed by a call site). They're listed separately so you can\n")
            f.write("audit — some are genuine fields with method-like names, others\n")
            f.write("are tool false-positives.\n\n")
            f.write("<details>\n<summary>Expand</summary>\n\n")
            for table in sorted(global_ambiguous,
                                key=lambda t: -sum(global_ambiguous[t].values())):
                meths = global_ambiguous[table]
                if not meths: continue
                f.write(f"### `{table}` — {len(meths)} ambiguous\n\n")
                for name, count in meths.most_common():
                    f.write(f"  - `{name}` ({count})\n")
                f.write("\n")
            f.write("</details>\n")

    print(f"Wrote {out_json.relative_to(REPO_ROOT)}", file=sys.stderr)
    print(f"Wrote {out_md.relative_to(REPO_ROOT)}", file=sys.stderr)
    print(f"Summary: {len(global_fields)} work tables, "
          f"{total_fields} distinct fields, "
          f"{total_accesses} accesses across {n_files} files")
    return 0


if __name__ == "__main__":
    sys.exit(main())
