#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Resource-id → DAT path resolver. Python reference for the
# PathBuilder.cpp functional decomp at RVA 0x0044b3a0.
#
# Usage:
#   tools/sqpack_path.py 0x12345678
#       → \data\12\34\56\78.DAT  (Windows-style separators)
#   tools/sqpack_path.py 0x12345678 --posix
#       → /data/12/34/56/78.DAT
#   tools/sqpack_path.py 0x12345678 --root /path/to/game
#       → /path/to/game/data/12/34/56/78.DAT
#       (auto-uses POSIX separators when --root is set)
#   tools/sqpack_path.py --scan /path/to/game
#       Walk the game's data dir and print every (resource_id, path)
#       pair found. Useful for building a working-resource catalog.

import argparse
import os
import re
import sys


def build_path(resource_id: int, posix: bool = False) -> str:
    """Mirror of build_resource_path(_posix) in PathBuilder.cpp."""
    b3 = (resource_id >> 24) & 0xFF
    b2 = (resource_id >> 16) & 0xFF
    b1 = (resource_id >>  8) & 0xFF
    b0 = (resource_id >>  0) & 0xFF
    sep = "/" if posix else "\\"
    return f"{sep}data{sep}{b3:02X}{sep}{b2:02X}{sep}{b1:02X}{sep}{b0:02X}.DAT"


_PATH_RE = re.compile(
    r"data[\\/](?P<b3>[0-9A-Fa-f]{2})[\\/]"
    r"(?P<b2>[0-9A-Fa-f]{2})[\\/]"
    r"(?P<b1>[0-9A-Fa-f]{2})[\\/]"
    r"(?P<b0>[0-9A-Fa-f]{2})\.DAT$"
)


def parse_path(path: str) -> int | None:
    """Inverse of build_path — extract resource_id from a known-format
    path. Returns None if path doesn't match the data\\BB\\BB\\BB\\BB.DAT
    pattern."""
    m = _PATH_RE.search(path.replace("\\", "/"))
    if not m:
        return None
    b3 = int(m.group("b3"), 16)
    b2 = int(m.group("b2"), 16)
    b1 = int(m.group("b1"), 16)
    b0 = int(m.group("b0"), 16)
    return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0


def scan_data_dir(game_root: str):
    """Walk <game_root>/data/ and yield (resource_id, full_path) for
    every file that matches the standard resource-id naming."""
    data_dir = os.path.join(game_root, "data")
    if not os.path.isdir(data_dir):
        print(f"error: not a directory: {data_dir}", file=sys.stderr)
        sys.exit(1)
    for dirpath, _, files in os.walk(data_dir):
        for f in files:
            full = os.path.join(dirpath, f)
            rid = parse_path(full)
            if rid is not None:
                yield rid, full


def main() -> int:
    ap = argparse.ArgumentParser(
        description="Resource-id → DAT path resolver (Python ref for "
                    "PathBuilder.cpp at RVA 0x0044b3a0).",
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("resource_id", nargs="?",
                    help="resource_id (hex with 0x prefix or decimal); omit when --scan is given")
    ap.add_argument("--posix", action="store_true",
                    help="use / separators instead of \\")
    ap.add_argument("--root", help="prefix output with this game-root (forces --posix)")
    ap.add_argument("--scan", metavar="GAME_ROOT",
                    help="walk GAME_ROOT/data/ and print every found (resource_id, path)")
    args = ap.parse_args()

    if args.scan:
        for rid, path in scan_data_dir(args.scan):
            print(f"0x{rid:08x}  {path}")
        return 0

    if not args.resource_id:
        ap.error("resource_id required (or use --scan GAME_ROOT)")

    rid = int(args.resource_id, 0)
    use_posix = args.posix or args.root is not None
    rel = build_path(rid, posix=use_posix)
    if args.root:
        # Strip the leading separator before joining.
        print(os.path.join(args.root, rel.lstrip("/").lstrip("\\")))
    else:
        print(rel)
    return 0


if __name__ == "__main__":
    sys.exit(main())
