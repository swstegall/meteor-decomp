# MurmurHash2 — `FUN_00d31490` ↔ garlemald's `murmur_hash2`

**Status**: ✅ validated bit-for-bit on 2026-05-01.

The 1.x client uses a custom **backward-walking** MurmurHash2 variant
to derive the 32-bit wire-id from a property's `/`-path string for
`SetActorPropertyPacket` (opcode 0x0137) — e.g. the string
`"charaWork.parameterSave.hp[0]"` hashes to `0x4232BCAA`, which is
what gets sent on the wire.

This is the *only* hash function in the SetActorProperty path; the
client does not normalise the string, lowercase it, or apply any
length suffix. Just MurmurHash2 with seed=0, reading bytes backward
from the end of the string.

## Source-of-truth function

- Binary: `ffxivgame.exe`, `.text` RVA `0x00931490`,
  `FUN_00d31490`, 170 bytes (0xAA).
- Magic constant `0x5BD1E995` appears 5 times in the function body
  (one per multiply site). This is the standard MurmurHash2 `M`.

## Garlemald's port

`garlemald-server/common/src/utils.rs::murmur_hash2` (written long
before this validation; the algorithm was reverse-engineered from
Project Meteor's C# `Common Class Lib/Utils.cs:214`, which itself
was reverse-engineered from the binary).

## Step-by-step correspondence

| binary asm (FUN_00d31490) | rust (murmur_hash2) | description |
|---|---|---|
| `MOV EAX, [ESP+0x4]` | `let data = key.as_bytes()` | data pointer |
| `PUSH ESI; MOV ESI, [ESP+0xc]` | `let mut len = key.len()` | length |
| `MOV ECX, ESI; XOR ECX, [ESP+0x10]` | `let mut h = seed ^ key.len() as u32` | initial hash state |
| `ADD EAX, ESI` | (implicit: `data_index = len - 4`) | walk-end pointer |
| `CMP ESI, 0x4; JC tail` | `while len >= 4` | guard |
| **Main loop:** | | |
| `IMUL ECX, ECX, 0x5bd1e995` | `h = h.wrapping_mul(M)` | h *= M |
| `MOVZX EBX, [EAX-0x2]` … `OR EDX, EBX` | byte-swap of `i32::from_le_bytes(data[di..di+4])` | k = (b0<<24)\|(b1<<16)\|(b2<<8)\|b3 |
| `IMUL EDX, EDX, 0x5bd1e995` | `k = k.wrapping_mul(M)` | k *= M |
| `MOV EBX, EDX; SHR EBX, 0x18; XOR EBX, EDX` | `k ^= k >> R` (R=24) | k ^= k >> 24 |
| `IMUL EBX, EBX, 0x5bd1e995` | `k = k.wrapping_mul(M)` | k *= M |
| `XOR ECX, EBX` | `h ^= k` | h ^= k |
| `SUB EAX, 0x4; SUB ESI, 0x4` | `data_index -= 4; len -= 4` | walk backward |
| `SUB EDI, 1; JNZ` | `while`-loop | iterate |
| **Tail** (cascading fall-through 3→2→1): | `match tail` | |
| tail=3: `h ^= [EAX] << 16` | `h ^= (data[0] as u32) << 16` | byte 0 |
| tail=3 then 2: `h ^= [EAX+ESI-2] << 8` | `h ^= (data[1] as u32) << 8` | byte 1 |
| tail=3 then 2 then 1: `h ^= [EAX+ESI-1]` | `h ^= data[2] as u32` | byte 2 |
| `IMUL EAX, EAX, 0x5bd1e995` | `h = h.wrapping_mul(M)` | h *= M |
| **Finalizer:** | | |
| `MOV EDX, ECX; SHR EDX, 0xd; XOR EDX, ECX` | `h ^= h >> 13` | |
| `IMUL EDX, EDX, 0x5bd1e995` | `h = h.wrapping_mul(M)` | |
| `MOV EAX, EDX; SHR EAX, 0xf; XOR EAX, EDX` | `h ^= h >> 15` | |
| `RET` | (return h) | |

The two layouts produce bit-identical hash values for any input.

## Test vectors (validated)

These were computed by `tools/validate_murmur2.py`'s pure-Python port
of the binary's algorithm and re-verified against garlemald's
`murmur_hash2` with a standalone Rust binary on 2026-05-01. All 6
match:

| string | hash (seed=0) |
|---|---:|
| `""` | `0x00000000` |
| `"a"` | `0x92685f5e` |
| `"hello"` | `0x08c5daa9` |
| `"charaWork.parameterSave.hp[0]"` | `0x4232bcaa` |
| `"playerWork.activeQuest"` | `0x40e82419` |
| `"/_init"` | `0x05c4c6b7` |

To re-run the validation:

```sh
# Compute Python expected values:
python3 tools/validate_murmur2.py

# Cross-check against garlemald's Rust impl:
rustc /tmp/murmur2_check.rs -o /tmp/murmur2_check && /tmp/murmur2_check
# (the validate_murmur2.py output prints the Rust source to drop into
# /tmp/murmur2_check.rs)
```

## Why the backward walk?

Standard MurmurHash2 walks forward through the input. The 1.x client's
variant walks backward. Two plausible reasons:

1. **Cache locality**: at the time of the function call, the string's
   *end* is the most recently-touched byte (the C-string terminator
   was just confirmed by the caller's `strlen` or equivalent). Walking
   backward means processing already-warm cache lines first.

2. **Source artifact**: the original SE engineer might have ported the
   reference MurmurHash2 with a typo and locked in the result via
   shipped test data. By the time anyone noticed, the wire ids were
   baked into server-side dispatch tables and couldn't change.

Either way, the variant is unique to FFXIV 1.x and is faithfully
reproduced by garlemald.

## What this validates

- **`SetActorPropertyPacket` wire ids in garlemald are correct.**
  Every `b.add_byte("charaWork.parameterSave.hp[0]", v)` in
  `map-server/src/packets/send/actor.rs` produces wire id `0x4232BCAA`,
  matching what the 1.x client expects.

- **Project Meteor's Murmur2 reference (C# `Utils.cs:214`) was
  correct.** Garlemald's port preserved that correctness through the
  Rust translation.

## What this does NOT validate

- The binary may *also* hash strings in other places using a different
  algorithm (e.g. for sqpack file lookup, opcode dispatch, or
  Lua-script identifier lookup). This validation only covers
  `FUN_00d31490`. Other hash functions need their own independent
  check.

- The seed parameter on `SetActorPropertyPacket` ids is hard-coded to
  0 in garlemald and Project Meteor. If the binary ever calls
  `FUN_00d31490` with a non-zero seed somewhere (e.g. for a
  per-channel salt), the ids derived from that path would be different
  and garlemald would need to thread the seed through.

## Related

- `tools/validate_murmur2.py` — Python port + test-vector generator.
- `garlemald-server/common/src/utils.rs:110` — the Rust impl.
- `garlemald-server/map-server/src/packets/send/actor.rs:573-591` —
  the SetActorProperty builders that call it.
- `asm/ffxivgame/00931490_FUN_00d31490.s` — the disassembly.
