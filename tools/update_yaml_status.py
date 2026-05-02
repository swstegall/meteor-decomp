#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Sync `config/<binary>.yaml` `status:` fields from
`build/easy_wins/<binary>.validate_results.json`.

Pipeline:
  derive_templates / stamp_clusters → src/<bin>/_rosetta/*.cpp
  validate_clusters → build/easy_wins/<bin>.validate_results.json
  update_yaml_status (this tool) → flips matching YAML entries
                                   `status: unmatched → matched`

Why a separate tool: validate_clusters is the heavy step (compile +
clone + compare across thousands of .cpp). The YAML edit is a small
sync-only step we can run any time on the latest results JSON.

YAML edit is line-oriented (regex-based). The schema is fixed by
`build_split_yaml.py` so we don't need a real YAML parser; we
preserve every other key verbatim and only rewrite `status:`.

Idempotent. Safe to re-run. Reports per-binary deltas.

Usage:
  tools/update_yaml_status.py ffxivboot
  tools/update_yaml_status.py ffxivgame --dry-run
  tools/update_yaml_status.py            # all five binaries
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CONFIG = REPO_ROOT / "config"
EASY_WINS = REPO_ROOT / "build" / "easy_wins"

ALL_BINARIES = ("ffxivboot", "ffxivconfig", "ffxivgame", "ffxivlogin", "ffxivupdater")

RE_RVA = re.compile(r"^\s*-\s*rva:\s*0x([0-9a-fA-F]+)\s*$")
RE_STATUS = re.compile(r"^(\s*)status:\s*(\S+)\s*$")
RE_TYPE = re.compile(r"^\s*type:\s*(\S+)\s*$")


IMAGE_BASE = 0x400000


def sync_yaml(stem: str, dry_run: bool) -> tuple[int, int, int]:
    """Returns (flipped, already_matched, missing)."""
    results_path = EASY_WINS / f"{stem}.validate_results.json"
    if not results_path.exists():
        print(f"  {stem}: no results file ({results_path.name}) — skipping")
        return (0, 0, 0)

    results = json.loads(results_path.read_text())
    # .cpp filenames encode the full VA (e.g. FUN_00402d40 → 0x402d40);
    # YAML stores image-relative RVA (e.g. 0x00002d40 = VA − 0x400000).
    green_rvas: set[int] = {
        r["rva"] - IMAGE_BASE
        for r in results
        if r.get("verdict") == "GREEN" and r.get("rva") is not None
    }

    yaml_path = CONFIG / f"{stem}.yaml"
    if not yaml_path.exists():
        print(f"  {stem}: no yaml ({yaml_path.name}) — skipping")
        return (0, 0, 0)

    lines = yaml_path.read_text().splitlines(keepends=True)
    flipped = already = 0
    out: list[str] = []

    # Each YAML record is a contiguous block starting with "- rva:". Track the
    # current record's rva + type so we know whether to rewrite a `status:`
    # line within it.
    current_rva: int | None = None
    current_type: str | None = None

    for line in lines:
        m_rva = RE_RVA.match(line)
        if m_rva:
            current_rva = int(m_rva.group(1), 16)
            current_type = None
            out.append(line)
            continue

        m_type = RE_TYPE.match(line)
        if m_type and current_rva is not None:
            current_type = m_type.group(1)
            out.append(line)
            continue

        m_status = RE_STATUS.match(line)
        if (m_status and current_rva is not None and current_type == "matching"
                and current_rva in green_rvas):
            existing = m_status.group(2)
            if existing == "matched":
                already += 1
                out.append(line)
            else:
                indent = m_status.group(1)
                out.append(f"{indent}status: matched\n")
                flipped += 1
            continue

        out.append(line)

    if dry_run:
        print(f"  {stem}: would flip {flipped} (already matched: {already})  [dry-run]")
    else:
        yaml_path.write_text("".join(out))
        print(f"  {stem}: flipped {flipped} (already matched: {already})")

    # Sanity check: every GREEN rva should map to a `matching` YAML row.
    missing = sum(1 for r in green_rvas if not _yaml_has_matching(out, r))
    return (flipped, already, missing)


def _yaml_has_matching(lines: list[str], target_rva: int) -> bool:
    """Check that the (regenerated) YAML has a `matching` record at this rva."""
    target_re = re.compile(rf"^\s*-\s*rva:\s*0x0*{target_rva:x}\s*$", re.IGNORECASE)
    in_block = False
    for line in lines:
        if target_re.match(line):
            in_block = True
            continue
        if in_block and line.lstrip().startswith("- rva:"):
            in_block = False
        if in_block:
            mt = RE_TYPE.match(line)
            if mt and mt.group(1) == "matching":
                return True
    return False


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", nargs="?", help="binary stem (default: all five)")
    ap.add_argument("--dry-run", action="store_true", help="print deltas without rewriting YAML")
    args = ap.parse_args()

    binaries = (args.binary,) if args.binary else ALL_BINARIES

    print(f"=== update_yaml_status (dry-run={args.dry_run}) ===")
    total_flipped = total_already = total_missing = 0
    for stem in binaries:
        flipped, already, missing = sync_yaml(stem, args.dry_run)
        total_flipped += flipped
        total_already += already
        total_missing += missing

    print(f"\ntotal: flipped={total_flipped}  already_matched={total_already}  green_without_yaml_match={total_missing}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
