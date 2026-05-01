#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Drive Ghidra in headless mode to:
  1. Create a project at build/ghidra/<binary>.gpr
  2. Import the binary from orig/<binary>.exe
  3. Run auto-analysis (with the Microsoft RTTI Analyzer enabled)
  4. Run our post-analysis scripts in tools/ghidra_scripts/:
       - DumpFunctions.java
       - DumpStrings.java
       - DumpRtti.java

Outputs:
  build/ghidra/<binary>.gpr    Ghidra project (re-runnable via -process)
  asm/<binary>/                one .s per function (RVA-prefixed filename)
  config/<binary>.symbols.json full function list with sizes + sections
  config/<binary>.strings.json every defined string with seed-hint flags
  config/<binary>.rtti.json    every recovered vtable / class

Usage:
  GHIDRA_HOME=/opt/homebrew/Cellar/ghidra/12.0.4/libexec \\
  python3 tools/import_to_ghidra.py ffxivlogin.exe

GHIDRA_HOME defaults to the brew install path if unset.
"""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_GHIDRA_HOME = "/opt/homebrew/Cellar/ghidra/12.0.4/libexec"
DEFAULT_JAVA_HOME = (
    "/opt/homebrew/Cellar/openjdk@21/21.0.11/libexec/openjdk.jdk/Contents/Home"
)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", help="binary name in orig/, e.g. ffxivgame.exe")
    ap.add_argument(
        "--ghidra-home",
        default=os.environ.get("GHIDRA_HOME", DEFAULT_GHIDRA_HOME),
        help=f"path to Ghidra install root (default: $GHIDRA_HOME or {DEFAULT_GHIDRA_HOME})",
    )
    ap.add_argument(
        "--java-home",
        default=os.environ.get("JAVA_HOME", DEFAULT_JAVA_HOME),
        help=f"JDK 21 path (default: $JAVA_HOME or {DEFAULT_JAVA_HOME})",
    )
    ap.add_argument(
        "--reanalyze",
        action="store_true",
        help="overwrite the existing Ghidra project and re-import the binary",
    )
    ap.add_argument(
        "--skip-import",
        action="store_true",
        help="re-run scripts only against an existing project (skip auto-analysis)",
    )
    ap.add_argument(
        "--analysis-timeout",
        type=int,
        default=0,
        help="cap auto-analysis at N seconds (0 = unlimited; ffxivgame.exe takes ~30-60 min)",
    )
    ap.add_argument(
        "--max-memory",
        default="8G",
        help="JVM max heap (-Xmx). Brew default is 2G; ffxivgame.exe needs ~6G+. (default: 8G)",
    )
    args = ap.parse_args()

    ghidra = Path(args.ghidra_home)
    launch = ghidra / "support" / "launch.sh"
    if not launch.exists():
        print(f"error: not a Ghidra install: {ghidra}  (no support/launch.sh)", file=sys.stderr)
        return 1

    src = REPO_ROOT / "orig" / args.binary
    if not src.exists():
        print(f"error: missing {src}; run tools/symlink_orig.sh", file=sys.stderr)
        return 1

    project_dir = REPO_ROOT / "build" / "ghidra"
    project_dir.mkdir(parents=True, exist_ok=True)
    project_name = Path(args.binary).stem
    scripts = REPO_ROOT / "tools" / "ghidra_scripts"

    # If reanalyzing, blow away the project so headless can re-import cleanly.
    project_marker = project_dir / f"{project_name}.gpr"
    if args.reanalyze:
        for p in project_dir.glob(f"{project_name}.*"):
            if p.is_file():
                p.unlink()
            elif p.is_dir():
                shutil.rmtree(p)

    has_project = project_marker.exists()
    # Mirror analyzeHeadless's call to launch.sh, but with our own --max-memory.
    headless_args: list[str] = [str(project_dir), project_name]
    if has_project and args.skip_import:
        headless_args += ["-process", args.binary, "-noanalysis"]
    elif has_project:
        headless_args += ["-process", args.binary]
    else:
        headless_args += ["-import", str(src)]

    headless_args += [
        "-scriptPath",
        str(scripts),
        "-postScript",
        "DumpFunctions.java",
        "-postScript",
        "DumpStrings.java",
        "-postScript",
        "DumpRtti.java",
    ]
    if args.analysis_timeout:
        headless_args += ["-analysisTimeoutPerFile", str(args.analysis_timeout)]

    # launch.sh signature:
    #   launch.sh <mode> <java-type> <name> <max-memory> <vmarg-list> <classname> <args>...
    cmd = [
        str(launch),
        "fg",
        "jdk",
        "Ghidra-Headless",
        args.max_memory,
        "",  # no extra vmargs
        "ghidra.app.util.headless.AnalyzeHeadless",
        *headless_args,
    ]

    env = os.environ.copy()
    env["JAVA_HOME"] = args.java_home
    env["METEOR_DECOMP_ROOT"] = str(REPO_ROOT)
    env["PATH"] = f"{args.java_home}/bin:" + env.get("PATH", "")

    print(f">>> JAVA_HOME={args.java_home}")
    print(f">>> METEOR_DECOMP_ROOT={REPO_ROOT}")
    print(">>> ghidra:", " ".join(cmd))
    try:
        subprocess.run(cmd, check=True, env=env)
    except subprocess.CalledProcessError as e:
        print(f"error: ghidra headless failed (exit {e.returncode})", file=sys.stderr)
        return e.returncode
    return 0


if __name__ == "__main__":
    sys.exit(main())
