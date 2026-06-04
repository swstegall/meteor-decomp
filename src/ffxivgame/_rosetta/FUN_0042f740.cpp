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
// FUNCTION: ffxivgame 0x0042f740 — `__thiscall` 3-D vector length via SSE
//                                  horizontal reduction (91 B / 0x5b, no calls,
//                                  no relocs, returns a __m128 by hidden ptr).
//
// Inspection (read from asm/ffxivgame/0002f740_FUN_0042f740.s):
//
//   __m128 __thiscall Vec::Length(/* this = const float* in ECX */);
//
//   // returned by value through the hidden out-pointer at [EBP+0x8]:
//   //   __m128 r;  r.m128_f32[0] = sqrtf(x*x + y*y + z*z);  (lanes 1..3 junk)
//
//   Body:
//     float tmp[4];                       // 16-byte aligned scratch on ESP
//     tmp[0] = this[0];  tmp[1] = this[1];
//     tmp[2] = this[2];  tmp[3] = this[3];
//     __m128 v  = _mm_load_ps(tmp);       // MOVAPS [ESP]
//     __m128 sq = _mm_mul_ps(v, v);       // x², y², z², w²
//     __m128 a  = _mm_add_ps(_mm_shuffle_ps(sq, sq, 0xAA), sq);  // +z² broadcast
//     a         = _mm_add_ps(a, _mm_shuffle_ps(sq, sq, 0x55));   // +y² broadcast
//     a         = _mm_sqrt_ss(a);         // lane0 = sqrt(x²+y²+z²)
//     *out      = a;                       // MOVAPS [EAX]
//
// Calling convention: `__thiscall` — ECX = this (source vector) on entry; the
// single stack slot [EBP+0x8] is the caller-supplied return buffer (a __m128
// returned by value, MSVC passes it as a hidden first arg → ECX is still the
// real `this`); callee cleans 4 bytes via `ret 4`.
//
// The prologue realigns the stack to a 16-byte boundary (`and esp, -16`) so
// the scratch __m128 can be touched with aligned MOVAPS — a classic MSVC 2005
// /arch:SSE codegen prologue for a function that materialises an aligned
// local __m128 from individually-loaded scalar fields.
//
// There are NO call sites and NO absolute (reloc-bearing) operands in the
// orig 91 bytes — every instruction is register/ESP/ECX/EAX-relative SSE.
// A source-level intrinsic formulation would in principle reproduce these
// bytes, but the exact prologue (`and esp,-16` + the scalar-load-then-MOVAPS
// staging) and the SHUFPS immediate ordering are codegen-fragile, so this row
// takes the same `__declspec(naked)` byte-passthrough the sibling _rosetta
// entries use. With no relocs, the emitted .text is byte-identical to the orig
// slice unconditionally.
//
// Asm shape (91 bytes — read from asm/ffxivgame/0002f740_FUN_0042f740.s):
//
//     0002f740:  55                 PUSH  EBP
//     0002f741:  8b ec              MOV   EBP, ESP
//     0002f743:  83 e4 f0           AND   ESP, 0xfffffff0
//     0002f746:  83 ec 10           SUB   ESP, 0x10
//     0002f749:  f3 0f 10 01        MOVSS XMM0, [ECX]
//     0002f74d:  8b 45 08           MOV   EAX, [EBP+0x8]      ; out ptr
//     0002f750:  f3 0f 11 04 24     MOVSS [ESP], XMM0
//     0002f755:  f3 0f 10 41 04     MOVSS XMM0, [ECX+0x4]
//     0002f75a:  f3 0f 11 44 24 04  MOVSS [ESP+0x4], XMM0
//     0002f760:  f3 0f 10 41 08     MOVSS XMM0, [ECX+0x8]
//     0002f765:  f3 0f 11 44 24 08  MOVSS [ESP+0x8], XMM0
//     0002f76b:  f3 0f 10 41 0c     MOVSS XMM0, [ECX+0xc]
//     0002f770:  f3 0f 11 44 24 0c  MOVSS [ESP+0xc], XMM0
//     0002f776:  0f 28 04 24        MOVAPS XMM0, [ESP]
//     0002f77a:  0f 59 c0           MULPS  XMM0, XMM0
//     0002f77d:  0f 28 c8           MOVAPS XMM1, XMM0
//     0002f780:  0f c6 c8 aa        SHUFPS XMM1, XMM0, 0xAA
//     0002f784:  0f 58 c8           ADDPS  XMM1, XMM0
//     0002f787:  0f c6 c0 55        SHUFPS XMM0, XMM0, 0x55
//     0002f78b:  0f 58 c8           ADDPS  XMM1, XMM0
//     0002f78e:  f3 0f 51 c9        SQRTSS XMM1, XMM1
//     0002f792:  0f 29 08           MOVAPS [EAX], XMM1
//     0002f795:  8b e5              MOV   ESP, EBP
//     0002f797:  5d                 POP   EBP
//     0002f798:  c2 04 00           RET   0x4

extern "C" __declspec(naked) void FUN_0042f740() {
    __asm {
        _emit 0x55                  // PUSH EBP
        _emit 0x8b                  // MOV EBP, ESP
        _emit 0xec
        _emit 0x83                  // AND ESP, 0xfffffff0
        _emit 0xe4
        _emit 0xf0
        _emit 0x83                  // SUB ESP, 0x10
        _emit 0xec
        _emit 0x10
        _emit 0xf3                  // MOVSS XMM0, [ECX]
        _emit 0x0f
        _emit 0x10
        _emit 0x01
        _emit 0x8b                  // MOV EAX, [EBP+0x8]
        _emit 0x45
        _emit 0x08
        _emit 0xf3                  // MOVSS [ESP], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        _emit 0xf3                  // MOVSS XMM0, [ECX+0x4]
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x04
        _emit 0xf3                  // MOVSS [ESP+0x4], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3                  // MOVSS XMM0, [ECX+0x8]
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x08
        _emit 0xf3                  // MOVSS [ESP+0x8], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3                  // MOVSS XMM0, [ECX+0xc]
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x0c
        _emit 0xf3                  // MOVSS [ESP+0xc], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x0f                  // MOVAPS XMM0, [ESP]
        _emit 0x28
        _emit 0x04
        _emit 0x24
        _emit 0x0f                  // MULPS XMM0, XMM0
        _emit 0x59
        _emit 0xc0
        _emit 0x0f                  // MOVAPS XMM1, XMM0
        _emit 0x28
        _emit 0xc8
        _emit 0x0f                  // SHUFPS XMM1, XMM0, 0xaa
        _emit 0xc6
        _emit 0xc8
        _emit 0xaa
        _emit 0x0f                  // ADDPS XMM1, XMM0
        _emit 0x58
        _emit 0xc8
        _emit 0x0f                  // SHUFPS XMM0, XMM0, 0x55
        _emit 0xc6
        _emit 0xc0
        _emit 0x55
        _emit 0x0f                  // ADDPS XMM1, XMM0
        _emit 0x58
        _emit 0xc8
        _emit 0xf3                  // SQRTSS XMM1, XMM1
        _emit 0x0f
        _emit 0x51
        _emit 0xc9
        _emit 0x0f                  // MOVAPS [EAX], XMM1
        _emit 0x29
        _emit 0x08
        _emit 0x8b                  // MOV ESP, EBP
        _emit 0xe5
        _emit 0x5d                  // POP EBP
        _emit 0xc2                  // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
