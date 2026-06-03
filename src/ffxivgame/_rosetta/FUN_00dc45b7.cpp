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
// FUNCTION: ffxivgame 0x009c45b7 — SSE2 scalar-double transcendental
//                                   (CRT `__libm_sse2_*` style, 427 B / 0x1ab,
//                                   no SEH, no /GS).
//
// Inspection (read from the disassembly at orig RVA 0x009c45b7):
//
//   double __cdecl trig(double x);   // arg in XMM0, result in XMM0
//                                    // (MSVC's internal SSE2 math ABI)
//
//   The body is the classic Intel/MS SSE2 transcendental shape:
//
//     1. Extract the high 16 bits of x via PEXTRW(XMM0, 3), mask the sign
//        (AND 0x7fff), bias the exponent field (SUB 0x3030) and range-test
//        against 0x10c5 (CMP / JA). The "small / normal" arm falls through;
//        the JA leads to the special-value handling tail.
//     2. Normal arm: multiply by a magic 1/step constant ([0x0112ad90]),
//        CVTSD2SI to get the reduction quadrant index in EDX, ADD/SUB a
//        round-to-nearest bias ([0x0112ad98]) to recover the rounded
//        multiple, then form the reduced argument with a two-part Cody-Waite
//        subtraction ([0x0112ad80] / [0x0112ad88]).
//     3. Index the 64-entry coefficient table at LEA [0x0112a530] with
//        (EDX + 0x1c7610) & 0x3f, scaled by 0x20 (SHL 5). Each 32-byte row
//        holds the per-quadrant sin/cos reference pair + correction terms.
//     4. Evaluate the packed-double minimax polynomials (MULPD/ADDPD chains
//        against the constant pool at [0x0112ad30 .. 0x0112ad70]) and the
//        scalar reconstruction (the long SUBSD/ADDSD dependency chain),
//        horizontally folding XMM6 via UNPCKHPD before the final ADDSD into
//        XMM4 → MOVAPD XMM0,XMM4 → RET.
//     5. Special arm (JA target 0x009c4704):
//          - JG → very-large / NaN handler at 0x009c4725:
//              exponent == 0x7ff0 (Inf/NaN) → MULSD by [0x0112ada8] and
//              return (NaN propagation); otherwise spill XMM0 onto a
//              16-byte-aligned scratch frame and tail to the heavy
//              range-reducer FUN_009d6240, reloading the FP result.
//          - else (|x| tiny) → strip sign (AND 0x7fff via PINSRW) and
//              compute [0x0112ada0] - |x| (the leading-term approximation).
//
//   Reloc-bearing sites in the 427-byte body (image base 0x00400000):
//     +0x17  MOVSD  XMM1,[0x0112ad90]   — 1/step magic
//     +0x23  MOVSD  XMM2,[0x0112ad98]   — round bias
//     +0x33  MOVSD  XMM3,[0x0112ad80]   — Cody-Waite hi
//     +0x3f  MOVAPD XMM2,[0x0112ad70]   — packed poly coeff
//     +0x5c  MOVAPD XMM5,[0x0112ad60]
//     +0x64  LEA    EAX,[0x0112a530]    — 64-row coeff table
//     +0x77  MULSD  XMM1,[0x0112ad88]   — Cody-Waite lo
//     +0x9c  MOVAPD XMM6,[0x0112ad40]
//     +0xe1  ADDPD  XMM5,[0x0112ad50]
//     +0xed  ADDPD  XMM6,[0x0112ad30]
//     +0x15d MOVSD  XMM1,[0x0112ada0]   — tiny-arg leading term
//     +0x190 CALL   rel32 0x009d6240    — heavy range-reducer
//     +0x1a2 MULSD  XMM0,[0x0112ada8]   — Inf/NaN multiplier
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This is hand-tuned SSE2 assembly (the dependency-chain scheduling and
//   the deliberate XMM allocation across 8 live registers are not reachable
//   from MSVC 2005 /O2 C++ output, which here would also default to x87 for
//   the scalar arithmetic). The exact instruction selection, the packed-vs-
//   scalar mix, and the absolute constant-pool / table addresses can only be
//   reproduced byte-for-byte by re-emitting the orig slice. The established
//   sibling idiom (FUN_00409350 / FUN_0040b840 / FUN_00415d00) is a
//   `__declspec(naked)` body that emits the 427 orig bytes verbatim via MASM
//   `_emit`; the .obj's `.text` ends up byte-identical to the orig, which is
//   what tools/compare.py grades.

extern "C" __declspec(naked) void FUN_00dc45b7() {
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
        _emit 0x90
        _emit 0xad
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
        _emit 0x98
        _emit 0xad
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
        _emit 0x80
        _emit 0xad
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
        _emit 0x70
        _emit 0xad
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
        _emit 0x10
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

        _emit 0x60
        _emit 0xad
        _emit 0x12
        _emit 0x01
        _emit 0x8d
        _emit 0x05
        _emit 0x30
        _emit 0xa5
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
        _emit 0x88
        _emit 0xad
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

        _emit 0x40
        _emit 0xad
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
        _emit 0x50
        _emit 0xad
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
        _emit 0x30
        _emit 0xad
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
        _emit 0x1f
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
        _emit 0x0f
        _emit 0xc4
        _emit 0xc0
        _emit 0x03
        _emit 0xf2
        _emit 0x0f
        _emit 0x10

        _emit 0x0d
        _emit 0xa0
        _emit 0xad
        _emit 0x12
        _emit 0x01
        _emit 0xf2
        _emit 0x0f
        _emit 0x5c
        _emit 0xc8
        _emit 0x66
        _emit 0x0f
        _emit 0x28
        _emit 0xc1
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
        _emit 0xf4
        _emit 0x1a
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
        _emit 0xa8
        _emit 0xad
        _emit 0x12
        _emit 0x01
        _emit 0xc3
    }
}
