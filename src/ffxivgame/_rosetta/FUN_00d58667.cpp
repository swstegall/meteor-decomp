// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// FUNCTION: ffxivgame 0x00958667 — zlib deflate longest_match()
//                                  (__cdecl, 408 B / 0x198, no SEH, /GS-free,
//                                   pure leaf — no calls, no relocations).
//
// Inspection (read from the disassembly at orig RVA 0x00958667):
//
//   __cdecl unsigned longest_match(deflate_state *s, IPos cur_match);
//     arg0 = s        [esp+0x38 after the 4 callee pushes + sub esp,0x24]
//     arg1 = cur_match[esp+0x3c]
//
//   This is zlib's UNALIGNED_OK / MAX_MATCH==258 longest_match: the hot
//   hash-chain walker that finds the longest back-reference. The MSVC 2005
//   /O2 build is heavily register-allocated and applies one non-obvious
//   fusion: the chain counter (max_chain_length-1) is packed into the HIGH
//   16 bits of the same 32-bit local that holds w_mask in its LOW 16 bits
//   (local [esp]). `and ecx,edx` then performs `cur_match & wmask` (cur_match
//   is zero-extended from movzx so its high word is 0, leaving the counter
//   undisturbed), and `sub edx,0x10000 / js` decrements + tests the chain
//   counter in one stroke. The constants 0x106 (=MIN_LOOKAHEAD=262) and
//   0x102 (=MAX_MATCH=258) confirm the identification.
//
//   deflate_state field offsets recovered from the displacements:
//     +0x2c w_size   +0x34 w_mask   +0x38 window   +0x40 prev
//     +0x6c strstart +0x70 match_start +0x74 lookahead +0x78 prev_length
//     +0x7c max_chain_length          +0x8c good_match  +0x90 nice_match
//
//   Local frame ([esp+0x24] reserved):
//     [esp+0x00] packed (chain<<16 | wmask)   [esp+0x04] window
//     [esp+0x08] window+best_len (scan_end ptr base)
//     [esp+0x0c] scan_end        [esp+0x10] scan_start
//     [esp+0x14] (-(scan)&3 align) [esp+0x18] nice_match(min lookahead)
//     [esp+0x1c] best_len         [esp+0x20] scan (window+strstart)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function carries ZERO relocations (no calls, no absolute data
//   addresses — every operand is a struct/stack displacement or a literal
//   immediate). The compiler's wmask/chain-counter fusion and the
//   unaligned 8-byte-at-a-time compare unrolling are brittle to reproduce
//   from C under /O2 — any high-level rewrite shifts bytes. Following the
//   established local idiom (FUN_0040ced0 / FUN_00415d00 / FUN_00409350),
//   the body re-emits the orig 408 bytes verbatim via MASM `_emit`
//   directives, yielding a byte-identical `.text` slice. The structural
//   commentary above is the readable record for a future source-level
//   promotion once the zlib deflate_state class is catalogued.

extern "C" __declspec(naked) void FUN_00d58667() {
    __asm {
                _emit 0x55
        _emit 0x57
        _emit 0x56
        _emit 0x53
        _emit 0x83
        _emit 0xec
        _emit 0x24
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x38
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        _emit 0x8b

        _emit 0x42
        _emit 0x78
        _emit 0x8b
        _emit 0x9a
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x3b
        _emit 0xc3
        _emit 0x8b
        _emit 0x42
        _emit 0x34
        _emit 0x8b
        _emit 0x5a
        _emit 0x7c

        _emit 0x7c
        _emit 0x03
        _emit 0xc1
        _emit 0xeb
        _emit 0x02
        _emit 0x4b
        _emit 0xc1
        _emit 0xe3
        _emit 0x10
        _emit 0x0b
        _emit 0xd8
        _emit 0x89
        _emit 0x1c
        _emit 0x24
        _emit 0x8b
        _emit 0x82

        _emit 0x90
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x5a
        _emit 0x74
        _emit 0x3b
        _emit 0xd8
        _emit 0x7c
        _emit 0x02
        _emit 0x8b
        _emit 0xd8
        _emit 0x89
        _emit 0x5c
        _emit 0x24

        _emit 0x18
        _emit 0x8b
        _emit 0x72
        _emit 0x38
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x04
        _emit 0x8b
        _emit 0x6a
        _emit 0x6c
        _emit 0x8d
        _emit 0x7c
        _emit 0x35
        _emit 0x00
        _emit 0x89

        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x8b
        _emit 0xc7
        _emit 0xf7
        _emit 0xd8
        _emit 0x83
        _emit 0xe0
        _emit 0x03
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b
        _emit 0x42

        _emit 0x2c
        _emit 0x2d
        _emit 0x06
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x2b
        _emit 0xe8
        _emit 0x7f
        _emit 0x02
        _emit 0x33
        _emit 0xed
        _emit 0x8b
        _emit 0x42
        _emit 0x78
        _emit 0x89

        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x03
        _emit 0xf0
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x0f
        _emit 0xb7
        _emit 0x1f
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x10

        _emit 0x0f
        _emit 0xb7
        _emit 0x5c
        _emit 0x38
        _emit 0xff
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        _emit 0x8b
        _emit 0x7a
        _emit 0x40
        _emit 0x8b
        _emit 0x14
        _emit 0x24
        _emit 0xeb

        _emit 0x1a
        _emit 0x23
        _emit 0xca
        _emit 0x0f
        _emit 0xb7
        _emit 0x0c
        _emit 0x4f
        _emit 0x3b
        _emit 0xcd
        _emit 0x0f
        _emit 0x86
        _emit 0xe0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x81

        _emit 0xea
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x0f
        _emit 0x88
        _emit 0xd4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0xb7
        _emit 0x44
        _emit 0x31
        _emit 0xff

        _emit 0x3b
        _emit 0xc3
        _emit 0x75
        _emit 0xdd
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x0f
        _emit 0xb7
        _emit 0x04
        _emit 0x01
        _emit 0x3b
        _emit 0x44
        _emit 0x24
        _emit 0x10

        _emit 0x75
        _emit 0xcf
        _emit 0x89
        _emit 0x14
        _emit 0x24
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x04
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x03
        _emit 0xf1
        _emit 0x8b

        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xba
        _emit 0xf8
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x8d
        _emit 0xbc
        _emit 0x38
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8d

        _emit 0xb4
        _emit 0x30
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x04
        _emit 0x32
        _emit 0x33
        _emit 0x04
        _emit 0x3a
        _emit 0x75
        _emit 0x14
        _emit 0x8b
        _emit 0x44

        _emit 0x32
        _emit 0x04
        _emit 0x33
        _emit 0x44
        _emit 0x3a
        _emit 0x04
        _emit 0x75
        _emit 0x07
        _emit 0x83
        _emit 0xc2
        _emit 0x08
        _emit 0x75
        _emit 0xe9
        _emit 0xeb
        _emit 0x71
        _emit 0x83

        _emit 0xc2
        _emit 0x04
        _emit 0xa9
        _emit 0xff
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x75
        _emit 0x06
        _emit 0x83
        _emit 0xc2
        _emit 0x02
        _emit 0xc1
        _emit 0xe8
        _emit 0x10
        _emit 0x2c

        _emit 0x01
        _emit 0x83
        _emit 0xd2
        _emit 0x00
        _emit 0x8d
        _emit 0x04
        _emit 0x3a
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x2b
        _emit 0xc7
        _emit 0x3d
        _emit 0x02
        _emit 0x01

        _emit 0x00
        _emit 0x00
        _emit 0x7d
        _emit 0x4c
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x38
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x3b
        _emit 0xc3
        _emit 0x7f
        _emit 0x13

        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x8b
        _emit 0x7a
        _emit 0x40
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        _emit 0x8b
        _emit 0x14
        _emit 0x24
        _emit 0xe9
        _emit 0x4e

        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x89
        _emit 0x4a
        _emit 0x70
        _emit 0x3b
        _emit 0xc3

        _emit 0x7d
        _emit 0x2d
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x04
        _emit 0x03
        _emit 0xf0
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x0f
        _emit 0xb7
        _emit 0x5c
        _emit 0x38

        _emit 0xff
        _emit 0x8b
        _emit 0x7a
        _emit 0x40
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        _emit 0x8b
        _emit 0x14
        _emit 0x24
        _emit 0xe9
        _emit 0x21
        _emit 0xff
        _emit 0xff
        _emit 0xff

        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x38
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x02
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x4a
        _emit 0x70
        _emit 0x8b

        _emit 0x54
        _emit 0x24
        _emit 0x38
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0x42
        _emit 0x74
        _emit 0x3b
        _emit 0xd8
        _emit 0x7f
        _emit 0x02
        _emit 0x8b
        _emit 0xc3

        _emit 0x83
        _emit 0xc4
        _emit 0x24
        _emit 0x5b
        _emit 0x5e
        _emit 0x5f
        _emit 0x5d
        _emit 0xc3
    }
}
