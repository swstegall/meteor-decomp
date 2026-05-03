#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Sqpack-cat — open a DAT file by resource_id and dump its contents.
#
# Implements steps 1–3 of the Phase 4 exit criterion in
# `docs/sqpack.md`:
#   1. ✅ Resolve resource_id → DAT path  (via tools/sqpack_path.py)
#   2. ✅ Open the DAT file               (this tool)
#   3. ✅ Walk the chunked PackRead format if applicable, else
#         dump raw bytes / detect known magics
#   4. 🔲 Decompression layer             (TBD — locate via zlib magic
#                                          in a future pass)
#
# Chunk format (recovered from ChunkReadUInt::ReadNextChunkHeader at
# RVA 0x004ebd40 + PackRead::ProcessChunk at RVA 0x00942740):
#
#   struct ChunkHeader {
#       u32 unknown_0;            // bytes 0..4 (not read by ReadNext)
#       u32 chunk_size;           // bytes 4..8 (optionally byte-swapped
#                                 //              if PackRead.m_flag15 = 1)
#   };                            // payload follows: chunk_size bytes
#
# Many DAT files do NOT use this format — they're file-type-specific
# binary blobs with their own magic ("GTEX" texture, "SEDB" sound DB,
# "MapL" map layout, etc.). The chunk walker only runs when the file
# starts with a plausible chunk header (chunk_size + 8 fits in the
# file size).

import argparse
import os
import struct
import sys

# Import the path resolver from the sibling tool.
_HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, _HERE)
from sqpack_path import build_path  # noqa: E402


# Known file-type magics — surfaced empirically from a real install
# scan. These are NOT chunked PackRead format; they have their own
# readers in the binary.
KNOWN_MAGICS = {
    b"GTEX": "Texture (DDS-like)",
    b"SEDB": "Sound DB (followed by RES tag)",
    b"MapL": "MapLayoutResource",
    b"PWIB": "PWIB (unknown — possibly procedural-world index buffer)",
    b"\x23fil": "CSV-like text (#fileSet,...)",
}


def detect_magic(head: bytes) -> str | None:
    """Match the first 4 bytes against the known-magic table."""
    return KNOWN_MAGICS.get(head[:4])


def looks_like_chunked(data: bytes) -> bool:
    """Heuristic: first chunk header parses + chunk_size fits in file."""
    if len(data) < 8:
        return False
    chunk_size = struct.unpack_from("<I", data, 4)[0]
    return 8 + chunk_size <= len(data) and chunk_size > 0


def walk_chunks(data: bytes, byteswap: bool = False, limit: int = 32):
    """Iterate (chunk_index, offset, header_u32, chunk_size) tuples
    using PackRead's chunk format. Stops at end-of-file or limit."""
    cursor = 0
    idx = 0
    while cursor + 8 <= len(data):
        hdr = struct.unpack_from("<I", data, cursor)[0]
        size_raw = struct.unpack_from("<I", data, cursor + 4)[0]
        if byteswap:
            size = struct.unpack("<I", struct.pack(">I", size_raw))[0]
        else:
            size = size_raw
        next_cursor = cursor + 8 + size
        if next_cursor > len(data):
            yield (idx, cursor, hdr, size, "OVERFLOW")
            return
        yield (idx, cursor, hdr, size, "ok")
        cursor = next_cursor
        idx += 1
        if idx >= limit:
            yield (None, cursor, None, None, f"...truncated at {limit}")
            return


def hexdump(data: bytes, max_bytes: int = 256) -> None:
    """Standard hex+ascii dump, up to max_bytes."""
    n = min(len(data), max_bytes)
    for i in range(0, n, 16):
        chunk = data[i:i+16]
        hex_part = " ".join(f"{b:02x}" for b in chunk).ljust(48)
        ascii_part = "".join(chr(b) if 32 <= b < 127 else "." for b in chunk)
        print(f"  {i:08x}  {hex_part}  {ascii_part}")
    if len(data) > max_bytes:
        print(f"  ... ({len(data) - max_bytes} more bytes)")


def main() -> int:
    ap = argparse.ArgumentParser(
        description="Sqpack-cat: open a DAT file by resource_id and dump contents.",
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("resource_id",
                    help="resource_id (hex 0x... or decimal)")
    ap.add_argument("--root", required=True,
                    help="game-root path (e.g. .../FINAL FANTASY XIV)")
    ap.add_argument("--raw", action="store_true",
                    help="dump file contents to stdout as raw bytes")
    ap.add_argument("--hexdump-bytes", type=int, default=256,
                    help="how many bytes to hexdump (default 256)")
    ap.add_argument("--chunks", action="store_true",
                    help="force chunk-walk even if heuristic says not chunked")
    ap.add_argument("--byteswap", action="store_true",
                    help="byte-swap chunk_size (matches PackRead.m_flag15=1)")
    args = ap.parse_args()

    rid = int(args.resource_id, 0)
    rel = build_path(rid, posix=True).lstrip("/")
    full = os.path.join(args.root, rel)

    if not os.path.exists(full):
        print(f"error: file not found: {full}", file=sys.stderr)
        # Helpful hint: scan for typo'd nearby IDs
        return 2

    size = os.path.getsize(full)
    with open(full, "rb") as fp:
        data = fp.read()

    if args.raw:
        sys.stdout.buffer.write(data)
        return 0

    print(f"Resource:   0x{rid:08x}")
    print(f"Path:       {full}")
    print(f"Size:       {size} bytes")

    magic = detect_magic(data)
    if magic:
        print(f"Magic:      {data[:4]!r} → {magic}")

    chunked = args.chunks or (magic is None and looks_like_chunked(data))
    if chunked:
        print(f"Chunks:     (PackRead format, byteswap={args.byteswap})")
        print(f"  {'idx':>4}  {'offset':>10}  {'hdr (u32)':>12}  {'size':>10}  status")
        for idx, off, hdr, sz, status in walk_chunks(
                data, byteswap=args.byteswap):
            if idx is None:
                print(f"  ----  {off:>10}  {'':>12}  {'':>10}  {status}")
            else:
                print(f"  {idx:>4}  {off:>10}  0x{hdr:08x}  {sz:>10}  {status}")
    else:
        print(f"Chunks:     n/a (file does not look chunk-formatted)")

    print()
    print(f"First {min(args.hexdump_bytes, len(data))} bytes:")
    hexdump(data, args.hexdump_bytes)
    return 0


if __name__ == "__main__":
    sys.exit(main())
