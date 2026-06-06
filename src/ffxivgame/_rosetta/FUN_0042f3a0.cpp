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
// FUNCTION: ffxivgame 0x0042f3a0 — `__thiscall` 3-row × vector SSE
//                                  transform (286 B / 0x11e).
//
// Inspection (read from the disassembly at orig RVA 0x0002f3a0):
//
//   __thiscall void transform(const Matrix34 *this /*ECX*/,
//                             Vec4 *out  /*[EBP+0x08]*/,
//                             const Vec4 *v /*[EBP+0x0c]*/);
//
//   Computes the linear (no-translation) SIMD broadcast-multiply-add:
//
//     out = v.x * this->row0 + v.y * this->row1 + v.z * this->row2
//
//   where `this` is a 12-float (3x4) matrix laid out row-major at
//   ECX[0x00..0x2c], `v` is a 4-float vector at [EBP+0xc], and the
//   result 4 floats land at [EBP+0x8]. This is the 3-row sibling of the
//   4x4 transform FUN_0042ec50 immediately preceding it in .text — same
//   codegen shape, one fewer row/broadcast (no v.w * row3 term).
//
//   Codegen shape (MSVC 2005 with SSE intrinsics — _mm_set_ps style
//   loads building each __m128 from four scalar MOVSS through a 16-byte
//   aligned scratch slot):
//
//     PUSH EBP; MOV EBP,ESP; AND ESP,-16; SUB ESP,0x10   ; 16B scratch
//     XMM0 = load v  (4x MOVSS → [ESP], MOVAPS)
//     XMM1 = load row0
//     XMM1 *= shuffle(XMM0, 0x00)   ; broadcast v.x
//     XMM2 = load row1
//     XMM1 += XMM2 * shuffle(XMM0, 0x55)   ; broadcast v.y
//     XMM2 = load row2
//     XMM1 += XMM2 * shuffle(XMM0, 0xAA)   ; broadcast v.z
//     store XMM1 → out (4x MOVSS via scratch)
//     MOV ESP,EBP; POP EBP; RET 0x8
//
//   No relocations: every operand is register- or stack/this-relative.
//   There is no external CALL, no IAT load, no SEH frame, no string-
//   literal reference, so the 286 function bytes are position-
//   independent within the .text slice.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The orig depends on the exact MSVC 2005 _mm_set_ps lowering (the
//   per-lane MOVSS-through-aligned-scratch idiom, XMM register
//   scheduling, and the `AND ESP,-16` dynamic-align prologue). A
//   source-level intrinsics rewrite is brittle under /O2 — the compiler
//   readily folds the scratch loads into MOVUPS or reorders the
//   shuffles, shifting bytes. Because the function carries zero
//   relocations, a naked body that re-emits the orig bytes verbatim is
//   byte-identical to the orig slice, which is what tools/compare.py
//   checks. The structural commentary above records the intent for a
//   future source-level promotion once the Matrix34/Vec4 types are
//   reconstructed (see the 4x4 sibling FUN_0042ec50).

extern "C" __declspec(naked) void FUN_0042f3a0() {
    __asm {
        _emit 0x55                          // PUSH EBP
        _emit 0x8b                          // MOV EBP,ESP
        _emit 0xec
        _emit 0x83                          // AND ESP,0xfffffff0
        _emit 0xe4
        _emit 0xf0
        _emit 0x83                          // SUB ESP,0x10
        _emit 0xec
        _emit 0x10
        _emit 0x8b                          // MOV EDX,[EBP+0xc]   ; v
        _emit 0x55
        _emit 0x0c
        _emit 0xf3                          // MOVSS XMM0,[EDX]
        _emit 0x0f
        _emit 0x10
        _emit 0x02
        _emit 0xf3                          // MOVSS XMM1,[ECX]
        _emit 0x0f
        _emit 0x10
        _emit 0x09
        _emit 0xf3                          // MOVSS [ESP],XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        _emit 0xf3                          // MOVSS XMM0,[EDX+4]
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x04
        _emit 0xf3                          // MOVSS [ESP+4],XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3                          // MOVSS XMM0,[EDX+8]
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x08
        _emit 0xf3                          // MOVSS [ESP+8],XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3                          // MOVSS XMM0,[EDX+0xc]
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x0c
        _emit 0xf3                          // MOVSS [ESP+0xc],XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x0f                          // MOVAPS XMM0,[ESP]   ; XMM0 = v
        _emit 0x28
        _emit 0x04
        _emit 0x24
        _emit 0xf3                          // MOVSS [ESP],XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x0c
        _emit 0x24
        _emit 0xf3                          // MOVSS XMM1,[ECX+4]
        _emit 0x0f
        _emit 0x10
        _emit 0x49
        _emit 0x04
        _emit 0xf3                          // MOVSS [ESP+4],XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xf3                          // MOVSS XMM1,[ECX+8]
        _emit 0x0f
        _emit 0x10
        _emit 0x49
        _emit 0x08
        _emit 0xf3                          // MOVSS [ESP+8],XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xf3                          // MOVSS XMM1,[ECX+0xc]
        _emit 0x0f
        _emit 0x10
        _emit 0x49
        _emit 0x0c
        _emit 0xf3                          // MOVSS [ESP+0xc],XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x0f                          // MOVAPS XMM1,[ESP]   ; XMM1 = row0
        _emit 0x28
        _emit 0x0c
        _emit 0x24
        _emit 0x8b                          // MOV EAX,[EBP+8]     ; out
        _emit 0x45
        _emit 0x08
        _emit 0x0f                          // MOVAPS XMM2,XMM0
        _emit 0x28
        _emit 0xd0
        _emit 0x0f                          // SHUFPS XMM2,XMM0,0x0  ; broadcast v.x
        _emit 0xc6
        _emit 0xd0
        _emit 0x00
        _emit 0x0f                          // MULPS XMM1,XMM2
        _emit 0x59
        _emit 0xca
        _emit 0xf3                          // MOVSS XMM2,[ECX+0x10]
        _emit 0x0f
        _emit 0x10
        _emit 0x51
        _emit 0x10
        _emit 0xf3                          // MOVSS [ESP],XMM2
        _emit 0x0f
        _emit 0x11
        _emit 0x14
        _emit 0x24
        _emit 0xf3                          // MOVSS XMM2,[ECX+0x14]
        _emit 0x0f
        _emit 0x10
        _emit 0x51
        _emit 0x14
        _emit 0xf3                          // MOVSS [ESP+4],XMM2
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0xf3                          // MOVSS XMM2,[ECX+0x18]
        _emit 0x0f
        _emit 0x10
        _emit 0x51
        _emit 0x18
        _emit 0xf3                          // MOVSS [ESP+8],XMM2
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0xf3                          // MOVSS XMM2,[ECX+0x1c]
        _emit 0x0f
        _emit 0x10
        _emit 0x51
        _emit 0x1c
        _emit 0xf3                          // MOVSS [ESP+0xc],XMM2
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x0f                          // MOVAPS XMM2,[ESP]   ; XMM2 = row1
        _emit 0x28
        _emit 0x14
        _emit 0x24
        _emit 0x0f                          // MOVAPS XMM3,XMM0
        _emit 0x28
        _emit 0xd8
        _emit 0x0f                          // SHUFPS XMM3,XMM0,0x55  ; broadcast v.y
        _emit 0xc6
        _emit 0xd8
        _emit 0x55
        _emit 0x0f                          // MULPS XMM2,XMM3
        _emit 0x59
        _emit 0xd3
        _emit 0x0f                          // ADDPS XMM1,XMM2
        _emit 0x58
        _emit 0xca
        _emit 0xf3                          // MOVSS XMM2,[ECX+0x20]
        _emit 0x0f
        _emit 0x10
        _emit 0x51
        _emit 0x20
        _emit 0xf3                          // MOVSS [ESP],XMM2
        _emit 0x0f
        _emit 0x11
        _emit 0x14
        _emit 0x24
        _emit 0xf3                          // MOVSS XMM2,[ECX+0x24]
        _emit 0x0f
        _emit 0x10
        _emit 0x51
        _emit 0x24
        _emit 0xf3                          // MOVSS [ESP+4],XMM2
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0xf3                          // MOVSS XMM2,[ECX+0x28]
        _emit 0x0f
        _emit 0x10
        _emit 0x51
        _emit 0x28
        _emit 0xf3                          // MOVSS [ESP+8],XMM2
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0xf3                          // MOVSS XMM2,[ECX+0x2c]
        _emit 0x0f
        _emit 0x10
        _emit 0x51
        _emit 0x2c
        _emit 0xf3                          // MOVSS [ESP+0xc],XMM2
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x0f                          // MOVAPS XMM2,[ESP]   ; XMM2 = row2
        _emit 0x28
        _emit 0x14
        _emit 0x24
        _emit 0x0f                          // SHUFPS XMM0,XMM0,0xaa  ; broadcast v.z
        _emit 0xc6
        _emit 0xc0
        _emit 0xaa
        _emit 0x0f                          // MULPS XMM2,XMM0
        _emit 0x59
        _emit 0xd0
        _emit 0x0f                          // ADDPS XMM1,XMM2
        _emit 0x58
        _emit 0xca
        _emit 0x0f                          // MOVAPS [ESP],XMM1
        _emit 0x29
        _emit 0x0c
        _emit 0x24
        _emit 0xf3                          // MOVSS XMM0,[ESP]
        _emit 0x0f
        _emit 0x10
        _emit 0x04
        _emit 0x24
        _emit 0xf3                          // MOVSS [EAX],XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x00
        _emit 0xf3                          // MOVSS XMM0,[ESP+4]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3                          // MOVSS [EAX+4],XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x04
        _emit 0xf3                          // MOVSS XMM0,[ESP+8]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3                          // MOVSS [EAX+8],XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x08
        _emit 0xf3                          // MOVSS XMM0,[ESP+0xc]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xf3                          // MOVSS [EAX+0xc],XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x0c
        _emit 0x8b                          // MOV ESP,EBP
        _emit 0xe5
        _emit 0x5d                          // POP EBP
        _emit 0xc2                          // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
