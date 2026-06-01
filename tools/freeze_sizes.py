#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Freeze a committed, toolchain-free per-function size manifest.

For each binary, read the (gitignored, Ghidra-derived)
`config/<bin>.symbols.json` plus the (committed)
`config/<bin>.size_overrides.json`, apply the override correction, and
write a compact, deterministic `config/<bin>.func_sizes.json` mapping
every function's decimal RVA → its effective byte size.

Why: the durable "solved" signal is the committed
`src/<bin>/_rosetta/FUN_<va>.cpp` tree, but turning a file count into a
*byte* count needs per-function sizes. Those sizes live in the
gitignored `symbols.json`, so in a clean CI checkout the byte metric
collapses to zero. `func_sizes.json` is the committed, deterministic
sizes source that decouples the byte metric from the toolchain.

The manifest covers EVERY symbol (not only solved ones) so a future
match's bytes are always resolvable without regenerating symbols.json.

Effective size = the override's `new_size` when an override exists for
that RVA, else the symbols `size`.

Usage:
  tools/freeze_sizes.py                 # all five binaries
  tools/freeze_sizes.py ffxivgame       # one binary
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CONFIG_DIR = REPO_ROOT / "config"

ALL_BINARIES = ("ffxivboot", "ffxivconfig", "ffxivgame", "ffxivlogin", "ffxivupdater")


def load_overrides(stem: str) -> dict[int, int]:
    """rva → new_size for every override row (last write wins on dup rva)."""
    path = CONFIG_DIR / f"{stem}.size_overrides.json"
    if not path.exists():
        return {}
    rows = json.loads(path.read_text())
    return {int(r["rva"]): int(r["new_size"]) for r in rows}


def build_manifest(stem: str) -> dict[str, int] | None:
    """Return {decimal_rva_str: effective_size}, sorted by integer rva.

    Returns None if the symbols.json input is absent.
    """
    sym_path = CONFIG_DIR / f"{stem}.symbols.json"
    if not sym_path.exists():
        return None
    syms = json.loads(sym_path.read_text())
    overrides = load_overrides(stem)

    # Effective size per rva. If symbols.json carries duplicate rvas
    # (it shouldn't — they're unique today), last write wins, matching
    # the override dict semantics above.
    effective: dict[int, int] = {}
    for s in syms:
        rva = int(s["rva"])
        size = overrides.get(rva, int(s.get("size", 0)))
        effective[rva] = size

    return {str(rva): effective[rva] for rva in sorted(effective)}


def write_manifest(stem: str, manifest: dict[str, int]) -> Path:
    """Write a compact, deterministic JSON object with a trailing newline."""
    out_path = CONFIG_DIR / f"{stem}.func_sizes.json"
    # Compact separators, keys already in integer-rva sorted order, plus
    # a trailing newline so the file is git-stable and POSIX-clean.
    text = json.dumps(manifest, separators=(",", ":")) + "\n"
    out_path.write_text(text)
    return out_path


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    ap.add_argument("binary", nargs="?", help="binary stem (default: all five)")
    args = ap.parse_args()

    binaries = (args.binary,) if args.binary else ALL_BINARIES

    print("=== freeze_sizes ===")
    any_written = False
    for stem in binaries:
        manifest = build_manifest(stem)
        if manifest is None:
            print(f"  {stem}: no symbols.json ({stem}.symbols.json) — skipping")
            continue
        out_path = write_manifest(stem, manifest)
        any_written = True
        print(f"  {stem}: wrote {out_path.name}  entries={len(manifest):>6d}")

    if not any_written:
        print("note: no symbols.json found for any binary — nothing frozen", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
