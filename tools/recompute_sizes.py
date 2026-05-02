#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Detect Ghidra-under-counted function sizes by scanning the orig PE
bytes past the reported size for instruction-continuation patterns
followed by INT3 padding.

Ghidra's flow analysis occasionally stops short of the actual end of
a function — typically when the function ends with a pattern its
analyser doesn't recognise as terminal (e.g. an XOR EAX,EAX/RET
appended after a logical exit, or a tail of `ADD ESP,4 / MOV EAX,ESI
/ POP ESI / RET N` that Ghidra split at the POP). The bytes are
still there in the binary; the function's "end" symbol is just 3-9 B
short.

This tool walks `config/<binary>.symbols.json`, reads the orig PE
at each function's RVA, and for any function whose reported `size`
ends RIGHT BEFORE a recognisable continuation pattern, computes the
corrected size and emits an override entry.

Heuristics (in order of confidence):
  1. The byte immediately after the reported end is `c3` (RET) or
     `c2 ?? ??` (RET imm16) — a pure missing terminal. Extend by
     1 or 3 B.
  2. The reported end is followed by a known epilogue prefix:
       `83 c4 ??` (ADD ESP, imm8 — cdecl cleanup)
       `8b c6 5e c2 ?? ??` (MOV EAX,ESI; POP ESI; RET imm16)
       `5e c3` (POP ESI; RET) / `5e c2 ?? ??` (POP ESI; RET imm16)
       `5f 5e c3` / `5f 5e c2 ?? ??` (POP EDI; POP ESI; RET[N])
       `5b 5e c3` / `5b 5e c2 ?? ??` (POP EBX; POP ESI; RET[N])
       `5d c3` / `5d c2 ?? ??` (POP EBP; RET[N])
     Extend until a `c3` or `c2 ?? ??` is consumed.
  3. After step 2, if the next bytes are `cc cc...` (INT3 padding),
     accept the new size. Otherwise revert.

Output:
  config/<binary>.size_overrides.json
    [{"rva": int, "rva_hex": "0x...", "name": "...",
      "old_size": int, "new_size": int, "reason": "..."}, ...]

`compare.py` reads this overlay if present and prefers its size over
both the YAML and `symbols.json`.

Usage:
  tools/recompute_sizes.py [binary]            # default ffxivgame
  tools/recompute_sizes.py ffxivgame --dry-run
"""

from __future__ import annotations

import argparse
import json
import struct
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CONFIG_DIR = REPO_ROOT / "config"


def _read_pe_text(orig_pe: Path) -> tuple[bytes, int, int]:
    """Return (data, text_va, text_ra) for the .text section."""
    data = orig_pe.read_bytes()
    e_lfanew = struct.unpack_from("<I", data, 0x3c)[0]
    n = struct.unpack_from("<H", data, e_lfanew + 6)[0]
    size_opt = struct.unpack_from("<H", data, e_lfanew + 20)[0]
    sec_off = e_lfanew + 24 + size_opt
    for i in range(n):
        b = sec_off + i * 40
        name = data[b : b + 8].rstrip(b"\0").decode("latin-1")
        if name == ".text":
            vaddr = struct.unpack_from("<I", data, b + 12)[0]
            raddr = struct.unpack_from("<I", data, b + 20)[0]
            return data, vaddr, raddr
    raise RuntimeError("no .text section")


# Epilogue continuation prefixes — when these appear immediately
# after the reported function end, the function probably extended
# further before Ghidra's analysis truncated it.
EPILOGUE_PREFIXES = [
    bytes([0x83, 0xc4]),   # ADD ESP, imm8
    bytes([0x8b, 0xc6]),   # MOV EAX, ESI
    bytes([0x8b, 0xc7]),   # MOV EAX, EDI
    bytes([0x8b, 0xc3]),   # MOV EAX, EBX
    bytes([0x33, 0xc0]),   # XOR EAX, EAX
    bytes([0x32, 0xc0]),   # XOR AL, AL
    bytes([0xb0, 0x01]),   # MOV AL, 1
    bytes([0xb8]),         # MOV EAX, imm32
    bytes([0x5e]),         # POP ESI
    bytes([0x5d]),         # POP EBP
    bytes([0x5f]),         # POP EDI
    bytes([0x5b]),         # POP EBX
]

TERMINATORS = {
    bytes([0xc3]): 1,                               # RET
    # RET imm16 needs to peek 2 more bytes; handled separately.
}


def _ends_at_terminator(byte: int) -> int:
    """Return length consumed if byte is a recognisable terminator,
    else 0. Caller checks for `c2 ?? ??` separately."""
    if byte == 0xc3:
        return 1
    return 0


def _is_acceptable_boundary(data: bytes, off: int, expected_next: int | None) -> bool:
    """Return True if `off` is a confident function-end boundary.

    Confidence comes from one of:
      - INT3 padding (`cc`) — the canonical alignment filler
      - `off` exactly equals the next-known-function's file offset
        (i.e., the next symbol starts here without padding — common
        for back-to-back thunks)
      - the byte at `off` looks like a typical function-start opcode
        (`8b ff` hot-patch prologue, `55` PUSH EBP, `56` PUSH ESI,
        `81 ec` SUB ESP imm, `83 ec` SUB ESP imm8, `e9`/`ff 25` JMP
        thunks)
    """
    if off >= len(data):
        return False
    if data[off] == 0xcc:
        return True
    if expected_next is not None and off == expected_next:
        return True
    # Common first-instruction patterns of x86 functions in this binary.
    b0 = data[off]
    b1 = data[off + 1] if off + 1 < len(data) else None
    if b0 == 0x55:                  # PUSH EBP (frame-pointer prologue)
        return True
    if b0 == 0x56:                  # PUSH ESI (single-callee-save)
        return True
    if b0 == 0x57:                  # PUSH EDI
        return True
    if b0 == 0x53:                  # PUSH EBX
        return True
    if b0 == 0xe9:                  # JMP rel32 (thunk start)
        return True
    if b0 == 0xff and b1 == 0x25:   # JMP [m32] (IAT thunk)
        return True
    if b0 == 0x8b and b1 == 0xff:   # MOV EDI, EDI (hot-patch prologue)
        return True
    if b0 == 0x83 and b1 in (0xec, 0xc4):   # SUB ESP, imm8 / ADD ESP, imm8
        return True
    if b0 == 0x81 and b1 in (0xec, 0xc4):   # SUB ESP, imm32 / ADD ESP, imm32
        return True
    if b0 == 0x33:                  # XOR (often XOR EAX,EAX prologue)
        return True
    if b0 == 0xb8:                  # MOV EAX, imm32
        return True
    if b0 == 0xb9:                  # MOV ECX, imm32 (singleton thunk)
        return True
    if b0 == 0x8a or b0 == 0x8b:    # MOV from memory
        return True
    if b0 == 0xc7:                  # MOV [m], imm32
        return True
    if b0 == 0x6a:                  # PUSH imm8
        return True
    if b0 == 0x68:                  # PUSH imm32
        return True
    if b0 == 0xc3:                  # RET — single-byte stub function
        return True
    if b0 == 0xc2:                  # RET imm16 — single stub
        return True
    return False


def _try_extend(data: bytes, file_off: int, old_size: int, max_extend: int = 16,
                expected_next_off: int | None = None) -> tuple[int, str] | None:
    """Try to detect a Ghidra under-count. Returns (new_size, reason)
    if a confident extension is found, else None."""
    n = len(data)
    after = file_off + old_size
    if after >= n:
        return None
    next_byte = data[after]

    # Case 1: pure missing terminal
    if next_byte == 0xc3:
        if _is_acceptable_boundary(data, after + 1, expected_next_off):
            return (old_size + 1, "RET (c3)")
        return None
    if next_byte == 0xc2 and after + 3 <= n:
        if _is_acceptable_boundary(data, after + 3, expected_next_off):
            return (old_size + 3, f"RET imm16 ({data[after + 1]:02x} {data[after + 2]:02x})")
        return None

    # Case 2: epilogue continuation — walk forward until we hit
    # a terminator (c3 or c2), then verify boundary.
    matched_prefix = None
    for prefix in EPILOGUE_PREFIXES:
        if data[after : after + len(prefix)] == prefix:
            matched_prefix = prefix
            break
    if matched_prefix is None:
        return None

    extra = 0
    while extra < max_extend and after + extra < n:
        b = data[after + extra]
        if b == 0xc3:
            extra += 1
            break
        if b == 0xc2 and after + extra + 3 <= n:
            extra += 3
            break
        extra += 1
    else:
        return None

    new_end = after + extra
    if not _is_acceptable_boundary(data, new_end, expected_next_off):
        return None

    return (old_size + extra, f"epilogue continuation ({matched_prefix.hex()} … terminator)")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", default="ffxivgame", nargs="?")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--max-extend", type=int, default=16,
                    help="max bytes to extend a function by (default 16)")
    args = ap.parse_args()
    stem = args.binary.replace(".exe", "")

    sym_path = CONFIG_DIR / f"{stem}.symbols.json"
    if not sym_path.exists():
        print(f"error: {sym_path} missing", file=sys.stderr)
        return 1
    syms = json.loads(sym_path.read_text())

    orig_pe = REPO_ROOT / "orig" / f"{stem}.exe"
    if not orig_pe.exists():
        print(f"error: {orig_pe} missing", file=sys.stderr)
        return 1
    data, text_va, text_ra = _read_pe_text(orig_pe)

    # Build sorted-by-rva list to detect "next function start" overlap.
    syms_sorted = sorted([s for s in syms if s.get("section") == ".text"], key=lambda s: s["rva"])
    next_start_by_rva: dict[int, int] = {}
    for i, s in enumerate(syms_sorted):
        if i + 1 < len(syms_sorted):
            next_start_by_rva[s["rva"]] = syms_sorted[i + 1]["rva"]

    overrides: list[dict] = []
    n_checked = 0
    for s in syms_sorted:
        rva = s["rva"]
        size = int(s.get("size", 0))
        name = s.get("name", "")
        if size == 0:
            continue
        n_checked += 1
        file_off = text_ra + (rva - text_va)
        next_start = next_start_by_rva.get(rva)
        next_start_file_off = (
            text_ra + (next_start - text_va) if next_start is not None else None
        )
        result = _try_extend(
            data, file_off, size,
            max_extend=args.max_extend,
            expected_next_off=next_start_file_off,
        )
        if result is None:
            continue
        new_size, reason = result
        # Don't extend past the next function's start.
        if next_start is not None and rva + new_size > next_start:
            continue
        overrides.append({
            "rva": rva,
            "rva_hex": f"{rva:#010x}",
            "name": name,
            "old_size": size,
            "new_size": new_size,
            "reason": reason,
        })

    out_path = CONFIG_DIR / f"{stem}.size_overrides.json"
    if not args.dry_run:
        out_path.write_text(json.dumps(overrides, indent=2) + "\n")

    print(f"binary:    {stem}")
    print(f"checked:   {n_checked:,}")
    print(f"overrides: {len(overrides):,}{' (dry-run)' if args.dry_run else ''}")
    if overrides and len(overrides) <= 10:
        print(f"\n  per-entry:")
        for o in overrides:
            print(f"    {o['rva_hex']:>10}  {o['old_size']:>4} → {o['new_size']:<4}  {o['reason']}  ({o['name']})")
    elif overrides:
        print(f"\n  first 10 of {len(overrides)}:")
        for o in overrides[:10]:
            print(f"    {o['rva_hex']:>10}  {o['old_size']:>4} → {o['new_size']:<4}  {o['reason']}  ({o['name']})")
        print(f"    … see {out_path.relative_to(REPO_ROOT)}")
    if not args.dry_run:
        print(f"\n  log: {out_path.relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
