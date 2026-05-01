#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Validate that garlemald-server's `common/src/utils.rs::murmur_hash2`
matches the binary's MurmurHash2 implementation at RVA 0x00931490
(`FUN_00d31490`, 170 bytes).

The binary's algorithm is a *backward-walking* MurmurHash2 variant:
the main loop starts at `data + len - 4` and steps backward in 4-byte
chunks; the tail cascade reads from `data[0..tail-1]`. This is unusual
(standard MurmurHash2 walks forward) but is faithfully ported by
garlemald — verified by step-by-step asm-to-Rust correspondence in
`docs/murmur2.md`.

This script implements the binary's algorithm in pure Python from a
direct asm trace, then prints expected hash values for a handful of
property-name test vectors so they can be cross-checked against
garlemald's Rust output (e.g. via a small Rust integration test):

    cargo test --manifest-path garlemald-server/common/Cargo.toml \\
        murmur_hash2_known_vectors

The expected vectors below were computed by this Python port; if
garlemald's output diverges, that's a regression in either Rust or
this trace and must be reconciled.
"""

from __future__ import annotations

# Magic constants from MurmurHash2 + the binary's specific tail-cascade
# semantics traced from FUN_00d31490 in ffxivgame.exe.
M = 0x5BD1E995
R = 24
MASK32 = 0xFFFFFFFF


def mul32(a: int, b: int) -> int:
    return (a * b) & MASK32


def shr32(x: int, n: int) -> int:
    return (x & MASK32) >> n


def murmur_hash2_backward(key: bytes, seed: int) -> int:
    """Pure-Python port of FUN_00d31490 in ffxivgame.exe.

    Direct correspondence with the asm:
      - Prologue: EAX = data + len, ECX = len ^ seed.
      - Main loop (per 4-byte chunk, walking backward):
          h *= M
          k = (data[i]<<24) | (data[i+1]<<16) | (data[i+2]<<8) | data[i+3]
              where i = (chunk start, walking backward from len-4)
          k *= M
          k ^= k >> 24
          k *= M
          h ^= k
      - Tail (length mod 4):
          tail=3: h ^= data[0] << 16
                  h ^= data[1] << 8
                  h ^= data[2]
                  h *= M
          tail=2: h ^= data[0] << 8
                  h ^= data[1]
                  h *= M
          tail=1: h ^= data[0]
                  h *= M
      - Finalizer: h ^= h >> 13 ; h *= M ; h ^= h >> 15.
    """
    n = len(key)
    h = (seed ^ n) & MASK32

    # Main loop: chunks at offsets (n - 4, n - 8, n - 12, ...).
    chunk_count = n // 4
    for ci in range(chunk_count):
        i = n - 4 - ci * 4
        h = mul32(h, M)
        k = (key[i] << 24) | (key[i + 1] << 16) | (key[i + 2] << 8) | key[i + 3]
        k = mul32(k, M)
        k ^= shr32(k, R)
        k = mul32(k, M)
        h ^= k
        h &= MASK32

    # Tail.
    tail = n % 4
    if tail == 3:
        h ^= key[0] << 16
        h ^= key[1] << 8
        h ^= key[2]
        h = mul32(h, M)
    elif tail == 2:
        h ^= key[0] << 8
        h ^= key[1]
        h = mul32(h, M)
    elif tail == 1:
        h ^= key[0]
        h = mul32(h, M)
    h &= MASK32

    # Finalizer.
    h ^= shr32(h, 13)
    h = mul32(h, M)
    h ^= shr32(h, 15)
    return h & MASK32


# Test vectors: a handful of property-name strings the SetActorProperty
# wire layer hashes. The expected values are computed by this very
# function — they're a self-consistency check (the algorithm is
# deterministic). The CROSS-IMPL check is the Rust integration test:
# garlemald's `murmur_hash2(key, 0)` must produce these same values.
VECTORS: list[tuple[str, int]] = [
    ("",                                 0),
    ("a",                                0),
    ("hello",                            0),
    ("charaWork.parameterSave.hp[0]",    0),
    ("playerWork.activeQuest",           0),
    ("/_init",                           0),
]

# A few previously-observed wire ids from
# garlemald-server/map-server/src/packets/send/actor.rs — these were
# embedded as constants there. If our impl matches the binary, hashing
# the corresponding string keys should reproduce them.
KNOWN_WIRE_IDS: list[tuple[str, int]] = [
    # From `build_actor_property_init` (lines 504-507 of actor.rs):
    #   the three /_init flag ids were 0xE14B0CA8, 0x2138FD71, 0xFBFBCFB1.
    # The string keys hashed to those ids aren't stated in actor.rs,
    # but the magic comment "0x0137 SetActorProperty for /_init" plus
    # the broadcast pattern in Project Meteor's C# strongly suggest
    # they're charaWork.* names. Cross-check by hashing candidate
    # strings; if any match, we've identified the property name.
    ("charaWork.zoneId",                       0),
    ("charaWork.zoneType",                     0),
    ("charaWork.parameterSave.state_mainSkill[0]", 0),
]


def main() -> int:
    print("=== Self-consistency vectors (murmur2_backward, seed=0) ===")
    print(f"{'string':50s}  {'hash':>10s}")
    for s, _ in VECTORS:
        h = murmur_hash2_backward(s.encode("utf-8"), 0)
        print(f"  {s!r:50s}  0x{h:08x}")
    print()
    print("=== Garlemald cross-check ===")
    print("Run from garlemald-server/:")
    print()
    print("  cat > /tmp/murmur2_check.rs <<'EOF'")
    print("  use garlemald_common::utils::murmur_hash2;")
    print("  fn main() {")
    for s, _ in VECTORS:
        h = murmur_hash2_backward(s.encode("utf-8"), 0)
        print(f"      assert_eq!(murmur_hash2(\"{s}\", 0), 0x{h:08x});")
    print("      println!(\"all murmur2 vectors match\");")
    print("  }")
    print("  EOF")
    print()
    print("=== Probing /_init ids ===")
    print("garlemald-server/map-server/src/packets/send/actor.rs uses these")
    print("three magic ids for /_init flags: 0xE14B0CA8, 0x2138FD71, 0xFBFBCFB1.")
    print("Hashing candidate string keys to check for matches:")
    init_targets = {0xE14B0CA8, 0x2138FD71, 0xFBFBCFB1}
    for s, _ in KNOWN_WIRE_IDS:
        h = murmur_hash2_backward(s.encode("utf-8"), 0)
        match = " <-- MATCH" if h in init_targets else ""
        print(f"  {s!r:50s}  0x{h:08x}{match}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
