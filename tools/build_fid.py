#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Build + apply a Ghidra Function ID (FidDb) library-signature database for
the MSVC-2005 (VC8) static CRT/STL the FFXIV 1.x client links against, so
the thousands of CRT/STL functions currently sitting in the work pool as
FUN_xxxxxxxx get named — and auto-reclassified out of the pool.

WHY THIS HELPS: config/<bin>.middleware.json shows the heuristic classifier
flags only ~149 of ~94,700 functions as library. A 33 MB static-MSVC binary
has thousands of CRT/STL functions; they're currently indistinguishable
from game logic in the `matching` pool. FidDb names them; then
build_split_yaml.py's existing `std::` / CRT / MFC / zlib name patterns
auto-reclassify them `matching` -> `middleware-*` on the next `make split`.
(So no new classifier code is needed — only the names.)

KEY GOTCHA (validated): `analyzeHeadless -import foo.lib` FAILS ("no load
spec") — Ghidra can't load a COFF *archive*. So we `llvm-ar x` the .lib into
its .obj members (Ghidra loads single .obj natively) and import those. macOS
BSD `ar` mangles MS COFF member names; `llvm-ar` handles them.

SCOPE: only the true third-party CRT/STL is signature-matchable. The bulk of
the binary is Square Enix's own engine (CDev/Rapture: engine_cdev ~5k, net
~2.5k, render ~900) — NOT library code; FidDb can't touch it. DX9 is
dynamically linked (d3dx9_41.dll) so D3DX isn't in the binary either. The
win is concentrated in CRT/STL.

Subcommands:
  extract   llvm-ar x the configured libs -> build/fid/objs/<lib>/
  import    analyzeHeadless-import the .obj into a FID project (FID/LID off)
  populate  CreateMultipleLibraries -> build/fid/ffxiv_vc8.fidb
  gen       extract + import + populate (the full one-time build)
  apply     attach the fidb to the ffxivgame project, re-run the Function ID
            analyzer, re-dump symbols.json (then `make split` reclassifies)

The populate step drives Ghidra's stock CreateMultipleLibraries via a headless
.properties file. If it balks (headless ask* is finicky), the runbook
docs/fid_signature_matching.md documents the 3-click GUI fallback for this
one-time step; `apply` is fully scripted either way.
"""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
FID_DIR = REPO_ROOT / "build" / "fid"
FIDB = FID_DIR / "ffxiv_vc8.fidb"
LANG_ID = "x86:LE:32:default"
LIB_FAMILY, LIB_VERSION, LIB_VARIANT = "MSVC", "8.0", "x86"

# x86 VC8 static CRT (/MT) + STL — the libs ffxivgame.exe statically links.
DEFAULT_LIBS = [
    "vstudio2005-workspace/cache/vc8-clean/Program Files/Microsoft Visual Studio 8/VC/lib/libcmt.lib",
    "vstudio2005-workspace/cache/vc8-clean/Program Files/Microsoft Visual Studio 8/VC/lib/libcpmt.lib",
]


def find_ghidra() -> Path:
    cellar = Path("/opt/homebrew/Cellar/ghidra")
    vs = sorted((p for p in cellar.iterdir() if (p / "libexec/support/launch.sh").exists()),
                key=lambda p: tuple(int(x) for x in p.name.split(".") if x.isdigit()))
    return (vs[-1] / "libexec") if vs else Path(os.environ.get("GHIDRA_HOME", ""))


def find_java() -> str:
    cellar = Path("/opt/homebrew/Cellar/openjdk@21")
    if cellar.is_dir():
        vs = sorted(cellar.iterdir(), key=lambda p: tuple(int(x) for x in p.name.split(".") if x.isdigit()))
        if vs:
            return str(vs[-1] / "libexec/openjdk.jdk/Contents/Home")
    return os.environ.get("JAVA_HOME", "")


def find_llvm_ar() -> str:
    for c in ("llvm-ar", "/opt/homebrew/opt/llvm/bin/llvm-ar", "/opt/homebrew/bin/llvm-ar"):
        if shutil.which(c) or Path(c).exists():
            return c
    raise SystemExit("llvm-ar not found (brew install llvm). BSD ar cannot extract MS COFF archives.")


def run_headless(gh: Path, jh: str, proj_loc: Path, proj_name: str, rest: list[str],
                 mem: str = "8G") -> int:
    cmd = [str(gh / "support/launch.sh"), "fg", "jdk", "Ghidra-Headless", mem, "",
           "ghidra.app.util.headless.AnalyzeHeadless", str(proj_loc), proj_name, *rest]
    env = os.environ.copy()
    env["JAVA_HOME"] = jh
    env["PATH"] = f"{jh}/bin:" + env.get("PATH", "")
    print(">>>", " ".join(cmd))
    return subprocess.run(cmd, env=env).returncode


def cmd_extract(args) -> int:
    llvm_ar = find_llvm_ar()
    objs_root = FID_DIR / "objs"
    if objs_root.exists():
        shutil.rmtree(objs_root)
    total = 0
    for lib in args.libs:
        libp = (REPO_ROOT.parent / lib) if not Path(lib).is_absolute() else Path(lib)
        if not libp.exists():
            print(f"ERROR: lib not found: {libp}", file=sys.stderr)
            return 1
        dest = objs_root / Path(lib).stem
        dest.mkdir(parents=True, exist_ok=True)
        subprocess.run([llvm_ar, "x", str(libp)], cwd=dest, check=True)
        n = len(list(dest.glob("*.obj")))
        print(f"  {libp.name}: extracted {n} .obj into {dest.relative_to(REPO_ROOT)}")
        total += n
    print(f"extracted ~{total} members under {objs_root.relative_to(REPO_ROOT)}")
    return 0


def cmd_import(args) -> int:
    gh, jh = find_ghidra(), find_java()
    fidscripts = gh / "Ghidra/Features/FunctionID/ghidra_scripts"
    objs_root = FID_DIR / "objs"
    proj_loc = FID_DIR / "proj"
    proj_loc.mkdir(parents=True, exist_ok=True)
    # import into folder /<family>/<version>/<variant> so CreateMultipleLibraries
    # (MASTER_DEPTH=3) reads name/version/variant from the path.
    proj_name = f"FidLibs/{LIB_FAMILY}/{LIB_VERSION}/{LIB_VARIANT}"
    rc = 0
    for sub in sorted(objs_root.iterdir()):
        if not sub.is_dir():
            continue
        rc = run_headless(gh, jh, proj_loc, proj_name, [
            "-import", str(sub), "-recursive",
            "-scriptPath", str(fidscripts),
            "-preScript", "FunctionIDHeadlessPrescript.java",
            "-postScript", "FunctionIDHeadlessPostscript.java",
        ])
        if rc != 0:
            return rc
    return rc


def _write_properties(fidscripts: Path):
    """Headless answers for the stock CreateMultipleLibraries prompts."""
    common = FID_DIR / "common_symbols.txt"
    common.write_text("", encoding="utf-8")  # empty = no common-symbol suppression
    props = {
        "Do Duplication Detection": "false",
        "Choose destination FidDB": FIDB.name,
        f"Select root folder containing all libraries (at a depth of 3):": "/",
        "Common symbols file (optional):": str(common),
        "Enter LanguageID To Process": LANG_ID,
    }
    text = "".join(f"{k} = {v}\n" for k, v in props.items())
    (fidscripts / "CreateMultipleLibraries.properties").write_text(text, encoding="utf-8")
    (fidscripts / "CreateEmptyFidDatabase.properties").write_text(
        f"Create new FidDb file = {FIDB}\n", encoding="utf-8")


def cmd_populate(args) -> int:
    gh, jh = find_ghidra(), find_java()
    fidscripts = gh / "Ghidra/Features/FunctionID/ghidra_scripts"
    proj_loc = FID_DIR / "proj"
    FID_DIR.mkdir(parents=True, exist_ok=True)
    if FIDB.exists():
        FIDB.unlink()
    _write_properties(fidscripts)
    # run on the imported project: create empty fidb (attaches it), then populate
    return run_headless(gh, jh, proj_loc, "FidLibs", [
        "-process", "-noanalysis", "-scriptPath", str(fidscripts),
        "-preScript", "CreateEmptyFidDatabase.java",
        "-postScript", "CreateMultipleLibraries.java",
    ])


def cmd_apply(args) -> int:
    gh, jh = find_ghidra(), find_java()
    fidscripts = gh / "Ghidra/Features/FunctionID/ghidra_scripts"
    proj_loc = REPO_ROOT / "build" / "ghidra"
    if not FIDB.exists():
        print(f"ERROR: {FIDB} not found — run `build_fid.py gen` first.", file=sys.stderr)
        return 1
    (fidscripts / "AttachFidDatabase.properties").write_text(
        f"Attach existing FidDb = {FIDB}\n", encoding="utf-8")
    print(">>> attaching FidDb + re-running Function ID analyzer on ffxivgame, then re-dumping")
    rc = run_headless(gh, jh, proj_loc, "ffxivgame", [
        "-process", "ffxivgame.exe",
        "-scriptPath", str(fidscripts), str(REPO_ROOT / "tools/ghidra_scripts"),
        "-preScript", "AttachFidDatabase.java",
        "-postScript", "DumpFunctions.java",
    ])
    if rc == 0:
        print("\nDONE. Now run `make split BINARY=ffxivgame.exe` — build_split_yaml's\n"
              "existing std::/CRT/MFC patterns will reclassify the newly-named\n"
              "library functions matching -> middleware-* automatically.")
    return rc


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    pe = sub.add_parser("extract"); pe.add_argument("--libs", nargs="+", default=DEFAULT_LIBS)
    sub.add_parser("import"); sub.add_parser("populate"); sub.add_parser("apply")
    pg = sub.add_parser("gen"); pg.add_argument("--libs", nargs="+", default=DEFAULT_LIBS)
    args = ap.parse_args()

    if args.cmd == "extract":
        return cmd_extract(args)
    if args.cmd == "import":
        return cmd_import(args)
    if args.cmd == "populate":
        return cmd_populate(args)
    if args.cmd == "apply":
        return cmd_apply(args)
    if args.cmd == "gen":
        for step in (cmd_extract, cmd_import, cmd_populate):
            rc = step(args)
            if rc != 0:
                print(f"gen aborted at {step.__name__} (rc={rc})", file=sys.stderr)
                return rc
        print(f"\nFidDb built: {FIDB.relative_to(REPO_ROOT)}. Next: `build_fid.py apply`.")
        return 0
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
