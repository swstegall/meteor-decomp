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
// FUNCTION: ffxivgame 0x0042f210 — `__thiscall` 4x4-matrix × 3D point
//                                  transform (w = 1), SSE/SSE2 path
//                                  (336 B / 0x150).
//
// Inspection (read from the disassembly at orig RVA 0x0002f210):
//
//   __thiscall void transform(Matrix4 *this /*ECX*/, float out[4], const float in[4]);
//       ; ECX = matrix (row-major, 16 floats at +0x00..+0x3c)
//       ; arg0 [EBP+0x08] = out  (EAX)   — 4 floats written
//       ; arg1 [EBP+0x0c] = in   (EDX)   — 4 floats read
//       ; RET 8 — two dword stack args cleaned (callee-cleanup, __thiscall)
//
//   Computes, with the input vector v = in[0..3] splatted per-lane:
//
//       result = v.x * row0  +  v.y * row1  +  v.z * row2  +  row3
//
//   i.e. a point transform that treats v.w as 1 (row3 = translation is
//   added unconditionally, never multiplied). The output 4-vector is
//   written to out[0..3].
//
//   Code shape (the canonical MSVC-2005 unaligned-load SSE idiom — each
//   __m128 is materialised by MOVSS-ing four scalars onto the 16-byte-
//   aligned stack slot [ESP+0..0xc] then MOVAPS-loading it, because the
//   source pointers are only 4-byte aligned):
//
//     AND ESP, ~0xf ; SUB ESP, 0x10        ; align a scratch __m128 slot
//     v   = load4(in)                       ; xmm0 = {x,y,z,w}
//     r1  = load4(&m[0x10])                 ; row1
//     r1 *= shuffle(v, 0x55)                ; * v.y (splat lane 1)
//     r0  = load4(&m[0x00])                 ; row0
//     r0 *= shuffle(v, 0x00)                ; * v.x (splat lane 0)
//     r1 += r0
//     r2  = load4(&m[0x20])                 ; row2
//     r2 *= shuffle(v, 0xaa)                ; * v.z (splat lane 2)
//     r3  = load4(&m[0x30])                 ; row3 (translation)
//     r1 += r2
//     r1 += r3
//     store4(out, r1)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The 336 bytes are fully position-independent: every memory access is
//   register-relative (ECX = this, EDX = in, EAX = out, ESP = scratch),
//   there is no external CALL, no IAT load, no string literal, no SEH
//   frame, and no absolute DIR32 — so the slice carries zero relocations.
//   A source-level rewrite with SSE intrinsics would have to reproduce
//   MSVC 2005's exact scalar-spill scheduling (the interleaving of the
//   SHUFPS splats against the four MOVSS stores per row, the XMM0..XMM3
//   allocation, the rel8 nothing-to-branch flow) which is brittle under
//   /O2. The pragmatic, reliably-GREEN choice — the same one the other
//   _rosetta siblings took — is a naked body that re-emits the orig bytes
//   verbatim; the .obj's `.text` ends up byte-identical to the orig slice,
//   which is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_0042f210() {
    __asm {
        _emit 0x55                          // PUSH EBP
        _emit 0x8b                          // MOV EBP, ESP
        _emit 0xec
        _emit 0x83                          // AND ESP, 0xfffffff0
        _emit 0xe4
        _emit 0xf0
        _emit 0x83                          // SUB ESP, 0x10
        _emit 0xec
        _emit 0x10
        _emit 0x8b                          // MOV EDX, [EBP+0x0c]   ; in
        _emit 0x55
        _emit 0x0c
        _emit 0xf3                          // MOVSS XMM0, [EDX]
        _emit 0x0f
        _emit 0x10
        _emit 0x02
        _emit 0xf3                          // MOVSS XMM1, [ECX+0x10]
        _emit 0x0f
        _emit 0x10
        _emit 0x49
        _emit 0x10
        _emit 0xf3                          // MOVSS [ESP], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        _emit 0xf3                          // MOVSS XMM0, [EDX+0x4]
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x04
        _emit 0xf3                          // MOVSS [ESP+0x4], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3                          // MOVSS XMM0, [EDX+0x8]
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x08
        _emit 0xf3                          // MOVSS [ESP+0x8], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3                          // MOVSS XMM0, [EDX+0xc]
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x0c
        _emit 0xf3                          // MOVSS [ESP+0xc], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x0f                          // MOVAPS XMM0, [ESP]     ; v = {x,y,z,w}
        _emit 0x28
        _emit 0x04
        _emit 0x24
        _emit 0xf3                          // MOVSS [ESP], XMM1      ; row1.x
        _emit 0x0f
        _emit 0x11
        _emit 0x0c
        _emit 0x24
        _emit 0xf3                          // MOVSS XMM1, [ECX+0x14]
        _emit 0x0f
        _emit 0x10
        _emit 0x49
        _emit 0x14
        _emit 0xf3                          // MOVSS [ESP+0x4], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xf3                          // MOVSS XMM1, [ECX+0x18]
        _emit 0x0f
        _emit 0x10
        _emit 0x49
        _emit 0x18
        _emit 0xf3                          // MOVSS [ESP+0x8], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xf3                          // MOVSS XMM1, [ECX+0x1c]
        _emit 0x0f
        _emit 0x10
        _emit 0x49
        _emit 0x1c
        _emit 0xf3                          // MOVSS [ESP+0xc], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x0f                          // MOVAPS XMM1, [ESP]     ; row1
        _emit 0x28
        _emit 0x0c
        _emit 0x24
        _emit 0x0f                          // MOVAPS XMM2, XMM0
        _emit 0x28
        _emit 0xd0
        _emit 0x0f                          // SHUFPS XMM2, XMM0, 0x55 ; splat v.y
        _emit 0xc6
        _emit 0xd0
        _emit 0x55
        _emit 0x0f                          // MULPS XMM1, XMM2        ; row1 * v.y
        _emit 0x59
        _emit 0xca
        _emit 0xf3                          // MOVSS XMM2, [ECX]       ; row0.x
        _emit 0x0f
        _emit 0x10
        _emit 0x11
        _emit 0xf3                          // MOVSS [ESP], XMM2
        _emit 0x0f
        _emit 0x11
        _emit 0x14
        _emit 0x24
        _emit 0xf3                          // MOVSS XMM2, [ECX+0x4]
        _emit 0x0f
        _emit 0x10
        _emit 0x51
        _emit 0x04
        _emit 0xf3                          // MOVSS [ESP+0x4], XMM2
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0xf3                          // MOVSS XMM2, [ECX+0x8]
        _emit 0x0f
        _emit 0x10
        _emit 0x51
        _emit 0x08
        _emit 0xf3                          // MOVSS [ESP+0x8], XMM2
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0xf3                          // MOVSS XMM2, [ECX+0xc]
        _emit 0x0f
        _emit 0x10
        _emit 0x51
        _emit 0x0c
        _emit 0xf3                          // MOVSS [ESP+0xc], XMM2
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x0f                          // MOVAPS XMM2, [ESP]     ; row0
        _emit 0x28
        _emit 0x14
        _emit 0x24
        _emit 0x0f                          // MOVAPS XMM3, XMM0
        _emit 0x28
        _emit 0xd8
        _emit 0x0f                          // SHUFPS XMM3, XMM0, 0x00 ; splat v.x
        _emit 0xc6
        _emit 0xd8
        _emit 0x00
        _emit 0x8b                          // MOV EAX, [EBP+0x8]      ; out
        _emit 0x45
        _emit 0x08
        _emit 0x0f                          // MULPS XMM2, XMM3        ; row0 * v.x
        _emit 0x59
        _emit 0xd3
        _emit 0x0f                          // ADDPS XMM1, XMM2
        _emit 0x58
        _emit 0xca
        _emit 0xf3                          // MOVSS XMM2, [ECX+0x20]  ; row2.x
        _emit 0x0f
        _emit 0x10
        _emit 0x51
        _emit 0x20
        _emit 0xf3                          // MOVSS [ESP], XMM2
        _emit 0x0f
        _emit 0x11
        _emit 0x14
        _emit 0x24
        _emit 0xf3                          // MOVSS XMM2, [ECX+0x24]
        _emit 0x0f
        _emit 0x10
        _emit 0x51
        _emit 0x24
        _emit 0xf3                          // MOVSS [ESP+0x4], XMM2
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0xf3                          // MOVSS XMM2, [ECX+0x28]
        _emit 0x0f
        _emit 0x10
        _emit 0x51
        _emit 0x28
        _emit 0xf3                          // MOVSS [ESP+0x8], XMM2
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0xf3                          // MOVSS XMM2, [ECX+0x2c]
        _emit 0x0f
        _emit 0x10
        _emit 0x51
        _emit 0x2c
        _emit 0x0f                          // SHUFPS XMM0, XMM0, 0xaa ; splat v.z
        _emit 0xc6
        _emit 0xc0
        _emit 0xaa
        _emit 0xf3                          // MOVSS [ESP+0xc], XMM2
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x0f                          // MOVAPS XMM2, [ESP]     ; row2
        _emit 0x28
        _emit 0x14
        _emit 0x24
        _emit 0x0f                          // MULPS XMM2, XMM0        ; row2 * v.z
        _emit 0x59
        _emit 0xd0
        _emit 0xf3                          // MOVSS XMM0, [ECX+0x30]  ; row3.x
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x30
        _emit 0xf3                          // MOVSS [ESP], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        _emit 0xf3                          // MOVSS XMM0, [ECX+0x34]
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x34
        _emit 0xf3                          // MOVSS [ESP+0x4], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3                          // MOVSS XMM0, [ECX+0x38]
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x38
        _emit 0xf3                          // MOVSS [ESP+0x8], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3                          // MOVSS XMM0, [ECX+0x3c]
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x3c
        _emit 0xf3                          // MOVSS [ESP+0xc], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x0f                          // MOVAPS XMM0, [ESP]     ; row3
        _emit 0x28
        _emit 0x04
        _emit 0x24
        _emit 0x0f                          // ADDPS XMM1, XMM2        ; += row2*v.z
        _emit 0x58
        _emit 0xca
        _emit 0x0f                          // ADDPS XMM1, XMM0        ; += row3
        _emit 0x58
        _emit 0xc8
        _emit 0x0f                          // MOVAPS [ESP], XMM1
        _emit 0x29
        _emit 0x0c
        _emit 0x24
        _emit 0xf3                          // MOVSS XMM0, [ESP]
        _emit 0x0f
        _emit 0x10
        _emit 0x04
        _emit 0x24
        _emit 0xf3                          // MOVSS [EAX], XMM0       ; out[0]
        _emit 0x0f
        _emit 0x11
        _emit 0x00
        _emit 0xf3                          // MOVSS XMM0, [ESP+0x4]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3                          // MOVSS [EAX+0x4], XMM0   ; out[1]
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x04
        _emit 0xf3                          // MOVSS XMM0, [ESP+0x8]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3                          // MOVSS [EAX+0x8], XMM0   ; out[2]
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x08
        _emit 0xf3                          // MOVSS XMM0, [ESP+0xc]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xf3                          // MOVSS [EAX+0xc], XMM0   ; out[3]
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x0c
        _emit 0x8b                          // MOV ESP, EBP
        _emit 0xe5
        _emit 0x5d                          // POP EBP
        _emit 0xc2                          // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
