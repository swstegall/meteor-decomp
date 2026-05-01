#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Phase 1: drive Ghidra in headless mode to:

  1. Create a project at build/ghidra/<binary>.gpr
  2. Import the binary from orig/<binary>.exe
  3. Run auto-analysis (with RTTI analyser enabled)
  4. Run our post-analysis scripts:
       - ghidra_scripts/dump_functions.py
       - ghidra_scripts/dump_strings.py
       - ghidra_scripts/dump_rtti.py

NOT WIRED UP YET. This is a stub that documents the intended pipeline
and what the wrapper will look like; flesh it out in Phase 1 once
Ghidra + JDK 17 are installed and verified locally. Tracking issue:
PLAN.md §6 Phase 1.

Once wired:

    GHIDRA_HOME=/Applications/ghidra/ghidra_11.0_PUBLIC \\
    python3 tools/import_to_ghidra.py ffxivgame.exe

writes:

    build/ghidra/ffxivgame.gpr
    config/strings.json
    config/rtti.json
    asm/ffxivgame/<rva>_<symbol>.s        (one per function)
    config/ffxivgame.symbols.json         (full symbol map)
"""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("binary", help="binary name in orig/, e.g. ffxivgame.exe")
    ap.add_argument(
        "--ghidra-home",
        default=os.environ.get("GHIDRA_HOME"),
        help="path to Ghidra install (or set $GHIDRA_HOME)",
    )
    ap.add_argument("--reanalyze", action="store_true", help="re-run analysis even if project exists")
    args = ap.parse_args()

    if not args.ghidra_home:
        print("error: --ghidra-home or $GHIDRA_HOME required", file=sys.stderr)
        return 1
    ghidra = Path(args.ghidra_home)
    headless = ghidra / "support" / "analyzeHeadless"
    if not headless.exists():
        print(f"error: not a Ghidra install: {ghidra}  (no support/analyzeHeadless)", file=sys.stderr)
        return 1

    src = REPO_ROOT / "orig" / args.binary
    if not src.exists():
        print(f"error: missing {src}; run tools/symlink_orig.sh", file=sys.stderr)
        return 1

    project_dir = REPO_ROOT / "build" / "ghidra"
    project_dir.mkdir(parents=True, exist_ok=True)
    project_name = Path(args.binary).stem
    scripts = REPO_ROOT / "tools" / "ghidra_scripts"

    # NOTE: this assumes Ghidra 11.x argument shape; verify against
    # `analyzeHeadless --help` output if the version drifts.
    cmd = [
        str(headless),
        str(project_dir),
        project_name,
        "-import" if not (project_dir / f"{project_name}.gpr").exists() else "-process",
        str(src),
        "-scriptPath",
        str(scripts),
        "-postScript",
        "dump_functions.py",
        "-postScript",
        "dump_strings.py",
        "-postScript",
        "dump_rtti.py",
    ]
    if args.reanalyze:
        cmd.append("-overwrite")

    print(">>> ghidra:", " ".join(cmd))
    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError as e:
        print(f"error: ghidra headless failed (exit {e.returncode})", file=sys.stderr)
        return e.returncode
    return 0


if __name__ == "__main__":
    sys.exit(main())
