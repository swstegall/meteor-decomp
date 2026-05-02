#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Cross-binary template seeding.

The matching recipe captures source patterns for a small set of
canonical idioms (return 0, return *this, empty stubs, accessor /
setter shapes). Each idiom compiles to the same byte-shape across
binaries — `void C::empty() {}` produces `c3` whether it lives in
ffxivgame.exe or ffxivboot.exe. The hand-written templates under
`src/ffxivgame/_rosetta/` therefore unlock the same clusters in
every other binary that ships the same idiom.

This tool:
  1. Walks `src/ffxivgame/_rosetta/FUN_*.cpp` for primary (non-stamped)
     templates — files NOT starting with `// [STAMPED]`.
  2. Computes each primary template's cluster shape hash by looking
     up its RVA in `build/easy_wins/ffxivgame.clusters.json`.
  3. For each target binary, opens its `clusters.json` and finds
     clusters with matching shape hashes.
  4. Picks a representative function (smallest RVA) from each
     matching target cluster and stamps a seed `.cpp` at
     `src/<target>/_rosetta/FUN_<rva>.cpp` using the template's
     content, with FUN_<source-va>/0x<source-va> rewritten to the
     target representative's RVA in comments.

After seeding, run `tools/stamp_clusters.py <target>` to expand the
seed templates into per-sibling .cpp files, then
`make rosetta-bulk BINARY=<target>.exe` to validate.

Usage:
  tools/seed_templates.py ffxivboot         # seed one target
  tools/seed_templates.py --all             # seed all non-source binaries
  tools/seed_templates.py --dry-run ffxivboot
  tools/seed_templates.py --source ffxivlogin --target ffxivboot
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC_DIR = REPO_ROOT / "src"
EASY_WINS = REPO_ROOT / "build" / "easy_wins"

KNOWN_BINARIES = ("ffxivgame", "ffxivlogin", "ffxivboot", "ffxivconfig", "ffxivupdater")


def _is_primary(cpp_text: str) -> bool:
    """Return True iff this .cpp is hand-written (not [STAMPED] from another)."""
    return not cpp_text.startswith("// [STAMPED]")


def _va_to_rva(va: int, image_base: int = 0x400000) -> int:
    return va - image_base


def _rva_to_va(rva: int, image_base: int = 0x400000) -> int:
    return rva + image_base


def _load_clusters(binary_stem: str) -> dict[str, list[dict]]:
    path = EASY_WINS / f"{binary_stem}.clusters.json"
    if not path.exists():
        return {}
    return json.loads(path.read_text())


def _build_rva_to_hash(clusters: dict[str, list[dict]]) -> dict[int, str]:
    """For a given clusters JSON, return rva → shape_hash mapping."""
    out: dict[int, str] = {}
    for h, members in clusters.items():
        for m in members:
            out[m["rva"]] = h
    return out


def _stamp_seed(template_text: str, source_va: int, target_va: int, target_rva: int) -> str:
    """Generate seed-template content. Same shape as stamp_clusters.py's
    output but with a `[SEED]` marker so re-runs don't recurse."""
    import re
    source_va_hex = f"{source_va:08x}"
    target_va_hex = f"{target_va:08x}"
    # Rewrite FUN_<source>/0x<source> for the target's VA. Use 8-char hex
    # form only — short field-offset hex strings stay untouched.
    text = re.sub(rf"\bFUN_{source_va_hex}\b", f"FUN_{target_va_hex}", template_text, flags=re.IGNORECASE)
    text = re.sub(rf"\b0x{source_va_hex}\b", f"0x{target_va_hex}", text, flags=re.IGNORECASE)
    header = (
        f"// [SEED] from FUN_{source_va_hex}.cpp by tools/seed_templates.py\n"
        f"//        target VA {target_va:#010x} (RVA {target_rva:#010x})\n"
        f"//        cross-binary cluster match — same shape hash, same C++ idiom.\n"
        f"//        After seeding, run tools/stamp_clusters.py to fan out to siblings.\n"
    )
    return header + text


def seed_one(source_stem: str, target_stem: str, dry_run: bool = False) -> dict:
    """Seed `src/<target>/_rosetta/` from `src/<source>/_rosetta/` primary
    templates whose cluster shapes also exist in the target binary."""
    source_rosetta = SRC_DIR / source_stem / "_rosetta"
    if not source_rosetta.is_dir():
        return {"error": f"source rosetta dir missing: {source_rosetta}"}

    source_clusters = _load_clusters(source_stem)
    target_clusters = _load_clusters(target_stem)
    if not source_clusters:
        return {"error": f"no clusters for source {source_stem} — run tools/cluster_shapes.py {source_stem}"}
    if not target_clusters:
        return {"error": f"no clusters for target {target_stem} — run tools/cluster_shapes.py {target_stem}"}

    source_rva_to_hash = _build_rva_to_hash(source_clusters)

    target_rosetta = SRC_DIR / target_stem / "_rosetta"
    if not dry_run:
        target_rosetta.mkdir(parents=True, exist_ok=True)

    seeded: list[dict] = []
    skipped_no_match: list[str] = []
    skipped_already: list[str] = []
    primary_count = 0

    for cpp_path in sorted(source_rosetta.glob("FUN_*.cpp")):
        text = cpp_path.read_text(errors="replace")
        if not _is_primary(text):
            continue
        primary_count += 1
        # Parse VA from filename.
        try:
            source_va = int(cpp_path.stem.removeprefix("FUN_"), 16)
        except ValueError:
            continue
        source_rva = _va_to_rva(source_va)
        shape_hash = source_rva_to_hash.get(source_rva)
        if shape_hash is None:
            skipped_no_match.append(f"{cpp_path.name}: no cluster in source — singleton or out-of-band")
            continue
        target_members = target_clusters.get(shape_hash)
        if not target_members:
            skipped_no_match.append(f"{cpp_path.name}: shape {shape_hash} absent from {target_stem}")
            continue

        # Pick representative: smallest RVA in the target cluster.
        rep = min(target_members, key=lambda m: m["rva"])
        target_rva = int(rep["rva"])
        target_va = _rva_to_va(target_rva)
        target_path = target_rosetta / f"FUN_{target_va:08x}.cpp"
        if target_path.exists():
            skipped_already.append(f"{target_path.name}: already exists in {target_stem}")
            continue

        if not dry_run:
            content = _stamp_seed(text, source_va, target_va, target_rva)
            target_path.write_text(content)
        seeded.append({
            "shape": shape_hash,
            "source_template": cpp_path.name,
            "target_path": target_path.name,
            "target_cluster_size": len(target_members),
        })

    return {
        "source": source_stem,
        "target": target_stem,
        "primary_templates_seen": primary_count,
        "seeded": len(seeded),
        "skipped_no_cluster_match": len(skipped_no_match),
        "skipped_already_exists": len(skipped_already),
        "details": seeded,
        "no_match_log": skipped_no_match,
        "already_exists_log": skipped_already,
    }


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("target", nargs="?", help="target binary stem (default: --all required)")
    ap.add_argument("--source", default="ffxivgame", help="source binary whose templates we seed FROM (default: ffxivgame)")
    ap.add_argument("--all", action="store_true", help="seed all known non-source binaries")
    ap.add_argument("--dry-run", action="store_true", help="show what would happen without writing")
    args = ap.parse_args()

    if args.all and not args.target:
        targets = [b for b in KNOWN_BINARIES if b != args.source]
    elif args.target and not args.all:
        targets = [args.target.replace(".exe", "")]
    else:
        print("usage: tools/seed_templates.py <target>  |  tools/seed_templates.py --all", file=sys.stderr)
        return 1

    grand_seeded = 0
    grand_skipped_no = 0
    grand_skipped_existed = 0
    for t in targets:
        report = seed_one(args.source, t, dry_run=args.dry_run)
        if "error" in report:
            print(f"=== {args.source} → {t} ===  ERROR: {report['error']}")
            continue
        print(f"=== {args.source} → {t} ===")
        print(f"  primary templates: {report['primary_templates_seen']}")
        print(f"  seeded:            {report['seeded']}{' (dry-run)' if args.dry_run else ''}")
        print(f"  no cluster match:  {report['skipped_no_cluster_match']}")
        print(f"  already existed:   {report['skipped_already_exists']}")
        if report["seeded"] and report["seeded"] <= 30:
            for s in report["details"]:
                print(f"    [seed] {s['target_path']:30}  ← {s['source_template']:30}  cluster size {s['target_cluster_size']}")
        grand_seeded += report["seeded"]
        grand_skipped_no += report["skipped_no_cluster_match"]
        grand_skipped_existed += report["skipped_already_exists"]

    if len(targets) > 1:
        print(f"\ntotal seeded: {grand_seeded}{' (dry-run)' if args.dry_run else ''}  "
              f"no-match: {grand_skipped_no}  existed: {grand_skipped_existed}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
