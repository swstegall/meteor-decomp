#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Compute project progress from `config/<binary>.yaml` work pool files.

Reports:
  - per-binary  : matched / functional / wip / unmatched / middleware
  - per-module  : same breakdown, sorted descending by total size
  - overall     : matched-byte percentage, contributor-claimed count

Until Phase 1 generates the YAML files, this script reports zero.
"""

from __future__ import annotations

import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CONFIG_DIR = REPO_ROOT / "config"

STATUSES = ("matched", "functional", "wip", "unmatched")
MIDDLEWARE = ("middleware-crt", "middleware-miles", "middleware-dx9", "middleware-stl")


def try_yaml():
    try:
        import yaml  # type: ignore
        return yaml
    except ImportError:
        return None


def main() -> int:
    yaml = try_yaml()
    if yaml is None:
        print("note: PyYAML not installed (`pip install pyyaml`)", file=sys.stderr)
        # Soft-fail; print the placeholder so make doesn't abort.

    yamls = sorted(CONFIG_DIR.glob("*.yaml")) if CONFIG_DIR.exists() else []
    if not yamls:
        print("(no work-pool yaml found yet — Phase 1 deliverable)")
        return 0

    if yaml is None:
        print("(install PyYAML to summarise)", file=sys.stderr)
        return 0

    grand = {s: {"count": 0, "bytes": 0} for s in (*STATUSES, *MIDDLEWARE)}
    for path in yamls:
        rows = yaml.safe_load(path.read_text()) or []
        if not isinstance(rows, list):
            continue
        per = {s: {"count": 0, "bytes": 0} for s in (*STATUSES, *MIDDLEWARE)}
        for r in rows:
            status = r.get("status", "unmatched")
            size = int(r.get("size", 0))
            tier = r.get("type", "matching")
            key = tier if tier in MIDDLEWARE else status
            if key in per:
                per[key]["count"] += 1
                per[key]["bytes"] += size
                grand[key]["count"] += 1
                grand[key]["bytes"] += size

        print(f"=== {path.name} ===")
        for k, v in per.items():
            if v["count"]:
                print(f"  {k:18s}  count={v['count']:>6d}  bytes={v['bytes']:>10,}")

    total_bytes = sum(v["bytes"] for v in grand.values())
    matched_bytes = grand["matched"]["bytes"] + grand["functional"]["bytes"]
    if total_bytes:
        pct = 100.0 * matched_bytes / total_bytes
        print(f"\noverall: matched/functional = {matched_bytes:,} / {total_bytes:,} bytes  ({pct:.2f}%)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
