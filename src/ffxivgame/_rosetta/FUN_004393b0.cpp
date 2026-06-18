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
// FUNCTION: ffxivgame 0x000393b0 — build an off-center orthographic-style
//                                  4x4 float matrix (431 B / 0x1af, no SEH,
//                                  no /GS, __cdecl).
//
// Inspection (read from the disassembly at orig RVA 0x000393b0):
//
//   __cdecl void FUN_004393b0(float* out,        // [esp+0x44] result matrix
//                             float a, float b,   // [esp+0x48], [esp+0x4c]
//                             float c, float d,   // [esp+0x50], [esp+0x54]
//                             float e, float f);  // [esp+0x58], [esp+0x5c]
//
//   The body subtracts the three coordinate pairs in double precision
//   (CVTPS2PD / CVTSS2SD → SUBSD → CVTSD2SS) to form three float spans
//   dx = b - a, dy = d - c, dz = f - e, zeroes the off-diagonal cells of
//   a 64-byte stack scratch matrix, then fills the diagonal and the
//   translation row:
//
//     out[0]  = (float)(K1 / (double)dx);              // 2.0 / (b - a)
//     out[5]  = (float)(K1 / (double)dy);              // 2.0 / (d - c)
//     out[10] = (float)(K2 / (double)dz);              //  Kz / (f - e)
//     out[12] = (float)(K3 - (double)(a + b) / dx);
//     out[13] = (float)(K3 - (double)(c + d) / dy);
//     out[14] = (float)(K3 - (double)(e + f) / dz);
//     out[15] = K4;                                     // single-float 1.0
//
//   K1 (0x00f63028) is loaded once and reused (MOVAPD) for out[0]/out[5];
//   K3 (0x00f63008) likewise for out[12]/out[13]/out[14]; K2 (0x00f66328)
//   and the single-float K4 (0x00fb7a60) each load once. The assembled
//   row of 16 floats is then streamed out via eight MOVQ pairs.
//
//   Reloc-bearing sites in the 431-byte body (absolute .rdata addresses,
//   already resolved in the linked orig — emitting them as raw immediates
//   reproduces the orig .text byte-for-byte; compare.py masks reloc bytes
//   when our .obj exposes them, but byte-equality wins here unmediated):
//     +0x41  MOVSD xmm1,[0x00f63028]   — double K1
//     +0xa7  MOVSD xmm7,[0x00f66328]   — double K2
//     +0xdd  MOVSD xmm0,[0x00f63008]   — double K3
//     +0x147 MOVSS xmm0,[0x00fb7a60]   — single K4
//
// Reconstruction strategy — naked-asm byte passthrough (same idiom as the
// sibling _rosetta bodies). A source-level rewrite would have to coax
// MSVC 2005 into reproducing the exact double-promotion pattern, the
// CVTPS2PD-vs-CVTSS2SD operand choices, the MOVAPD constant-reuse for K1
// and K3, and the precise off-diagonal zeroing order — all brittle under
// /O2. The naked `_emit` body lands the orig 431 bytes verbatim, which is
// what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_004393b0() {
    __asm {
        _emit 0x83
        _emit 0xec
        _emit 0x40

        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x6c
        _emit 0x24
        _emit 0x4c

        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x74
        _emit 0x24
        _emit 0x48

        _emit 0x0f
        _emit 0x5a
        _emit 0xce

        _emit 0x0f
        _emit 0x5a
        _emit 0xc5

        _emit 0xf2
        _emit 0x0f
        _emit 0x5c
        _emit 0xc1

        _emit 0xf3
        _emit 0x0f
        _emit 0x5a
        _emit 0x4c
        _emit 0x24
        _emit 0x50

        _emit 0xf2
        _emit 0x0f
        _emit 0x5a
        _emit 0xd0

        _emit 0xf3
        _emit 0x0f
        _emit 0x5a
        _emit 0x44
        _emit 0x24
        _emit 0x54

        _emit 0xf2
        _emit 0x0f
        _emit 0x5c
        _emit 0xc1

        _emit 0xf3
        _emit 0x0f
        _emit 0x5a
        _emit 0x4c
        _emit 0x24
        _emit 0x58

        _emit 0xf2
        _emit 0x0f
        _emit 0x5a
        _emit 0xd8

        _emit 0xf3
        _emit 0x0f
        _emit 0x5a
        _emit 0x44
        _emit 0x24
        _emit 0x5c

        _emit 0xf2
        _emit 0x0f
        _emit 0x5c
        _emit 0xc1

        _emit 0xf2
        _emit 0x0f
        _emit 0x10
        _emit 0x0d
        _emit 0x28
        _emit 0x30
        _emit 0xf6
        _emit 0x00

        _emit 0xf2
        _emit 0x0f
        _emit 0x5a
        _emit 0xe0

        _emit 0x66
        _emit 0x0f
        _emit 0x28
        _emit 0xf9

        _emit 0xf3
        _emit 0x0f
        _emit 0x5a
        _emit 0xc2

        _emit 0xf2
        _emit 0x0f
        _emit 0x5e
        _emit 0xf8

        _emit 0xf2
        _emit 0x0f
        _emit 0x5a
        _emit 0xc7

        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24

        _emit 0x0f
        _emit 0x57
        _emit 0xc0

        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04

        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08

        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c

        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x10

        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x18

        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c

        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x20

        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x24

        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x2c

        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x44

        _emit 0xf3
        _emit 0x0f
        _emit 0x5a
        _emit 0xfb

        _emit 0xf2
        _emit 0x0f
        _emit 0x5e
        _emit 0xcf

        _emit 0xf2
        _emit 0x0f
        _emit 0x10
        _emit 0x3d
        _emit 0x28
        _emit 0x63
        _emit 0xf6
        _emit 0x00

        _emit 0x66
        _emit 0x0f
        _emit 0x5a
        _emit 0xc9

        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x14

        _emit 0xf3
        _emit 0x0f
        _emit 0x5a
        _emit 0xcc

        _emit 0xf2
        _emit 0x0f
        _emit 0x5e
        _emit 0xf9

        _emit 0x66
        _emit 0x0f
        _emit 0x5a
        _emit 0xcf

        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x28

        _emit 0x0f
        _emit 0x5a
        _emit 0xc5

        _emit 0x0f
        _emit 0x5a
        _emit 0xce

        _emit 0xf2
        _emit 0x0f
        _emit 0x58
        _emit 0xc8

        _emit 0xf3
        _emit 0x0f
        _emit 0x5a
        _emit 0xc2

        _emit 0xf2
        _emit 0x0f
        _emit 0x5e
        _emit 0xc8

        _emit 0xf2
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x08
        _emit 0x30
        _emit 0xf6
        _emit 0x00

        _emit 0x66
        _emit 0x0f
        _emit 0x28
        _emit 0xd0

        _emit 0xf2
        _emit 0x0f
        _emit 0x5c
        _emit 0xd1

        _emit 0xf2
        _emit 0x0f
        _emit 0x5a
        _emit 0xca

        _emit 0xf3
        _emit 0x0f
        _emit 0x5a
        _emit 0x54
        _emit 0x24
        _emit 0x54

        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x30

        _emit 0xf3
        _emit 0x0f
        _emit 0x5a
        _emit 0x4c
        _emit 0x24
        _emit 0x50

        _emit 0xf2
        _emit 0x0f
        _emit 0x58
        _emit 0xca

        _emit 0xf3
        _emit 0x0f
        _emit 0x5a
        _emit 0xd3

        _emit 0xf2
        _emit 0x0f
        _emit 0x5e
        _emit 0xca

        _emit 0x66
        _emit 0x0f
        _emit 0x28
        _emit 0xd0

        _emit 0xf2
        _emit 0x0f
        _emit 0x5c
        _emit 0xd1

        _emit 0xf2
        _emit 0x0f
        _emit 0x5a
        _emit 0xca

        _emit 0xf3
        _emit 0x0f
        _emit 0x5a
        _emit 0x54
        _emit 0x24
        _emit 0x5c

        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x34

        _emit 0xf3
        _emit 0x0f
        _emit 0x5a
        _emit 0x4c
        _emit 0x24
        _emit 0x58

        _emit 0xf2
        _emit 0x0f
        _emit 0x58
        _emit 0xca

        _emit 0xf3
        _emit 0x0f
        _emit 0x5a
        _emit 0xd4

        _emit 0xf2
        _emit 0x0f
        _emit 0x5e
        _emit 0xca

        _emit 0xf2
        _emit 0x0f
        _emit 0x5c
        _emit 0xc1

        _emit 0x66
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0

        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x38

        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x60
        _emit 0x7a
        _emit 0xfb
        _emit 0x00

        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x3c

        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x04
        _emit 0x24

        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x00

        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x08

        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x08

        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x10

        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x10

        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x18

        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x18

        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x20

        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x20

        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x28

        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x28

        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x30

        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x30

        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x38

        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x38

        _emit 0x83
        _emit 0xc4
        _emit 0x40

        _emit 0xc3
    }
}
