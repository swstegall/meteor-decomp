#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Relocation-aware byte-shape clusterer.

`tools/cluster_shapes.py` clusters by exact bytes — useful for the
many tiny accessor / setter / stub shapes whose entire body is opcodes
+ short-byte displacements (no relocation). It misses larger shapes
where the body is otherwise structurally identical but each function
calls a *different* helper or loads a *different* global, putting each
instance in its own singleton cluster.

This tool wildcards the bytes that the linker would fill in (CALL/JMP
displacements, absolute moves, immediate-loaded addresses) and hashes
the wildcarded structural skeleton. Functions whose only difference is
which target they CALL or which global they load now cluster together,
and matching one source template unlocks all of them via
`compare.py`'s existing relocation-aware diff (which already wildcards
the same byte positions on the .obj side).

Reads:
  asm/<binary>/*.s           per-function disassembly dumps from
                             tools/import_to_ghidra.py

Writes:
  build/easy_wins/<binary>.clusters_reloc.json
    same shape as cluster_shapes.py's output but with relocation-
    aware shape hashes (and a `reloc_mask` field per cluster member
    so stamp_clusters.py / validate_clusters.py can reason about it)

Usage:
  tools/cluster_relocs.py [binary]                 # default ffxivgame
  tools/cluster_relocs.py ffxivgame --top 30
  tools/cluster_relocs.py ffxivgame --min-size 5

Reloc-byte detection (pattern-based, fast — no full disassembler):
  e8 ?? ?? ?? ??              CALL near       (reloc at +1, 4 bytes)
  e9 ?? ?? ?? ??              JMP near        (reloc at +1, 4 bytes)
  ff 15 ?? ?? ?? ??           CALL m32        (reloc at +2, 4 bytes)
  ff 25 ?? ?? ?? ??           JMP m32         (reloc at +2, 4 bytes)
  a1 ?? ?? ?? ??              MOV EAX,[moff32](reloc at +1, 4 bytes)
  a3 ?? ?? ?? ??              MOV [moff32],EAX(reloc at +1, 4 bytes)
  b8..bf ?? ?? ?? ??          MOV r32,imm32   (reloc at +1, 4 bytes
                                               IFF imm32 looks address-y)
  68 ?? ?? ?? ??              PUSH imm32      (reloc at +1, 4 bytes
                                               IFF imm32 looks address-y)
  8b ??d ?? ?? ?? ??          MOV r32,[moff32](reloc at +2, 4 bytes
                                               for ModRM mod=00 r/m=101
                                               i.e. second byte ends in
                                               0b101 with mod=00 — bytes
                                               05/0d/15/1d/25/2d/35/3d)
  89 ??d ?? ?? ?? ??          MOV [moff32],r32(reloc at +2, 4 bytes
                                               same ModRM constraint)
  c7 ??d ?? ?? ?? ?? ?? ?? ?? ?? MOV [moff32],imm32 (10 bytes; reloc
                                               at +2 covers the moff32;
                                               trailing imm32 may also
                                               be a reloc if looks
                                               address-y)

"Looks address-y" heuristic: little-endian uint32 ≥ image_base
(0x00400000 by default). Tunable via --image-base; functions that load
small constants below that threshold are NOT wildcarded.
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
RE_INSTR_LINE = re.compile(
    r"^\s*[0-9a-fA-F]+:\s+((?:[0-9a-fA-F][0-9a-fA-F]\s+)+)\s",
    re.MULTILINE,
)


def _looks_address_like(b1: int, b2: int, b3: int, b4: int, image_base: int) -> bool:
    """Treat a little-endian uint32 as address-like if >= image_base."""
    val = b1 | (b2 << 8) | (b3 << 16) | (b4 << 24)
    return val >= image_base


def reloc_mask_for_body(body: bytes, image_base: int = 0x00400000) -> bytearray:
    """Return a 1-byte-per-position mask: 1 = relocation byte (wildcarded
    in structural hash), 0 = real opcode/displacement byte.

    Walks the byte stream linearly. For each opcode-prefix recognized,
    advances by the instruction's full encoded length and marks the
    reloc bytes within. Unknown opcodes advance by 1 byte (over-
    aggressive for pattern-mismatch but conservative for clustering —
    we under-mask rather than over-mask)."""
    mask = bytearray(len(body))
    i = 0
    n = len(body)
    while i < n:
        b = body[i]
        # 83 modrm imm8 / 81 modrm imm32 / 80 modrm imm8 — ALU with
        # immediate. The ModR/M byte can have values that match other
        # opcodes (e.g. 0x83 0xe9 NN is `SUB ECX, imm8` but byte 1 = 0xe9
        # would otherwise look like a JMP rel32 opcode). Recognise the
        # full 3- or 6-byte form here so the linear walker doesn't get
        # fooled.
        if b == 0x83 and i + 3 <= n:
            i += 3   # opcode + modrm + imm8 (no reloc — pure structural)
            continue
        if b == 0x81 and i + 6 <= n:
            # imm32 form may be address-y → wildcard if so.
            modrm = body[i + 1]
            mod = (modrm >> 6) & 0b11
            rm = modrm & 0b111
            if mod == 0 and rm == 0b101:
                # MOD=00, R/M=101 → disp32 absolute address ALU. reloc on +2.
                for j in range(i + 2, i + 6):
                    mask[j] = 1
                if i + 10 <= n and _looks_address_like(
                    body[i + 6], body[i + 7], body[i + 8], body[i + 9], image_base
                ):
                    for j in range(i + 6, i + 10):
                        mask[j] = 1
                    i += 10
                    continue
                i += 10
                continue
            # Plain register form: 1 (op) + 1 (modrm) + 4 (imm32) = 6 B.
            i += 6
            continue
        if b == 0x80 and i + 3 <= n:
            i += 3
            continue
        # E8 — CALL rel32 ; E9 — JMP rel32
        if b in (0xE8, 0xE9) and i + 5 <= n:
            for j in range(i + 1, i + 5):
                mask[j] = 1
            i += 5
            continue
        # FF 15 — CALL m32 ; FF 25 — JMP m32
        if b == 0xFF and i + 6 <= n and body[i + 1] in (0x15, 0x25):
            for j in range(i + 2, i + 6):
                mask[j] = 1
            i += 6
            continue
        # A1 — MOV EAX,[moff32] ; A3 — MOV [moff32],EAX
        if b in (0xA1, 0xA3) and i + 5 <= n:
            for j in range(i + 1, i + 5):
                mask[j] = 1
            i += 5
            continue
        # B8..BF — MOV r32, imm32 ; 68 — PUSH imm32 (only when address-y)
        if (0xB8 <= b <= 0xBF or b == 0x68) and i + 5 <= n:
            if _looks_address_like(body[i + 1], body[i + 2], body[i + 3], body[i + 4], image_base):
                for j in range(i + 1, i + 5):
                    mask[j] = 1
            i += 5
            continue
        # 8B / 89 — MOV r32,m / MOV m,r32 ; with ModRM mod=00 r/m=101
        # (absolute m32 form). Second byte mask: top 2 bits = 00, low 3
        # bits = 101 → values 05, 0D, 15, 1D, 25, 2D, 35, 3D.
        if b in (0x8B, 0x89) and i + 6 <= n:
            modrm = body[i + 1]
            mod = (modrm >> 6) & 0b11
            rm = modrm & 0b111
            if mod == 0 and rm == 0b101:
                for j in range(i + 2, i + 6):
                    mask[j] = 1
                i += 6
                continue
        # C7 — MOV r/m32, imm32 ; with ModRM mod=00 r/m=101 covers
        # `MOV [moff32], imm32` (10-byte form). reloc at +2 (the m32
        # operand); the trailing imm32 may also be address-y.
        if b == 0xC7 and i + 10 <= n:
            modrm = body[i + 1]
            mod = (modrm >> 6) & 0b11
            rm = modrm & 0b111
            if mod == 0 and rm == 0b101:
                for j in range(i + 2, i + 6):
                    mask[j] = 1
                # Check trailing imm32 for address-likeness
                if _looks_address_like(body[i + 6], body[i + 7], body[i + 8], body[i + 9], image_base):
                    for j in range(i + 6, i + 10):
                        mask[j] = 1
                i += 10
                continue
        # Unknown opcode — advance by 1, leave mask alone.
        i += 1
    return mask


def structural_hash(body: bytes, mask: bytearray) -> str:
    """SHA-1 of the body with reloc-masked bytes replaced by 0x00."""
    structural = bytearray(body)
    for j in range(len(mask)):
        if mask[j]:
            structural[j] = 0
    return hashlib.sha1(bytes(structural)).hexdigest()[:12]


def parse_function(asm_path: Path) -> dict | None:
    text = asm_path.read_text(errors="replace")
    rva_m = RE_HEADER_RVA.search(text)
    size_m = RE_HEADER_SIZE.search(text)
    name_m = RE_HEADER_NAME.search(text)
    if not (rva_m and size_m and name_m):
        return None
    rva = int(rva_m.group(1), 16)
    size = int(size_m.group(1), 16)
    name = name_m.group(1).strip()

    body = bytearray()
    for m in RE_INSTR_LINE.finditer(text):
        for tok in m.group(1).split():
            body.append(int(tok, 16))
    if len(body) != size:
        return None
    return {
        "rva": rva,
        "rva_hex": f"{rva:#010x}",
        "size": size,
        "name": name,
        "asm": asm_path.name,
        "body_bytes": bytes(body),
    }


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", default="ffxivgame", nargs="?")
    ap.add_argument("--top", type=int, default=30, help="how many top clusters to show in the report (default 30)")
    ap.add_argument("--min-size", type=int, default=2, help="ignore clusters with fewer members (default 2)")
    ap.add_argument("--max-fn-size", type=int, default=200, help="ignore functions larger than this (default 200)")
    ap.add_argument("--image-base", type=lambda s: int(s, 0), default=0x00400000,
                    help="image base for the address-likeness heuristic (default 0x00400000)")
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
        body = fn["body_bytes"]
        mask = reloc_mask_for_body(body, image_base=args.image_base)
        h = structural_hash(body, mask)
        record = {
            "rva": fn["rva"],
            "rva_hex": fn["rva_hex"],
            "size": fn["size"],
            "name": fn["name"],
            "asm": fn["asm"],
            "reloc_bytes": sum(mask),
        }
        clusters.setdefault(h, []).append(record)
        n_processed += 1

    multi_clusters = {h: members for h, members in clusters.items() if len(members) >= args.min_size}
    cluster_list = sorted(
        multi_clusters.items(),
        key=lambda kv: (-len(kv[1]), kv[1][0]["size"]),
    )

    json_path = OUT_ROOT / f"{stem}.clusters_reloc.json"
    json_payload = {h: members for h, members in cluster_list}
    json_path.write_text(json.dumps(json_payload, indent=2) + "\n")

    report_path = OUT_ROOT / f"{stem}.clusters_reloc.report.md"
    n_total_members = sum(len(m) for m in multi_clusters.values())
    n_clusters = len(multi_clusters)
    with report_path.open("w") as f:
        f.write(f"# {stem}.exe — relocation-aware shape clusters\n\n")
        f.write(f"Auto-generated by `tools/cluster_relocs.py`. Companion to "
                f"`tools/cluster_shapes.py` (exact-byte clusters). Wildcards "
                f"reloc-bearing bytes (CALL/JMP displacements, absolute "
                f"moves, address-like immediates) so functions whose only "
                f"difference is the linker's fixup target cluster together.\n\n")
        f.write(f"## Summary\n\n")
        f.write(f"- functions analysed: **{n_processed:,}** (skipped {n_skipped} "
                f"on parse error or size > {args.max_fn_size} B)\n")
        f.write(f"- clusters with ≥ {args.min_size} members: **{n_clusters:,}**\n")
        f.write(f"- functions covered by multi-member clusters: **{n_total_members:,}** "
                f"({100*n_total_members/max(n_processed,1):.1f}% of analysed)\n")
        f.write(f"- top {args.top} clusters by size shown below; full JSON at "
                f"`{json_path.relative_to(REPO_ROOT)}`\n\n")
        f.write(f"### Top {args.top} clusters\n\n")
        f.write("| rank | shape | members | size | reloc B | sample bytes | sample name |\n")
        f.write("|---:|---|---:|---:|---:|---|---|\n")
        for i, (h, members) in enumerate(cluster_list[: args.top], 1):
            sample = members[0]
            asm_text = (asm_dir / sample["asm"]).read_text(errors="replace")
            body = bytearray()
            for m in RE_INSTR_LINE.finditer(asm_text):
                for tok in m.group(1).split():
                    body.append(int(tok, 16))
            byte_str = " ".join(f"{b:02x}" for b in body[:16])
            if len(body) > 16:
                byte_str += " …"
            f.write(f"| {i} | `{h}` | {len(members)} | {sample['size']} | "
                    f"{sample['reloc_bytes']} | `{byte_str}` | "
                    f"`{sample['name']}` |\n")

    print(f"wrote: {json_path.relative_to(REPO_ROOT)}  ({n_clusters} clusters, {n_total_members} member functions)")
    print(f"wrote: {report_path.relative_to(REPO_ROOT)}")
    print(f"  analysed: {n_processed:,}  skipped: {n_skipped}  cluster coverage: "
          f"{100*n_total_members/max(n_processed,1):.1f}%")
    return 0


if __name__ == "__main__":
    sys.exit(main())
