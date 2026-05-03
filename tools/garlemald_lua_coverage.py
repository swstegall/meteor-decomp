#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Coverage report: garlemald's Lua-binding API surface vs. the methods
its own server-side scripts actually call.

This is the actionable companion to `docs/cpp_bindings_index.md`. The
client-side C++ binding inventory tells us what the engine exposes;
this tool tells us what garlemald's scripts NEED its userdata.rs to
expose so they don't crash with `attempt to call a nil value`.

What it produces

  build/wire/garlemald_lua_coverage.json
  build/wire/garlemald_lua_coverage.md

For each garlemald UserData type (LuaPlayer, LuaActor, LuaNpc, ...)
we report:

  - Methods bound in `userdata.rs`     — defined via add_method()
  - Methods called from scripts         — observed in scripts/lua/
  - Coverage gaps                       — called but not bound (likely
                                          runtime errors)
  - Dead bindings                       — bound but never called (cleanup
                                          candidates)

Type inference is convention-based via variable-name → UserData mapping
(e.g. `player:` → LuaPlayer, `quest:` → LuaQuestHandle, ...). The
mapping table is in CONVENTIONS below; extend it as garlemald grows
new variable conventions.

Usage:
  tools/garlemald_lua_coverage.py
  tools/garlemald_lua_coverage.py --garlemald-root <path>
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

# Variable-name → UserData type. Garlemald's scripts use these
# conventions consistently; if a script uses a non-conventional name
# (e.g. `myPlayer:GetName()`), this tool will undercount its calls.
#
# Important: a few "stub" types here actually delegate to a base type
# at runtime (LuaPlayer ⊂ LuaActor, LuaNpc ⊂ LuaActor). The tool
# reports cross-type bindings under "bound elsewhere" rather than as
# a true gap.
CONVENTIONS = {
    "player":       "LuaPlayer",
    "p":            "LuaPlayer",
    "npc":          "LuaNpc",
    "actor":        "LuaActor",
    "doorNpc":      "LuaNpc",
    "papalymo":     "LuaNpc",
    "yda":          "LuaNpc",
    "exitTriggerNpc": "LuaNpc",
    "quest":        "LuaQuestHandle",
    "data":         "LuaQuestDataHandle",
    "director":     "LuaDirectorHandle",
    "contentArea":  "LuaContentArea",
    "CurrentArea":  "LuaZone",   # CurrentArea field is a LuaZone, not LuaContentArea
    "zone":         "LuaZone",
    "party":        "LuaParty",
    "item":         "LuaItemData",
    "retainer":     "LuaRetainer",
    "package":      "LuaItemPackage",
    "recipe":       "LuaRecipe",
    "node":         "LuaGatherNode",
    "leve":         "LuaRegionalLeve",
}

# Methods bound on a "lower" type that get inherited by composition
# at runtime. Example: LuaPlayer holds a LuaActor base, and method
# calls like `player:PlayAnimation()` reach LuaActor's binding via
# garlemald's UserData metamethod forwarding.
#
# Calls of methods bound on a different UserData type than expected
# are categorized as "bound elsewhere" rather than true gaps.

# Calls of the form `GetWorldManager():method()` always go to LuaWorldManager.
WORLD_MANAGER_RE = re.compile(r"GetWorldManager\(\)\s*:\s*([A-Za-z_][A-Za-z0-9_]+)")


def find_userdata_bindings(rs_path: Path) -> dict[str, set[str]]:
    """Parse a Rust source file looking for `impl UserData for <T> {` blocks
    and per-block `add_method("name", ...)` declarations.
    """
    bindings: dict[str, set[str]] = defaultdict(set)
    if not rs_path.exists():
        return dict(bindings)
    text = rs_path.read_text()
    # Walk through `impl UserData for <T>` blocks. Track brace depth.
    impl_re = re.compile(r"impl\s+UserData\s+for\s+(\w+)\s*\{")
    method_re = re.compile(r'add_(?:async_)?method\(\s*"([^"]+)"')
    pos = 0
    while True:
        m = impl_re.search(text, pos)
        if not m:
            break
        cls = m.group(1)
        # Find matching closing brace by depth-tracking
        i = m.end()  # just past the `{`
        depth = 1
        while i < len(text) and depth > 0:
            c = text[i]
            if c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    break
            i += 1
        block = text[m.end():i]
        for mm in method_re.finditer(block):
            bindings[cls].add(mm.group(1))
        pos = i
    return dict(bindings)


def find_script_calls(scripts_root: Path) -> tuple[dict[str, set[str]], set[str]]:
    """Walk garlemald's Lua scripts collecting `<var>:<method>` calls.

    Returns:
      calls_per_type — {UserData_type: set of methods called}
      unmapped_vars — set of variable names that didn't match CONVENTIONS
                       (helpful for extending the table)
    """
    calls_per_type: dict[str, set[str]] = defaultdict(set)
    unmapped_vars: set[str] = set()
    # Match patterns like `player:GetName(`, `quest:GetData(`
    call_re = re.compile(r"(\w+)\s*:\s*([A-Za-z_][A-Za-z0-9_]+)\s*\(")
    for root, _, files in os.walk(scripts_root):
        for f in files:
            if not f.endswith(".lua"):
                continue
            path = Path(root) / f
            try:
                text = path.read_text(errors="replace")
            except Exception:
                continue
            # Strip Lua line + block comments to avoid false positives
            text = re.sub(r"--\[\[.*?\]\]", "", text, flags=re.DOTALL)
            text = re.sub(r"--[^\n]*", "", text)
            for m in call_re.finditer(text):
                var = m.group(1)
                method = m.group(2)
                t = CONVENTIONS.get(var)
                if t is not None:
                    calls_per_type[t].add(method)
                else:
                    unmapped_vars.add(var)
            # WorldManager pattern
            for m in WORLD_MANAGER_RE.finditer(text):
                calls_per_type["LuaWorldManager"].add(m.group(1))
    return dict(calls_per_type), unmapped_vars


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--garlemald-root", type=Path,
                    default=REPO_ROOT.parent / "garlemald-server",
                    help="garlemald-server repo root (default: ../garlemald-server)")
    ap.add_argument("--out-dir", type=Path,
                    default=REPO_ROOT / "build" / "wire",
                    help="output dir (default: build/wire)")
    args = ap.parse_args()

    if not args.garlemald_root.exists():
        print(f"error: --garlemald-root {args.garlemald_root} not found",
              file=sys.stderr)
        return 1
    args.out_dir.mkdir(parents=True, exist_ok=True)

    rs_path = args.garlemald_root / "map-server" / "src" / "lua" / "userdata.rs"
    scripts_path = args.garlemald_root / "scripts" / "lua"
    bindings = find_userdata_bindings(rs_path)
    calls, unmapped = find_script_calls(scripts_path)

    # Build a global "method → which classes bind it" map for the
    # cross-type check.
    method_to_bound_classes: dict[str, set[str]] = defaultdict(set)
    for t, methods in bindings.items():
        for m in methods:
            method_to_bound_classes[m].add(t)

    # Per-UserData report
    all_types = sorted(set(bindings) | set(calls))
    coverage: dict[str, dict] = {}
    total_called = 0
    total_bound = 0
    total_true_gap = 0
    total_cross_type = 0
    total_dead = 0
    for t in all_types:
        bound = bindings.get(t, set())
        called = calls.get(t, set())
        ok = sorted(bound & called)             # bound + called on this type
        called_minus_bound = called - bound     # called but not bound on this type
        # Split: bound on a different type (cross-type) vs not bound anywhere
        true_gap = sorted(m for m in called_minus_bound
                          if not method_to_bound_classes.get(m))
        cross_type = sorted(
            (m, sorted(method_to_bound_classes[m]))
            for m in called_minus_bound
            if method_to_bound_classes.get(m)
        )
        dead = sorted(bound - called)           # bound but never called
        coverage[t] = {
            "bound_count": len(bound),
            "called_count": len(called),
            "ok_count": len(ok),
            "true_gap_count": len(true_gap),
            "cross_type_count": len(cross_type),
            "dead_count": len(dead),
            "true_gap_methods": true_gap,
            "cross_type_methods": [
                {"method": m, "bound_in": classes} for m, classes in cross_type
            ],
            "dead_methods": dead,
        }
        total_bound += len(bound)
        total_called += len(called)
        total_true_gap += len(true_gap)
        total_cross_type += len(cross_type)
        total_dead += len(dead)

    # JSON
    out_json = args.out_dir / "garlemald_lua_coverage.json"
    out_json.write_text(json.dumps({
        "summary": {
            "userdata_types": len(all_types),
            "total_bindings": total_bound,
            "total_distinct_calls": total_called,
            "total_true_gaps": total_true_gap,
            "total_cross_type_calls": total_cross_type,
            "total_dead_bindings": total_dead,
            "unmapped_variable_count": len(unmapped),
        },
        "coverage": coverage,
        "unmapped_variables": sorted(unmapped),
    }, indent=2))

    # Markdown
    out_md = args.out_dir / "garlemald_lua_coverage.md"
    with out_md.open("w") as f:
        f.write("# Garlemald Lua-binding coverage report\n\n")
        f.write("Auto-generated by `tools/garlemald_lua_coverage.py`.\n\n")
        f.write("Cross-references methods bound in garlemald's\n")
        f.write("`map-server/src/lua/userdata.rs` against methods CALLED in\n")
        f.write("garlemald's `scripts/lua/` tree. Type inference uses the\n")
        f.write("variable-name → UserData mapping documented in the tool.\n\n")
        f.write("## Summary\n\n")
        f.write(f"- **UserData types observed**: {len(all_types)}\n")
        f.write(f"- **Total bindings (add_method)**: {total_bound}\n")
        f.write(f"- **Total distinct method calls**: {total_called}\n")
        f.write(f"- **TRUE gaps** (called but bound nowhere — high-priority): "
                f"**{total_true_gap}**\n")
        f.write(f"- **Cross-type calls** (called on type T but bound on type T' — "
                f"reaches via composition): {total_cross_type}\n")
        f.write(f"- **Dead bindings** (bound but never called — cleanup candidates): "
                f"{total_dead}\n")
        f.write(f"- **Unmapped variable names** (not in CONVENTIONS): {len(unmapped)}\n\n")

        f.write("## Per-UserData type coverage\n\n")
        f.write("| UserData | Bound | Called | OK | **True gaps** | Cross-type | Dead |\n")
        f.write("|---|---:|---:|---:|---:|---:|---:|\n")
        for t in all_types:
            c = coverage[t]
            gap_marker = f"**{c['true_gap_count']}**" if c['true_gap_count'] else "0"
            f.write(f"| `{t}` | {c['bound_count']} | {c['called_count']} | "
                    f"{c['ok_count']} | {gap_marker} | {c['cross_type_count']} | "
                    f"{c['dead_count']} |\n")
        f.write("\n")

        f.write("## TRUE gaps — methods called but bound NOWHERE\n\n")
        f.write("These methods don't appear in any `add_method(...)` call\n")
        f.write("anywhere in `userdata.rs`. Each is a guaranteed\n")
        f.write("`attempt to call a nil value` at runtime — top-priority\n")
        f.write("binding additions.\n\n")
        any_gap = False
        for t in all_types:
            c = coverage[t]
            if not c["true_gap_methods"]:
                continue
            any_gap = True
            f.write(f"### `{t}` — {c['true_gap_count']} missing\n\n")
            for m in c["true_gap_methods"]:
                f.write(f"  - `{m}`\n")
            f.write("\n")
        if not any_gap:
            f.write("*(none — every called method is bound somewhere)*\n\n")

        f.write("## Cross-type calls — bound on a different UserData type\n\n")
        f.write("These methods ARE bound somewhere, but on a different type than\n")
        f.write("the call's apparent variable type. Most likely intentional\n")
        f.write("(garlemald's UserData composition forwards method calls between\n")
        f.write("related types — e.g. LuaActor methods reachable from LuaPlayer).\n")
        f.write("Worth verifying the runtime forwarding is actually wired.\n\n")
        f.write("<details>\n<summary>Expand</summary>\n\n")
        for t in all_types:
            c = coverage[t]
            if not c["cross_type_methods"]:
                continue
            f.write(f"### `{t}` — {c['cross_type_count']} cross-type\n\n")
            for entry in c["cross_type_methods"]:
                f.write(f"  - `{entry['method']}` → bound in: "
                        f"{', '.join(f'`{x}`' for x in entry['bound_in'])}\n")
            f.write("\n")
        f.write("</details>\n\n")

        f.write("## Dead bindings — bound but never called\n\n")
        f.write("Methods defined in `userdata.rs` that no script touches. Could be\n")
        f.write("intentional (forward-looking API surface) or stale. Review at\n")
        f.write("leisure.\n\n")
        f.write("<details>\n<summary>Expand</summary>\n\n")
        for t in all_types:
            c = coverage[t]
            if not c["dead_methods"]:
                continue
            f.write(f"### `{t}` — {c['dead_count']} unused\n\n")
            for m in c["dead_methods"]:
                f.write(f"  - `{m}`\n")
            f.write("\n")
        f.write("</details>\n\n")

        if unmapped:
            f.write("## Unmapped script variables\n\n")
            f.write("These variable names appeared in `<var>:<method>` calls but\n")
            f.write("aren't in the CONVENTIONS table. If they're objects of a known\n")
            f.write("UserData type, extend `tools/garlemald_lua_coverage.py`'s\n")
            f.write("`CONVENTIONS` dict to include them. (Many here will be Lua\n")
            f.write("locals like `data`, `temp`, `i` etc. — false positives.)\n\n")
            f.write("<details>\n<summary>Expand</summary>\n\n")
            for v in sorted(unmapped):
                f.write(f"  - `{v}`\n")
            f.write("\n</details>\n")

    print(f"Wrote {out_json.relative_to(REPO_ROOT)}", file=sys.stderr)
    print(f"Wrote {out_md.relative_to(REPO_ROOT)}", file=sys.stderr)
    print(f"Summary: {len(all_types)} types, {total_bound} bindings, "
          f"{total_called} distinct calls, {total_true_gap} TRUE gaps, "
          f"{total_cross_type} cross-type, {total_dead} dead bindings")
    return 0


if __name__ == "__main__":
    sys.exit(main())
