#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Compile-once-share-many cluster validator.

`make rosetta-bulk` invokes cl.exe per `.cpp` file. For binaries with
hundreds of stamped cluster siblings (every member of which is
guaranteed to compile to the same .obj `.text` bytes by the cluster-
hash invariant), this re-compiles thousands of identical sources.
Each cl.exe invocation under Wine costs ~5 seconds, so a 1500-file
bulk takes ~2 hours when the actual unique work is ~20 compiles.

This tool:
  1. Walks `src/<bin>/_rosetta/*.cpp`.
  2. Looks up each .cpp's cluster in `build/easy_wins/<bin>.clusters.json`.
     - .cpp's whose RVA appears in a multi-member cluster share a "primary"
       (chosen as the alphabetically-first cluster member that exists as a
       .cpp under _rosetta/).
     - .cpp's whose RVA is NOT in any multi-member cluster (singletons or
       hash-misses) are compiled individually.
  3. **Compiles only the primaries** (one cl.exe per cluster). Singletons
     are compiled normally.
  4. **Clones each primary's .obj to every sibling's expected obj path**
     (file copy — sub-millisecond).
  5. Runs `compare.py` on every .cpp (cheap — ~50 ms each).
  6. Prints aggregate GREEN / PARTIAL / MISMATCH counts.

Idempotent: skips compile if the target .obj already exists and is
newer than the .cpp source. Skips clone if the sibling .obj already
exists and is newer than the primary's.

Speedup on ffxivboot.exe (1,588 .cpp files across 18 clusters):
  make rosetta-bulk:  ~120 min  (1,588 cl.exe invocations)
  validate_clusters:  ~3 min    (18 cl.exe + 1,570 file copies)

Usage:
  tools/validate_clusters.py ffxivboot
  tools/validate_clusters.py ffxivboot --no-compare    # skip compare.py
  tools/validate_clusters.py ffxivboot --rebuild       # ignore existing .obj
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import time
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC_DIR = REPO_ROOT / "src"
BUILD_OBJ = REPO_ROOT / "build" / "obj" / "_rosetta"
EASY_WINS = REPO_ROOT / "build" / "easy_wins"
TOOLS = REPO_ROOT / "tools"
CL_WINE = TOOLS / "cl-wine.sh"
COMPARE = TOOLS / "compare.py"

ROSETTA_FLAGS = (
    "/c /O2 /Oy /GR /EHsc /Gy /GS /MT "
    "/Zc:wchar_t /Zc:forScope /TP"
).split()

RE_FUN_NAME = re.compile(r"^FUN_([0-9a-fA-F]+)$")


def parse_va(cpp_path: Path) -> int | None:
    m = RE_FUN_NAME.match(cpp_path.stem)
    return int(m.group(1), 16) if m else None


def needs_rebuild(obj: Path, src: Path) -> bool:
    if not obj.exists():
        return True
    return src.stat().st_mtime > obj.stat().st_mtime


def needs_clone(sibling_obj: Path, primary_obj: Path) -> bool:
    if not sibling_obj.exists():
        return True
    return primary_obj.stat().st_mtime > sibling_obj.stat().st_mtime


def compile_one(src_cpp: Path, obj_out: Path) -> bool:
    """Run cl-wine.sh on one .cpp. Returns True on success.

    Paths are made relative to REPO_ROOT before being passed to cl.exe —
    the absolute form starts with `/U` (or `/O`, `/G`, etc.) on POSIX,
    which cl.exe parses as flag prefixes. Relative paths sidestep that
    entirely, and the Makefile's `make rosetta-bulk` target relies on
    the same trick (`for cpp in src/<bin>/_rosetta/*.cpp` — the loop
    variable carries the relative path).
    """
    obj_out.parent.mkdir(parents=True, exist_ok=True)
    rel_src = src_cpp.relative_to(REPO_ROOT)
    rel_obj = obj_out.relative_to(REPO_ROOT)
    cmd = [str(CL_WINE), *ROSETTA_FLAGS, f"/Fo{rel_obj}", str(rel_src)]
    try:
        subprocess.run(cmd, check=True, capture_output=True, text=True, cwd=str(REPO_ROOT))
        return True
    except subprocess.CalledProcessError as e:
        print(f"  cl FAILED: {src_cpp.name}", file=sys.stderr)
        if e.stdout:
            print(e.stdout, file=sys.stderr)
        if e.stderr:
            print(e.stderr, file=sys.stderr)
        return False


def run_compare(binary_stem: str, func_name: str) -> tuple[str, str]:
    """Run compare.py and return (verdict, one-line-summary).

    Verdict is GREEN / PARTIAL / MISMATCH / ERROR.
    """
    cmd = [
        sys.executable,
        str(COMPARE),
        f"BINARY={binary_stem}.exe",
        f"FUNC={func_name}",
    ]
    proc = subprocess.run(cmd, capture_output=True, text=True)
    out = proc.stdout
    rc = proc.returncode
    summary = ""
    for line in out.splitlines():
        if any(tag in line for tag in ("GREEN", "PARTIAL", "MISMATCH")):
            summary = line.strip()
            break
    if rc == 0:
        return ("GREEN", summary or "GREEN")
    if rc == 1:
        return ("PARTIAL", summary or "PARTIAL")
    if rc == 2:
        return ("MISMATCH", summary or "MISMATCH")
    return ("ERROR", summary or f"compare.py rc={rc}")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", help="binary stem (ffxivgame, ffxivboot, etc.)")
    ap.add_argument("--rebuild", action="store_true", help="ignore existing .obj, force re-compile of primaries")
    ap.add_argument("--no-compare", action="store_true", help="skip the compare.py validation pass")
    ap.add_argument("--reloc", action="store_true",
                    help="use the relocation-aware clusters JSON (clusters_reloc.json) "
                         "instead of the exact-byte clusters JSON")
    ap.add_argument("--image-base", type=lambda s: int(s, 0), default=0x400000)
    ap.add_argument("--quiet", action="store_true", help="suppress per-file logging")
    args = ap.parse_args()

    stem = args.binary.replace(".exe", "")
    rosetta_dir = SRC_DIR / stem / "_rosetta"
    obj_dir = BUILD_OBJ / stem
    if not rosetta_dir.is_dir():
        print(f"error: {rosetta_dir} not found", file=sys.stderr)
        return 1

    clusters_filename = f"{stem}.clusters_reloc.json" if args.reloc else f"{stem}.clusters.json"
    clusters_path = EASY_WINS / clusters_filename
    clusters: dict[str, list[dict]] = {}
    if clusters_path.exists():
        clusters = json.loads(clusters_path.read_text())
        if args.reloc:
            print(f"  (using reloc-aware clusters from {clusters_path.name})")
    else:
        run_tool = "tools/cluster_relocs.py" if args.reloc else "tools/cluster_shapes.py"
        print(f"warning: {clusters_path} missing — every .cpp will be compiled individually "
              f"(run {run_tool} {stem} for cluster speedup)", file=sys.stderr)

    rva_to_hash: dict[int, str] = {}
    for h, members in clusters.items():
        for m in members:
            rva_to_hash[m["rva"]] = h

    cpps = sorted(rosetta_dir.glob("FUN_*.cpp"))
    if not cpps:
        print(f"error: no FUN_*.cpp under {rosetta_dir}", file=sys.stderr)
        return 1

    obj_dir.mkdir(parents=True, exist_ok=True)

    # ----- Phase 1: pick a primary per cluster + identify singletons. -----
    cluster_primary: dict[str, Path] = {}   # shape_hash → primary .cpp
    cpp_to_primary: dict[Path, Path] = {}    # cpp → primary cpp (or self for primaries)
    singletons: list[Path] = []
    for cpp in cpps:
        va = parse_va(cpp)
        if va is None:
            continue
        rva = va - args.image_base
        h = rva_to_hash.get(rva)
        if h is None:
            singletons.append(cpp)
            cpp_to_primary[cpp] = cpp
        else:
            if h not in cluster_primary:
                cluster_primary[h] = cpp
            cpp_to_primary[cpp] = cluster_primary[h]

    n_clusters = len(cluster_primary)
    n_cluster_members = sum(1 for c, p in cpp_to_primary.items() if c is not p)
    print(f"=== validate_clusters[{stem}] ===")
    print(f"  .cpp files:         {len(cpps):>5}")
    print(f"  cluster primaries:  {n_clusters:>5}")
    print(f"  cluster siblings:   {n_cluster_members:>5}  (will clone primary's .obj)")
    print(f"  singletons:         {len(singletons):>5}  (will compile each)")

    # ----- Phase 2: compile primaries + singletons. -----
    t0 = time.time()
    n_compiled = 0
    n_compile_skipped = 0
    n_compile_failed = 0
    to_compile = list(cluster_primary.values()) + singletons
    for src_cpp in to_compile:
        obj_out = obj_dir / (src_cpp.stem + ".obj")
        if not args.rebuild and not needs_rebuild(obj_out, src_cpp):
            n_compile_skipped += 1
            continue
        if not args.quiet:
            print(f"  cl  {src_cpp.name}")
        if compile_one(src_cpp, obj_out):
            n_compiled += 1
        else:
            n_compile_failed += 1
    t_compile = time.time() - t0
    print(f"  compile phase:      {n_compiled:>5} compiled  "
          f"{n_compile_skipped} skipped (up-to-date)  "
          f"{n_compile_failed} failed  "
          f"({t_compile:.1f}s)")

    # ----- Phase 3: clone primaries' .obj to siblings. -----
    t0 = time.time()
    n_cloned = 0
    n_clone_skipped = 0
    n_clone_failed = 0
    for cpp, primary in cpp_to_primary.items():
        if cpp is primary:
            continue
        primary_obj = obj_dir / (primary.stem + ".obj")
        sibling_obj = obj_dir / (cpp.stem + ".obj")
        if not primary_obj.exists():
            n_clone_failed += 1
            continue
        if not args.rebuild and not needs_clone(sibling_obj, primary_obj):
            n_clone_skipped += 1
            continue
        try:
            shutil.copyfile(primary_obj, sibling_obj)
            shutil.copystat(primary_obj, sibling_obj)
            n_cloned += 1
        except OSError as e:
            print(f"  clone FAILED: {sibling_obj.name}: {e}", file=sys.stderr)
            n_clone_failed += 1
    t_clone = time.time() - t0
    print(f"  clone phase:        {n_cloned:>5} cloned    "
          f"{n_clone_skipped} skipped (up-to-date)  "
          f"{n_clone_failed} failed  "
          f"({t_clone:.1f}s)")

    if args.no_compare:
        print(f"  --no-compare set — skipping verdict pass")
        return 0

    # ----- Phase 4: run compare.py per .cpp. -----
    t0 = time.time()
    verdict_counts = {"GREEN": 0, "PARTIAL": 0, "MISMATCH": 0, "ERROR": 0}
    failures: list[tuple[str, str]] = []
    for cpp in cpps:
        verdict, summary = run_compare(stem, cpp.stem)
        verdict_counts[verdict] += 1
        if verdict != "GREEN":
            failures.append((cpp.stem, summary))
    t_compare = time.time() - t0
    print(f"  compare phase:      {verdict_counts['GREEN']:>5} GREEN  "
          f"{verdict_counts['PARTIAL']} PARTIAL  "
          f"{verdict_counts['MISMATCH']} MISMATCH  "
          f"{verdict_counts['ERROR']} ERROR  "
          f"({t_compare:.1f}s)")

    if failures and len(failures) <= 30:
        print(f"\n  failures (first 30):")
        for name, summary in failures[:30]:
            print(f"    {name}: {summary}")

    summary_path = REPO_ROOT / "build" / "logs" / f"validate_clusters_{stem}.txt"
    summary_path.parent.mkdir(parents=True, exist_ok=True)
    with summary_path.open("w") as f:
        f.write(f"binary={stem}\n")
        f.write(f"cpps={len(cpps)}\n")
        f.write(f"primaries={n_clusters}\n")
        f.write(f"siblings_cloned={n_cluster_members}\n")
        f.write(f"singletons={len(singletons)}\n")
        for k, v in verdict_counts.items():
            f.write(f"{k}={v}\n")
        f.write(f"compile_time_sec={t_compile:.1f}\n")
        f.write(f"clone_time_sec={t_clone:.1f}\n")
        f.write(f"compare_time_sec={t_compare:.1f}\n")
    print(f"\n  log: {summary_path.relative_to(REPO_ROOT)}")
    return 0 if verdict_counts["MISMATCH"] == 0 and verdict_counts["ERROR"] == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
