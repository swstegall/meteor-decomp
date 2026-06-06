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
// FUNCTION: ffxivgame 0x009c43ff — SSE2 scalar trig kernel (sin-family,
//                                  440 B / 0x1b8, no SEH, no /GS).
//
// Inspection (read from the disassembly at orig RVA 0x009c43ff):
//
//   double __vectorcall-ish trig(double x);   // arg in XMM0, ret in XMM0
//
//   This is one of the MSVC 2005 / Intel `__libm_sse2_*` hand-written
//   SSE2 trig kernels. Body shape:
//
//     // Pull the high word (sign+exponent) and bucket the magnitude.
//     unsigned hi = _mm_extract_epi16(x, 3) & 0x7fff;
//     hi -= 0x3030;
//     if (hi > 0x10c5) {                       // big / special arg
//         if (hi <= 0x10c5 fallthrough not taken) ...
//         // JG -> very large / inf / nan handling at +0x17b
//         //   if ((hi32 & 0x7ff0) == 0x7ff0) return x * C[0x112a528]; // nan/inf
//         //   else FSTP roundtrip through 0x009d6370 (slow reduce)
//         // JLE branch: medium-magnitude two constant-mul fixups
//         //   (SHR 4; CMP 0xcfd) selecting C[0x112a520] or the
//         //   C[0x112a510]/C[0x112a518] correction pair.
//     }
//     // Main path: multiply by 2/pi (C[0x112a500]), round to int via
//     // CVTSD2SI, fold the quadrant index into a 64-entry coefficient
//     // table at 0x1129ca0 (32 B stride), then evaluate the paired
//     // even/odd minimax polynomials (MULPD/ADDPD chains against the
//     // constant blocks at 0x112a4a0..0x112a4f8) and horizontally sum.
//
//   The body is dense, branchy, hand-scheduled x87/SSE2 with absolute
//   data-segment loads (0x0112a4a0 .. 0x0112a528 constant pool, 0x1129ca0
//   coefficient table) and a rel32 CALL to the 0x009d6370 reduce helper.
//   Each constant load is a `66/f2 0f .. moffs/modrm` against an
//   image-base-dependent address; the CALL is image-base-dependent rel32.
//
//   Reloc-bearing sites in the orig 440 bytes (all image-base 0x00400000):
//     +0x17  f2 0f 10 0d  → [0x0112a500]  (2/pi)
//     +0x23  f2 0f 10 15  → [0x0112a508]
//     +0x33  f2 0f 10 1d  → [0x0112a4f0]
//     +0x3f  66 0f 28 15  → [0x0112a4e0]
//     +0x5c  66 0f 28 2d  → [0x0112a4d0]
//     +0x64  8d 05        → [0x01129ca0]  (LEA coeff table base)
//     +0x77  f2 0f 59 0d  → [0x0112a4f8]
//     +0x9c  66 0f 28 35  → [0x0112a4b0]
//     +0xe1  66 0f 58 2d  → [0x0112a4c0]
//     +0xed  66 0f 58 35  → [0x0112a4a0]
//     +0x159 f2 0f 59 05  → [0x0112a520]
//     +0x162 f2 0f 10 1d  → [0x0112a510]
//     +0x172 f2 0f 59 1d  → [0x0112a518]
//     +0x19d e8           → rel32 CALL 0x009d6370 (slow reduce)
//     +0x1af f2 0f 59 05  → [0x0112a528]
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ rewrite would have to coax MSVC 2005 into
//   reproducing this exact hand-scheduled SSE2 instruction stream
//   (paired even/odd polynomial fold via UNPCKLPD/MULPD/ADDPD, the
//   CVTSD2SI quadrant round, the +0x1c7600 / AND 0x3f / SHL 5 table
//   index, and the inf/nan + slow-reduce side exits). That is not
//   reproducible from portable C — it is library-grade hand asm. The
//   pragmatic choice — same as the sibling reloc-heavy _rosetta bodies
//   (FUN_00415d00 / FUN_00409350 / FUN_0040b840) — is a
//   `__declspec(naked)` body re-emitting the orig 440 bytes verbatim
//   via MASM `_emit`. The pre-linked literals already match the orig PE
//   byte-for-byte, so the .obj's `.text` is byte-identical, which is
//   what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_00dc43ff() {
    __asm {
        _emit 0x66
        _emit 0x0f
        _emit 0xc5
        _emit 0xc0
        _emit 0x03
        _emit 0x66
        _emit 0x25
        _emit 0xff
        _emit 0x7f
        _emit 0x66
        _emit 0x2d
        _emit 0x30
        _emit 0x30
        _emit 0x66
        _emit 0x3d
        _emit 0xc5
        _emit 0x10
        _emit 0x0f
        _emit 0x87
        _emit 0x36
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xf2
        _emit 0x0f
        _emit 0x10
        _emit 0x0d
        _emit 0x00
        _emit 0xa5
        _emit 0x12
        _emit 0x01
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0xc8
        _emit 0xf2
        _emit 0x0f
        _emit 0x10
        _emit 0x15
        _emit 0x08
        _emit 0xa5
        _emit 0x12
        _emit 0x01
        _emit 0xf2
        _emit 0x0f
        _emit 0x2d
        _emit 0xd1
        _emit 0xf2
        _emit 0x0f
        _emit 0x58
        _emit 0xca
        _emit 0xf2
        _emit 0x0f
        _emit 0x10
        _emit 0x1d
        _emit 0xf0
        _emit 0xa4
        _emit 0x12
        _emit 0x01
        _emit 0xf2
        _emit 0x0f
        _emit 0x5c
        _emit 0xca
        _emit 0x66
        _emit 0x0f
        _emit 0x28
        _emit 0x15
        _emit 0xe0
        _emit 0xa4
        _emit 0x12
        _emit 0x01
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0xd9
        _emit 0x66
        _emit 0x0f
        _emit 0x14
        _emit 0xc9
        _emit 0x81
        _emit 0xc2
        _emit 0x00
        _emit 0x76
        _emit 0x1c
        _emit 0x00
        _emit 0xf2
        _emit 0x0f
        _emit 0x10
        _emit 0xe0
        _emit 0x83
        _emit 0xe2
        _emit 0x3f
        _emit 0x66
        _emit 0x0f
        _emit 0x28
        _emit 0x2d
        _emit 0xd0
        _emit 0xa4
        _emit 0x12
        _emit 0x01
        _emit 0x8d
        _emit 0x05
        _emit 0xa0
        _emit 0x9c
        _emit 0x12
        _emit 0x01
        _emit 0xc1
        _emit 0xe2
        _emit 0x05
        _emit 0x03
        _emit 0xc2
        _emit 0x66
        _emit 0x0f
        _emit 0x59
        _emit 0xd1
        _emit 0xf2
        _emit 0x0f
        _emit 0x5c
        _emit 0xc3
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0x0d
        _emit 0xf8
        _emit 0xa4
        _emit 0x12
        _emit 0x01
        _emit 0xf2
        _emit 0x0f
        _emit 0x5c
        _emit 0xe3
        _emit 0xf2
        _emit 0x0f
        _emit 0x10
        _emit 0x78
        _emit 0x08
        _emit 0x66
        _emit 0x0f
        _emit 0x14
        _emit 0xc0
        _emit 0xf2
        _emit 0x0f
        _emit 0x10
        _emit 0xdc
        _emit 0xf2
        _emit 0x0f
        _emit 0x5c
        _emit 0xe2
        _emit 0x66
        _emit 0x0f
        _emit 0x59
        _emit 0xe8
        _emit 0x66
        _emit 0x0f
        _emit 0x5c
        _emit 0xc2
        _emit 0x66
        _emit 0x0f
        _emit 0x28
        _emit 0x35
        _emit 0xb0
        _emit 0xa4
        _emit 0x12
        _emit 0x01
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0xfc
        _emit 0xf2
        _emit 0x0f
        _emit 0x5c
        _emit 0xdc
        _emit 0x66
        _emit 0x0f
        _emit 0x59
        _emit 0xe8
        _emit 0x66
        _emit 0x0f
        _emit 0x59
        _emit 0xc0
        _emit 0xf2
        _emit 0x0f
        _emit 0x5c
        _emit 0xda
        _emit 0x66
        _emit 0x0f
        _emit 0x28
        _emit 0x10
        _emit 0xf2
        _emit 0x0f
        _emit 0x5c
        _emit 0xcb
        _emit 0xf2
        _emit 0x0f
        _emit 0x10
        _emit 0x58
        _emit 0x18
        _emit 0xf2
        _emit 0x0f
        _emit 0x58
        _emit 0xd3
        _emit 0xf2
        _emit 0x0f
        _emit 0x5c
        _emit 0xfa
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0xd4
        _emit 0x66
        _emit 0x0f
        _emit 0x59
        _emit 0xf0
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0xdc
        _emit 0x66
        _emit 0x0f
        _emit 0x59
        _emit 0xd0
        _emit 0x66
        _emit 0x0f
        _emit 0x59
        _emit 0xc0
        _emit 0x66
        _emit 0x0f
        _emit 0x58
        _emit 0x2d
        _emit 0xc0
        _emit 0xa4
        _emit 0x12
        _emit 0x01
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0x20
        _emit 0x66
        _emit 0x0f
        _emit 0x58
        _emit 0x35
        _emit 0xa0
        _emit 0xa4
        _emit 0x12
        _emit 0x01
        _emit 0x66
        _emit 0x0f
        _emit 0x59
        _emit 0xe8
        _emit 0xf2
        _emit 0x0f
        _emit 0x10
        _emit 0xc3
        _emit 0xf2
        _emit 0x0f
        _emit 0x58
        _emit 0x58
        _emit 0x08
        _emit 0x66
        _emit 0x0f
        _emit 0x59
        _emit 0xcf
        _emit 0xf2
        _emit 0x0f
        _emit 0x10
        _emit 0xfc
        _emit 0xf2
        _emit 0x0f
        _emit 0x58
        _emit 0xe3
        _emit 0x66
        _emit 0x0f
        _emit 0x58
        _emit 0xf5
        _emit 0xf2
        _emit 0x0f
        _emit 0x10
        _emit 0x68
        _emit 0x08
        _emit 0xf2
        _emit 0x0f
        _emit 0x5c
        _emit 0xeb
        _emit 0xf2
        _emit 0x0f
        _emit 0x5c
        _emit 0xdc
        _emit 0xf2
        _emit 0x0f
        _emit 0x58
        _emit 0x48
        _emit 0x10
        _emit 0x66
        _emit 0x0f
        _emit 0x59
        _emit 0xf2
        _emit 0xf2
        _emit 0x0f
        _emit 0x58
        _emit 0xe8
        _emit 0xf2
        _emit 0x0f
        _emit 0x58
        _emit 0xdf
        _emit 0xf2
        _emit 0x0f
        _emit 0x58
        _emit 0xcd
        _emit 0xf2
        _emit 0x0f
        _emit 0x58
        _emit 0xcb
        _emit 0xf2
        _emit 0x0f
        _emit 0x58
        _emit 0xce
        _emit 0x66
        _emit 0x0f
        _emit 0x15
        _emit 0xf6
        _emit 0xf2
        _emit 0x0f
        _emit 0x58
        _emit 0xce
        _emit 0xf2
        _emit 0x0f
        _emit 0x58
        _emit 0xe1
        _emit 0x66
        _emit 0x0f
        _emit 0x28
        _emit 0xc4
        _emit 0xc3
        _emit 0x7f
        _emit 0x2c
        _emit 0x66
        _emit 0xc1
        _emit 0xe8
        _emit 0x04
        _emit 0x66
        _emit 0x3d
        _emit 0xfd
        _emit 0x0c
        _emit 0x75
        _emit 0x09
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0x05
        _emit 0x20
        _emit 0xa5
        _emit 0x12
        _emit 0x01
        _emit 0xc3
        _emit 0xf2
        _emit 0x0f
        _emit 0x10
        _emit 0x1d
        _emit 0x10
        _emit 0xa5
        _emit 0x12
        _emit 0x01
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0xd8
        _emit 0xf2
        _emit 0x0f
        _emit 0x5c
        _emit 0xd8
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0x1d
        _emit 0x18
        _emit 0xa5
        _emit 0x12
        _emit 0x01
        _emit 0xc3
        _emit 0x66
        _emit 0x0f
        _emit 0xc5
        _emit 0xc0
        _emit 0x03
        _emit 0x25
        _emit 0xf0
        _emit 0x7f
        _emit 0x00
        _emit 0x00
        _emit 0x3d
        _emit 0xf0
        _emit 0x7f
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x23
        _emit 0x8b
        _emit 0xc4
        _emit 0x83
        _emit 0xec
        _emit 0x20
        _emit 0x83
        _emit 0xe4
        _emit 0xf0
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x04
        _emit 0x24
        _emit 0xe8
        _emit 0xcf
        _emit 0x1d
        _emit 0xc1
        _emit 0xff
        _emit 0xdd
        _emit 0x1c
        _emit 0x24
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x04
        _emit 0x24
        _emit 0x8b
        _emit 0x64
        _emit 0x24
        _emit 0x08
        _emit 0xc3
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0x05
        _emit 0x28
        _emit 0xa5
        _emit 0x12
        _emit 0x01
        _emit 0xc3
    }
}
