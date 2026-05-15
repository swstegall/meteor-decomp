#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Compare a re-linked PE against orig and report per-section + per-byte
match statistics.

Layout:
  - Print PE header summary (sig at 0x00, NT signature at 0x3c+)
  - Per-section: compare bytes within each section's `(rptr, rsize)`
    range, report match%
  - Per-RVA spot-check: for each function row in the YAML work pool,
    locate the function's bytes at the orig RVA in BOTH binaries and
    report whether they match
  - Overall byte-match%

Usage:
  tools/diff_pe.py ffxivlogin
  tools/diff_pe.py ffxivlogin --verbose
"""

from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
ORIG = REPO_ROOT / "orig"
LINK = REPO_ROOT / "build" / "link"
PE_LAYOUT = REPO_ROOT / "build" / "pe-layout"
CONFIG = REPO_ROOT / "config"


def parse_pe(data: bytes) -> dict:
    """Return dict with image_base, entry_va, sections {name: {rptr, rsize, vaddr, vsize, chars}}."""
    pe_off = struct.unpack_from("<I", data, 0x3c)[0]
    if data[pe_off:pe_off + 4] != b"PE\0\0":
        raise ValueError("missing PE\\0\\0 magic")
    n_sections = struct.unpack_from("<H", data, pe_off + 6)[0]
    opt_size = struct.unpack_from("<H", data, pe_off + 0x14)[0]
    opt_off = pe_off + 0x18
    image_base = struct.unpack_from("<I", data, opt_off + 0x1c)[0]
    entry_rva = struct.unpack_from("<I", data, opt_off + 0x10)[0]
    sec_off = opt_off + opt_size
    sections: dict[str, dict] = {}
    for i in range(n_sections):
        b = sec_off + i * 40
        name = data[b:b + 8].rstrip(b"\0").decode("ascii", errors="replace")
        sections[name] = {
            "vsize": struct.unpack_from("<I", data, b + 8)[0],
            "vaddr": struct.unpack_from("<I", data, b + 12)[0],
            "rsize": struct.unpack_from("<I", data, b + 16)[0],
            "rptr":  struct.unpack_from("<I", data, b + 20)[0],
            "chars": struct.unpack_from("<I", data, b + 36)[0],
        }
    return {
        "image_base": image_base,
        "entry_va": image_base + entry_rva,
        "sections": sections,
    }


def diff_section(orig: bytes, ours: bytes, sec: dict) -> tuple[int, int, int]:
    """Return (matching_bytes, total_bytes, first_diff_offset_or_-1)."""
    ob = orig[sec["rptr"]:sec["rptr"] + sec["rsize"]]
    ub = ours[sec["rptr"]:sec["rptr"] + sec["rsize"]]
    sz = min(len(ob), len(ub))
    n_match = 0
    first_diff = -1
    for i in range(sz):
        if ob[i] == ub[i]:
            n_match += 1
        elif first_diff < 0:
            first_diff = i
    return n_match, sz, first_diff


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", help="binary stem (e.g. ffxivlogin)")
    ap.add_argument("--verbose", action="store_true", help="show all per-section + first-diff details")
    args = ap.parse_args()
    binary = args.binary

    orig_path = ORIG / f"{binary}.exe"
    ours_path = LINK / f"{binary}.exe"
    if not orig_path.exists():
        print(f"error: {orig_path} not found")
        return 1
    if not ours_path.exists():
        print(f"error: {ours_path} not found — run tools/link_pe.sh {binary} first")
        return 1

    orig = orig_path.read_bytes()
    ours = ours_path.read_bytes()
    o_pe = parse_pe(orig)
    u_pe = parse_pe(ours)

    print(f"=== diff_pe: {binary}.exe ===")
    print(f"  orig: {len(orig):,} B  image_base=0x{o_pe['image_base']:08x}  entry=0x{o_pe['entry_va']:08x}")
    print(f"  ours: {len(ours):,} B  image_base=0x{u_pe['image_base']:08x}  entry=0x{u_pe['entry_va']:08x}")
    print()

    # PE header diff (first 0x400 bytes — covers DOS stub + NT headers + section table).
    header_size = 0x400
    n_match = sum(1 for a, b in zip(orig[:header_size], ours[:header_size]) if a == b)
    print(f"  PE header (first 0x{header_size:x}): {n_match}/{header_size} ({100*n_match/header_size:.1f}%)")

    # Per-section diff.
    print("\n  Section comparison:")
    print(f"  {'section':10s}  {'orig vsize':>10s} {'ours vsize':>10s}  {'orig rsize':>10s} {'ours rsize':>10s}  {'match':>10s}  {'first_diff':>10s}")
    common_sections = set(o_pe["sections"]) & set(u_pe["sections"])
    only_orig = set(o_pe["sections"]) - common_sections
    only_ours = set(u_pe["sections"]) - common_sections
    overall_secm = 0
    overall_secn = 0
    for sec_name in sorted(common_sections):
        o_sec = o_pe["sections"][sec_name]
        u_sec = u_pe["sections"][sec_name]
        # Use orig section's rptr/rsize to get the comparable region;
        # ours section's bytes at the same FILE OFFSET (assuming layout
        # parity). For "are they really the same section", we compare
        # bytes that live at the same on-disk offset.
        ob = orig[o_sec["rptr"]:o_sec["rptr"] + o_sec["rsize"]]
        ub = ours[u_sec["rptr"]:u_sec["rptr"] + u_sec["rsize"]]
        sz = min(len(ob), len(ub))
        n_match = 0
        first_diff = -1
        for i in range(sz):
            if ob[i] == ub[i]:
                n_match += 1
            elif first_diff < 0:
                first_diff = i
        overall_secm += n_match
        overall_secn += sz
        match_pct = 100 * n_match / sz if sz else 0
        first_diff_str = f"0x{first_diff:x}" if first_diff >= 0 else "-"
        print(f"  {sec_name:10s}  {o_sec['vsize']:>10,} {u_sec['vsize']:>10,}  {o_sec['rsize']:>10,} {u_sec['rsize']:>10,}  {n_match:>5,}/{sz:<5,} ({match_pct:.1f}%)  {first_diff_str:>10s}")
    if only_orig:
        print(f"\n  sections only in orig: {sorted(only_orig)}")
    if only_ours:
        print(f"\n  sections only in ours: {sorted(only_ours)}")

    # Overall byte match.
    total_match = sum(1 for a, b in zip(orig, ours) if a == b)
    total_min = min(len(orig), len(ours))
    print(f"\n  Overall byte match (full file): {total_match:,}/{total_min:,} ({100*total_match/total_min:.2f}%)")
    print(f"  Section bytes match:             {overall_secm:,}/{overall_secn:,} ({100*overall_secm/overall_secn:.2f}% of section content)")

    # Verbose: spot-check first 5 functions
    if args.verbose:
        sys.path.insert(0, str(REPO_ROOT / "tools"))
        from emit_passthrough_cpp import stream_yaml_rows
        yaml_path = CONFIG / f"{binary}.yaml"
        if yaml_path.exists():
            print("\n  Per-function spot-check (first 10):")
            count = 0
            o_text = orig[o_pe["sections"][".text"]["rptr"]:o_pe["sections"][".text"]["rptr"] + o_pe["sections"][".text"]["rsize"]]
            u_text = ours[u_pe["sections"][".text"]["rptr"]:u_pe["sections"][".text"]["rptr"] + u_pe["sections"][".text"]["rsize"]]
            for r, _e, sz, sym, _st in stream_yaml_rows(yaml_path):
                if count >= 10:
                    break
                if sz is None:
                    continue
                o_off = r - o_pe["sections"][".text"]["vaddr"]
                if o_off < 0 or o_off + sz > len(o_text):
                    continue
                o_func = bytes(o_text[o_off:o_off + sz])
                # Look up in ours at same offset.
                u_at_orig = bytes(u_text[o_off:o_off + sz]) if o_off + sz <= len(u_text) else b""
                same = "MATCH" if u_at_orig == o_func else "DIFF "
                # Search ours for the function bytes (might be at a different offset).
                found = bytes(u_text).find(o_func) if o_func else -1
                offset_str = f"0x{found:08x}" if found >= 0 else "NOT FOUND"
                print(f"    {sym:30s} (rva 0x{r:08x}, sz {sz:>5}): at orig offset {same}  found at ours offset {offset_str}")
                count += 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
