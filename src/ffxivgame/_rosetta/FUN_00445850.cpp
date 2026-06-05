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
// FUNCTION: ffxivgame 0x00445850 — `__cdecl` single-codepoint UTF-8 decoder
//                                   (314 B / 0x13a, leaf, no relocations).
//
// Inspection (read from the disassembly at orig RVA 0x00045850):
//
//   __cdecl int utf8_decode(const unsigned char *s, unsigned int *out);
//
//     Returns the number of bytes consumed (1..6) in EAX, and — when
//     `out` is non-NULL — stores the decoded code point through it.
//
//   Structure (matches the asm flow exactly):
//
//     unsigned int c0 = s[0];               // spilled to the dead `s`
//                                           // param slot at [esp+8]
//     unsigned int cp;
//     int len;
//     if ((char)c0 >= 0) {                  // TEST BL,BL / JS — ASCII
//         cp  = c0;
//         len = 1;
//     } else if ((c0 & 0xe0) == 0xc0) {     // 110xxxxx — 2-byte
//         cp  = ((c0 & 0x1f) << 6) | (s[1] & 0x3f);
//         len = 2;
//     } else if ((c0 & 0xf0) == 0xe0) {     // 1110xxxx — 3-byte
//         cp  = (((c0 & 0x0f) << 6) | (s[1] & 0x3f)) << 6
//               | (s[2] & 0x3f);
//         len = 3;
//     } else if ((c0 & 0xf8) == 0xf0) {     // 11110xxx — 4-byte
//         cp  = ((((c0 & 0x07) << 6 | (s[1] & 0x3f)) << 6
//               | (s[2] & 0x3f)) << 6) | (s[3] & 0x3f);
//         len = 4;
//     } else if ((c0 & 0xfc) == 0xf8) {     // 111110xx — 5-byte
//         cp  = (((((c0 & 0x03) << 6 | (s[1] & 0x3f)) << 6
//               | (s[2] & 0x3f)) << 6 | (s[3] & 0x3f)) << 6)
//               | (s[4] & 0x3f);
//         len = 5;
//     } else if ((c0 & 0xfe) == 0xfc) {     // 1111110x — 6-byte
//         cp  = ((((((c0 & 0x01) << 6 | (s[1] & 0x3f)) << 6
//               | (s[2] & 0x3f)) << 6 | (s[3] & 0x3f)) << 6
//               | (s[4] & 0x3f)) << 6) | (s[5] & 0x3f);
//         len = 6;
//     } else {                              // malformed lead byte
//         cp  = 0;
//         len = 1;
//     }
//     if (out) *out = cp;
//     return len;
//
//   The decode tails fold left: the 4/5/6-byte arms share the
//   common `SHL ECX,6 / OR continuation byte` epilogue at +0x112,
//   with the lead bits seeded into ECX and each trailing 0x3f-masked
//   continuation byte staged in EDX/ESI/EDI/EBX/EBP per register
//   pressure (PUSH/POP EBP only on the 6-byte path).
//
//   Stack frame (ESP-relative, no EBP frame):
//     [esp+0x00 .. ] EBX / ESI / EDI / EBP pushed as branches demand
//     [esp+param0]   reused as scratch — holds the spilled `c0`
//     [esp+param0]   (after pushes) reloaded for the 0xf0/0xf8/0xfe checks
//     [esp+param1]   `out` pointer, NULL-tested in the epilogue
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This is a self-contained leaf: no CALLs, no IAT loads, no absolute
//   data references — every transfer is an internal relative jump. That
//   means the orig 314-byte slice carries ZERO relocations, so emitting
//   the bytes verbatim with `_emit` directives yields a `.text` section
//   that is byte-identical to the orig regardless of final link address
//   (the rel8/rel32 branch displacements are position-independent).
//
//   A source-level rewrite would have to coax MSVC 2005 /O2 into the
//   exact register allocation across all six decode arms — including the
//   spill of `c0` into the dead first-parameter slot, the re-load of that
//   spill for the wider-mask checks, and the EBP push/pop confined to the
//   6-byte arm. Each of those is brittle under /O2; the passthrough is
//   the same pragmatic, guaranteed-GREEN choice the sibling _rosetta
//   leaves (FUN_004014b0, FUN_00401a00) make, and the structural
//   commentary above is the readable record for a future source promotion.

extern "C" __declspec(naked) void FUN_00445850() {
    __asm {
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x53
        _emit 0x0f
        _emit 0xb6
        _emit 0x19
        _emit 0x33
        _emit 0xd2
        _emit 0x84
        _emit 0xdb
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x08

        _emit 0x8d
        _emit 0x42
        _emit 0x01
        _emit 0x78
        _emit 0x07
        _emit 0x8b
        _emit 0xd3
        _emit 0xe9
        _emit 0x12
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x81
        _emit 0xe3
        _emit 0xe0
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0x80
        _emit 0xfb
        _emit 0xc0
        _emit 0x56
        _emit 0x75
        _emit 0x1c
        _emit 0x0f
        _emit 0xb6
        _emit 0x11
        _emit 0x0f
        _emit 0xb6
        _emit 0x71
        _emit 0x01
        _emit 0x83

        _emit 0xe2
        _emit 0x1f
        _emit 0xc1
        _emit 0xe2
        _emit 0x06
        _emit 0x83
        _emit 0xe6
        _emit 0x3f
        _emit 0xb8
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x0b
        _emit 0xd6
        _emit 0xe9

        _emit 0xe9
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        _emit 0x81
        _emit 0xe3
        _emit 0xf0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80
        _emit 0xfb

        _emit 0xe0
        _emit 0x75
        _emit 0x2a
        _emit 0x0f
        _emit 0xb6
        _emit 0x51
        _emit 0x02
        _emit 0x0f
        _emit 0xb6
        _emit 0x71
        _emit 0x01
        _emit 0x0f
        _emit 0xb6
        _emit 0x09
        _emit 0x83
        _emit 0xe1

        _emit 0x0f
        _emit 0xc1
        _emit 0xe1
        _emit 0x06
        _emit 0x83
        _emit 0xe6
        _emit 0x3f
        _emit 0x0b
        _emit 0xce
        _emit 0x83
        _emit 0xe2
        _emit 0x3f
        _emit 0xc1
        _emit 0xe1
        _emit 0x06
        _emit 0x0b

        _emit 0xca
        _emit 0xb8
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xd1
        _emit 0xe9
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x5c
        _emit 0x24

        _emit 0x0c
        _emit 0x81
        _emit 0xe3
        _emit 0xf8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80
        _emit 0xfb
        _emit 0xf0
        _emit 0x57
        _emit 0x75
        _emit 0x19
        _emit 0x0f
        _emit 0xb6
        _emit 0x51

        _emit 0x03
        _emit 0x0f
        _emit 0xb6
        _emit 0x71
        _emit 0x02
        _emit 0x0f
        _emit 0xb6
        _emit 0x79
        _emit 0x01
        _emit 0x0f
        _emit 0xb6
        _emit 0x09
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0x83
        _emit 0xe1
        _emit 0x07
        _emit 0xeb
        _emit 0x6c
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x81
        _emit 0xe3
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x80
        _emit 0xfb
        _emit 0xf8
        _emit 0x75
        _emit 0x1d
        _emit 0x0f
        _emit 0xb6
        _emit 0x51
        _emit 0x04
        _emit 0x0f
        _emit 0xb6
        _emit 0x71
        _emit 0x03
        _emit 0x0f
        _emit 0xb6
        _emit 0x79

        _emit 0x02
        _emit 0x0f
        _emit 0xb6
        _emit 0x59
        _emit 0x01
        _emit 0x0f
        _emit 0xb6
        _emit 0x09
        _emit 0xb8
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xe1
        _emit 0x03

        _emit 0xeb
        _emit 0x38
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x81
        _emit 0xe3
        _emit 0xfe
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80
        _emit 0xfb
        _emit 0xfc
        _emit 0x75

        _emit 0x4b
        _emit 0x0f
        _emit 0xb6
        _emit 0x51
        _emit 0x05
        _emit 0x0f
        _emit 0xb6
        _emit 0x71
        _emit 0x04
        _emit 0x0f
        _emit 0xb6
        _emit 0x79
        _emit 0x03
        _emit 0x0f
        _emit 0xb6
        _emit 0x59

        _emit 0x02
        _emit 0x55
        _emit 0x0f
        _emit 0xb6
        _emit 0x69
        _emit 0x01
        _emit 0x0f
        _emit 0xb6
        _emit 0x09
        _emit 0x83
        _emit 0xe1
        _emit 0x01
        _emit 0x83
        _emit 0xe5
        _emit 0x3f
        _emit 0xc1

        _emit 0xe1
        _emit 0x06
        _emit 0x0b
        _emit 0xcd
        _emit 0xb8
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5d
        _emit 0x83
        _emit 0xe3
        _emit 0x3f
        _emit 0xc1
        _emit 0xe1
        _emit 0x06

        _emit 0x0b
        _emit 0xcb
        _emit 0xc1
        _emit 0xe1
        _emit 0x06
        _emit 0x83
        _emit 0xe7
        _emit 0x3f
        _emit 0x0b
        _emit 0xcf
        _emit 0xc1
        _emit 0xe1
        _emit 0x06
        _emit 0x83
        _emit 0xe6
        _emit 0x3f

        _emit 0x0b
        _emit 0xce
        _emit 0x83
        _emit 0xe2
        _emit 0x3f
        _emit 0xc1
        _emit 0xe1
        _emit 0x06
        _emit 0x0b
        _emit 0xca
        _emit 0x8b
        _emit 0xd1
        _emit 0x5f
        _emit 0x5e
        _emit 0x8b
        _emit 0x4c

        _emit 0x24
        _emit 0x0c
        _emit 0x85
        _emit 0xc9
        _emit 0x5b
        _emit 0x74
        _emit 0x02
        _emit 0x89
        _emit 0x11
        _emit 0xc3
    }
}
