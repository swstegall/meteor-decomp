#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Decode shipped FFXIV 1.x `.le.lpb` script files to standard Lua 5.1 bytecode.

Two wrapper formats observed across the ~2671 shipped script files in
`<install>/client/script/`:

  rlu\\x0b  — uncompressed: 8-byte header + raw Lua 5.1 bytecode.
              Only 1 file out of 2671 uses this variant (some sort of
              edge-case test fixture or unencoded build leftover).

  rle\\x0c  — XOR-obfuscated: 16-byte header + payload XOR'd with 0x73.
              The first 3 bytes of the Lua 5.1 signature (`\\x1bLu`) are
              stored at bytes 13-15 of the header (also XOR'd with 0x73 —
              decoded literal `ff 68 3f 06` → `8c 1b 4c 75`, and bytes 1..3
              of the decoded prefix are the missing Lua header bytes).
              The remaining bytecode starts at offset 16, also XOR'd
              with 0x73. So the full Lua bytecode is:
                  decoded[13:16] XOR 0x73   (= `\\x1bLu`)
                  + decoded[16:] XOR 0x73   (= rest of bytecode starting
                                              with `aQ\\x00\\x01...`)

The "rle\\x0c" name historically suggests RLE compression but the cipher
is actually just byte-wise XOR with 0x73 — there's no run-length stage.

The shipped script-tree filenames are also obfuscated via a substitution
cipher (case-folded, then per-character: a-j ↔ 9-0 by position; k-z
pair-swapped around midpoint sum=37). See `decode_filename` below.

Usage:
    tools/decode_lpb.py <install_root>             # bulk-decode all .lpb
    tools/decode_lpb.py <install_root> <subpath>   # decode one file

Output: each `<install>/client/script/.../foo.le.lpb` →
        `build/lpb/foo.luac` (raw Lua 5.1 bytecode for unluac).
"""

from __future__ import annotations

import argparse
import os
import sys
from pathlib import Path


def decode_lpb(data: bytes) -> bytes | None:
    """Decode a shipped .le.lpb file's bytes to standard Lua 5.1 bytecode."""
    if data[:4] == b"rlu\x0b":
        # Uncompressed: 8-byte header + raw Lua bytecode
        return data[8:]
    if data[:4] == b"rle\x0c":
        # XOR-0x73 obfuscation. Bytes 13-15 hold the obfuscated `\x1bLu`
        # prefix that's missing from the body at offset 16.
        prefix = bytes(b ^ 0x73 for b in data[13:16])
        body = bytes(b ^ 0x73 for b in data[16:])
        return prefix + body
    return None


def encode_filename(name: str) -> str:
    """Apply the FFXIV 1.x script-tree filename cipher.

    Case-fold the input, then map each character:
      - alpha at position 1..10 (a..j)  → digit (10 - pos)  (a→9, j→0)
      - alpha at position 11..26 (k..z) → alpha (37 - pos)  (k↔z, l↔y,
                                                              ..., r↔s)
      - digit D                          → alpha (10 - D)    (0↔j, 9↔a)
      - non-alphanum: passthrough

    The cipher is an involution (applying it twice yields the input).
    Validated against `ZoneMoveProgTest` → `kvw5xvo5usv3q5rq` (16/16) and
    `Man0g0` → `x9wj3j` (6/6).
    """
    out = []
    for c in name.lower():
        if c.isalpha():
            pos = ord(c) - ord("a") + 1  # 1..26
            if 1 <= pos <= 10:
                out.append(str(10 - pos))
            else:
                out.append(chr(ord("a") + (37 - pos) - 1))
        elif c.isdigit():
            d = int(c)
            out.append(chr(ord("a") + (10 - d) - 1))
        else:
            out.append(c)
    return "".join(out)


def decode_filename(s: str) -> str:
    """Inverse of encode_filename — same algorithm (involution)."""
    return encode_filename(s)


def find_lpb(install_root: Path, source_name: str) -> Path | None:
    """Find a shipped .le.lpb file by its decoded source name.

    Walks `<install>/client/script/` looking for a file whose basename
    (minus `.le.lpb`) equals `encode_filename(source_name)`.
    """
    enc = encode_filename(source_name)
    script_dir = install_root / "client" / "script"
    for root, _, files in os.walk(script_dir):
        for f in files:
            if f == f"{enc}.le.lpb":
                return Path(root) / f
    return None


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("install_root", type=Path,
                    help="FFXIV install root (contains client/script/)")
    ap.add_argument("source_name", nargs="?",
                    help="Original source-side name to decode (e.g. 'Man0g0' or "
                         "'OpeningDirector'). If omitted, bulk-decodes all .lpb.")
    ap.add_argument("--out", type=Path, default=Path("build/lpb"),
                    help="Output directory for decoded .luac files")
    ap.add_argument("--show-cipher", action="store_true",
                    help="Print the cipher mapping table and exit")
    args = ap.parse_args()

    if args.show_cipher:
        print("FFXIV 1.x script-tree filename cipher:")
        for c in "abcdefghijklmnopqrstuvwxyz0123456789":
            print(f"  {c} ↔ {encode_filename(c)}")
        return 0

    args.out.mkdir(parents=True, exist_ok=True)

    if args.source_name:
        # Single-file mode
        path = find_lpb(args.install_root, args.source_name)
        if path is None:
            enc = encode_filename(args.source_name)
            print(f"error: no shipped .lpb matches source '{args.source_name}' "
                  f"(ciphered: '{enc}.le.lpb')", file=sys.stderr)
            return 1
        decoded = decode_lpb(path.read_bytes())
        if decoded is None:
            print(f"error: unrecognized .lpb wrapper magic in {path}", file=sys.stderr)
            return 1
        out = args.out / f"{args.source_name}.luac"
        out.write_bytes(decoded)
        print(f"  {args.source_name}: {path}")
        print(f"  → {out} ({len(decoded)} B)")
        if decoded[:5] == b"\x1bLuaQ":
            print(f"  ✓ valid Lua 5.1 bytecode (run unluac to decompile)")
        return 0

    # Bulk mode
    script_dir = args.install_root / "client" / "script"
    n_decoded = 0
    n_failed = 0
    for root, _, files in os.walk(script_dir):
        for f in files:
            if not f.endswith(".le.lpb"):
                continue
            path = Path(root) / f
            decoded = decode_lpb(path.read_bytes())
            if decoded is None:
                n_failed += 1
                continue
            # Mirror the script-tree directory layout in the output
            rel = path.relative_to(script_dir)
            stem = rel.stem  # e.g. 'x9wj3j.le' → 'x9wj3j'
            stem = stem[:-3] if stem.endswith(".le") else stem  # strip .le infix
            out_path = args.out / rel.parent / f"{stem}.luac"
            out_path.parent.mkdir(parents=True, exist_ok=True)
            out_path.write_bytes(decoded)
            n_decoded += 1
    print(f"Decoded {n_decoded} .lpb files to {args.out}/ ({n_failed} failed)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
