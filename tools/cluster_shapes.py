#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Group functions by byte-identical body — when a single source idiom
compiles to the same machine code in N classes (e.g. an `inline T
get_field()` template instantiation across N RTTI types), every
copy lands at a distinct RVA but their bytes are identical.

Once those byte-identical sets ("shape clusters") are identified,
matching ONE representative function with a hand-written .cpp
unlocks all its cluster siblings — same source idiom compiles to
the same .obj `.text` bytes, and `compare.py` matches that .obj
against orig at every sibling's RVA. The companion tool
`tools/stamp_clusters.py` automates the per-sibling .cpp stamping.

This first-pass clusterer uses **exact byte hashing** — no
relocation wildcarding. That's enough for the ≤30 byte accessor /
setter shapes that dominate the easy-wins queue (no CALLs, no
absolute addresses, no relocations to worry about). A second-pass
"structural" clusterer that wildcards reloc bytes would coarsen
the grouping further, but exact-hash already exposes the bulk of
the multipliers (a single 8-byte bit-flag-getter shape has ~80+
exact-byte siblings in ffxivgame.exe).

Reads:
  asm/<binary>/*.s        per-function disassembly dumps from
                          tools/import_to_ghidra.py

Writes:
  build/easy_wins/<binary>.clusters.json
    { "<sha1-12>": [{"rva": int, "rva_hex": "0x...", "name": "...",
                      "size": int, "asm": "<filename>"}, ...], ... }
  build/easy_wins/<binary>.clusters.report.md
    Top-N clusters by size with sample bytes + member counts.

Usage:
  tools/cluster_shapes.py [binary]            # default ffxivgame
  tools/cluster_shapes.py ffxivgame --top 30
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
ASM_ROOT = REPO_ROOT / "asm"
OUT_ROOT = REPO_ROOT / "build" / "easy_wins"

RE_HEADER_RVA = re.compile(r"^# rva\s+0x([0-9a-fA-F]+)", re.MULTILINE)
RE_HEADER_SIZE = re.compile(r"^# size\s+0x([0-9a-fA-F]+)", re.MULTILINE)
RE_HEADER_NAME = re.compile(r"^# function\s+(\S+)", re.MULTILINE)
# Match a per-instruction line: "    rva:  hex hex hex  ASM…"
RE_INSTR_LINE = re.compile(
    r"^\s*[0-9a-fA-F]+:\s+((?:[0-9a-fA-F][0-9a-fA-F]\s+)+)\s",
    re.MULTILINE,
)


def parse_function(asm_path: Path) -> dict | None:
    """Return {rva, size, name, asm, body_bytes} or None on parse error."""
    text = asm_path.read_text(errors="replace")
    rva_m = RE_HEADER_RVA.search(text)
    size_m = RE_HEADER_SIZE.search(text)
    name_m = RE_HEADER_NAME.search(text)
    if not (rva_m and size_m and name_m):
        return None
    rva = int(rva_m.group(1), 16)
    size = int(size_m.group(1), 16)
    name = name_m.group(1).strip()

    # Concatenate the byte columns from each instruction line.
    body = bytearray()
    for m in RE_INSTR_LINE.finditer(text):
        for tok in m.group(1).split():
            body.append(int(tok, 16))
    if len(body) != size:
        # Mismatch typically means a malformed line in the dump.
        # Skip — better to leave out than to mis-cluster.
        return None
    return {
        "rva": rva,
        "rva_hex": f"{rva:#010x}",
        "size": size,
        "name": name,
        "asm": asm_path.name,
        "body_bytes": bytes(body),
    }


def shape_hash(body: bytes) -> str:
    return hashlib.sha1(body).hexdigest()[:12]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", default="ffxivgame", nargs="?")
    ap.add_argument("--top", type=int, default=30, help="how many top clusters to show in the report (default 30)")
    ap.add_argument("--min-size", type=int, default=2, help="ignore clusters smaller than this (default 2)")
    ap.add_argument("--max-fn-size", type=int, default=200, help="ignore functions larger than this (default 200)")
    args = ap.parse_args()
    stem = args.binary.replace(".exe", "")

    asm_dir = ASM_ROOT / stem
    if not asm_dir.is_dir():
        print(f"error: {asm_dir} not found — run `make split BINARY={stem}.exe` first", file=sys.stderr)
        return 1

    OUT_ROOT.mkdir(parents=True, exist_ok=True)

    clusters: dict[str, list[dict]] = {}
    n_processed = 0
    n_skipped = 0
    for asm_path in sorted(asm_dir.glob("*.s")):
        fn = parse_function(asm_path)
        if fn is None:
            n_skipped += 1
            continue
        if fn["size"] > args.max_fn_size:
            continue
        h = shape_hash(fn["body_bytes"])
        record = {k: v for k, v in fn.items() if k != "body_bytes"}
        clusters.setdefault(h, []).append(record)
        n_processed += 1

    # Filter out singletons.
    multi_clusters = {h: members for h, members in clusters.items() if len(members) >= args.min_size}
    # Sort by cluster size (descending), then by member size (so smallest first)
    cluster_list = sorted(
        multi_clusters.items(),
        key=lambda kv: (-len(kv[1]), kv[1][0]["size"]),
    )

    # Persist JSON (full, not truncated).
    json_path = OUT_ROOT / f"{stem}.clusters.json"
    json_payload = {h: members for h, members in cluster_list}
    json_path.write_text(json.dumps(json_payload, indent=2) + "\n")

    # Build the report.
    report_path = OUT_ROOT / f"{stem}.clusters.report.md"
    n_total_members = sum(len(m) for m in multi_clusters.values())
    n_clusters = len(multi_clusters)
    with report_path.open("w") as f:
        f.write(f"# {stem}.exe — byte-identical shape clusters\n\n")
        f.write(f"Auto-generated by `tools/cluster_shapes.py`.\n\n")
        f.write(f"## Summary\n\n")
        f.write(f"- functions analysed: **{n_processed:,}** (skipped {n_skipped} on parse error or size > {args.max_fn_size} B)\n")
        f.write(f"- clusters with ≥ {args.min_size} members: **{n_clusters:,}**\n")
        f.write(f"- functions covered by multi-member clusters: **{n_total_members:,}** "
                f"({100*n_total_members/max(n_processed,1):.1f}% of analysed)\n")
        f.write(f"- top {args.top} clusters by size shown below; full JSON at `{json_path.relative_to(REPO_ROOT)}`\n\n")
        f.write(f"### Top {args.top} clusters\n\n")
        f.write(f"Match one representative source file → unlock the rest with "
                f"`tools/stamp_clusters.py`.\n\n")
        f.write(f"| rank | shape | members | size | sample bytes | sample name |\n")
        f.write(f"|---:|---|---:|---:|---|---|\n")
        for i, (h, members) in enumerate(cluster_list[: args.top], 1):
            sample = members[0]
            # Reconstruct the sample bytes from the asm file (we discarded
            # body_bytes for json-serializability).
            asm_text = (asm_dir / sample["asm"]).read_text(errors="replace")
            body = bytearray()
            for m in RE_INSTR_LINE.finditer(asm_text):
                for tok in m.group(1).split():
                    body.append(int(tok, 16))
            byte_str = " ".join(f"{b:02x}" for b in body[:16])
            if len(body) > 16:
                byte_str += " …"
            f.write(f"| {i} | `{h}` | {len(members)} | {sample['size']} | `{byte_str}` | "
                    f"`{sample['name']}` |\n")

    print(f"wrote: {json_path.relative_to(REPO_ROOT)}  ({n_clusters} clusters, {n_total_members} member functions)")
    print(f"wrote: {report_path.relative_to(REPO_ROOT)}")
    print(f"  analysed: {n_processed:,}  skipped: {n_skipped}  cluster coverage: "
          f"{100*n_total_members/max(n_processed,1):.1f}%")
    return 0


if __name__ == "__main__":
    sys.exit(main())
