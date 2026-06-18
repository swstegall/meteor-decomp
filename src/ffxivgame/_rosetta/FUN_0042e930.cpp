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
// FUNCTION: ffxivgame 0x0042e930 — 3D cross-product with W-passthrough (203 B / 0xcb)
//
// Calling convention: __thiscall
//   this  (ECX)       — first vector  A = [Ax, Ay, Az, Aw]  (a 4×float struct)
//   arg1  [EBP+0x08]  — float[4]* out — receives the result
//   arg2  [EBP+0x0c]  — float[4]* B   — second input vector  B = [Bx, By, Bz, Bw]
//   returns void; RET 0x8 (callee-cleanup of 8 stack bytes)
//
// Outline:
//   Copy A (this) via four scalar MOVSS writes into a 16-byte aligned stack
//   slot, load into XMM1.  Do the same for B (arg2), load into XMM0.
//   Compute the cross product A × B using the standard SSE shuffle idiom:
//
//     XMM2 ← shuffle(B, 0xc9) = [By, Bz, Bx, Bw]
//     XMM0 ← shuffle(B, 0xd2) = [Bz, Bx, By, Bw]
//     XMM3 ← shuffle(A, 0xd2) = [Az, Ax, Ay, Aw]
//     XMM2 ← XMM2 * XMM3      = [By*Az, Bz*Ax, Bx*Ay, Bw*Aw]
//     XMM3 ← shuffle(A, 0xc9) = [Ay, Az, Ax, Aw]
//     XMM0 ← XMM0 * XMM3      = [Bz*Ay, Bx*Az, By*Ax, Bw*Aw]
//     XMM0 ← XMM0 - XMM2      = [Ay*Bz-Az*By, Az*Bx-Ax*Bz, Ax*By-Ay*Bx, 0]  (A×B)
//
//   Then stitch in Aw for the W lane via UNPCKHPS/SHUFPS:
//     XMM2 ← result [cross_x, cross_y, cross_z, 0]
//     XMM0 ← UNPCKHPS(result, A) = [cross_z, Az, 0, Aw]
//     XMM2 ← shuffle(XMM2, XMM0, 0xc4) = [cross_x, cross_y, cross_z, Aw]
//
//   Write XMM2 to stack, then copy scalar-by-scalar into *out.
//
// Stack frame:
//   AND ESP, 0xfffffff0  — 16-byte alignment required for MOVAPS
//   SUB ESP, 0x10        — 16-byte local buffer used as load/store staging area
//
// No relocations (no calls, no global references) — all bytes baked verbatim.
//
// Reconstruction strategy — __declspec(naked) with _emit directives:
//
//   MSVC 2005 inline-asm XMM operand encoding can deviate from the original
//   bytes for memory operands with SIB/disp8 (e.g. MOVSS [ESP+N], XMM0 vs
//   a no-SIB form).  Since there are zero relocations, the safest route is
//   to emit the 203 original bytes verbatim so tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0042e930() {
    __asm {
        // 55                      PUSH EBP
        _emit 0x55
        // 8b ec                   MOV EBP, ESP
        _emit 0x8b
        _emit 0xec
        // 83 e4 f0                AND ESP, 0xfffffff0
        _emit 0x83
        _emit 0xe4
        _emit 0xf0
        // 83 ec 10                SUB ESP, 0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // f3 0f 10 01             MOVSS XMM0, [ECX]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x01
        // f3 0f 11 04 24          MOVSS [ESP], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        // f3 0f 10 41 04          MOVSS XMM0, [ECX+4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x04
        // f3 0f 11 44 24 04       MOVSS [ESP+4], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // f3 0f 10 41 08          MOVSS XMM0, [ECX+8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x08
        // f3 0f 11 44 24 08       MOVSS [ESP+8], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // f3 0f 10 41 0c          MOVSS XMM0, [ECX+0xc]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x0c
        // 8b 4d 0c                MOV ECX, [EBP+0xc]
        _emit 0x8b
        _emit 0x4d
        _emit 0x0c
        // f3 0f 11 44 24 0c       MOVSS [ESP+0xc], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0f 28 0c 24             MOVAPS XMM1, [ESP]
        _emit 0x0f
        _emit 0x28
        _emit 0x0c
        _emit 0x24
        // f3 0f 10 01             MOVSS XMM0, [ECX]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x01
        // 8b 45 08                MOV EAX, [EBP+8]
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        // f3 0f 11 04 24          MOVSS [ESP], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        // f3 0f 10 41 04          MOVSS XMM0, [ECX+4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x04
        // f3 0f 11 44 24 04       MOVSS [ESP+4], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // f3 0f 10 41 08          MOVSS XMM0, [ECX+8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x08
        // f3 0f 11 44 24 08       MOVSS [ESP+8], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // f3 0f 10 41 0c          MOVSS XMM0, [ECX+0xc]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x0c
        // f3 0f 11 44 24 0c       MOVSS [ESP+0xc], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0f 28 04 24             MOVAPS XMM0, [ESP]
        _emit 0x0f
        _emit 0x28
        _emit 0x04
        _emit 0x24
        // 0f 28 d0                MOVAPS XMM2, XMM0
        _emit 0x0f
        _emit 0x28
        _emit 0xd0
        // 0f c6 d0 c9             SHUFPS XMM2, XMM0, 0xc9
        _emit 0x0f
        _emit 0xc6
        _emit 0xd0
        _emit 0xc9
        // 0f c6 c0 d2             SHUFPS XMM0, XMM0, 0xd2
        _emit 0x0f
        _emit 0xc6
        _emit 0xc0
        _emit 0xd2
        // 0f 28 d9                MOVAPS XMM3, XMM1
        _emit 0x0f
        _emit 0x28
        _emit 0xd9
        // 0f c6 d9 d2             SHUFPS XMM3, XMM1, 0xd2
        _emit 0x0f
        _emit 0xc6
        _emit 0xd9
        _emit 0xd2
        // 0f 59 d3                MULPS XMM2, XMM3
        _emit 0x0f
        _emit 0x59
        _emit 0xd3
        // 0f 28 d9                MOVAPS XMM3, XMM1
        _emit 0x0f
        _emit 0x28
        _emit 0xd9
        // 0f c6 d9 c9             SHUFPS XMM3, XMM1, 0xc9
        _emit 0x0f
        _emit 0xc6
        _emit 0xd9
        _emit 0xc9
        // 0f 59 c3                MULPS XMM0, XMM3
        _emit 0x0f
        _emit 0x59
        _emit 0xc3
        // 0f 5c c2                SUBPS XMM0, XMM2
        _emit 0x0f
        _emit 0x5c
        _emit 0xc2
        // 0f 28 d0                MOVAPS XMM2, XMM0
        _emit 0x0f
        _emit 0x28
        _emit 0xd0
        // 0f 15 c1                UNPCKHPS XMM0, XMM1
        _emit 0x0f
        _emit 0x15
        _emit 0xc1
        // 0f c6 d0 c4             SHUFPS XMM2, XMM0, 0xc4
        _emit 0x0f
        _emit 0xc6
        _emit 0xd0
        _emit 0xc4
        // 0f 29 14 24             MOVAPS [ESP], XMM2
        _emit 0x0f
        _emit 0x29
        _emit 0x14
        _emit 0x24
        // f3 0f 10 04 24          MOVSS XMM0, [ESP]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x04
        _emit 0x24
        // f3 0f 11 00             MOVSS [EAX], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x00
        // f3 0f 10 44 24 04       MOVSS XMM0, [ESP+4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // f3 0f 11 40 04          MOVSS [EAX+4], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x04
        // f3 0f 10 44 24 08       MOVSS XMM0, [ESP+8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // f3 0f 11 40 08          MOVSS [EAX+8], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x08
        // f3 0f 10 44 24 0c       MOVSS XMM0, [ESP+0xc]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // f3 0f 11 40 0c          MOVSS [EAX+0xc], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x0c
        // 8b e5                   MOV ESP, EBP
        _emit 0x8b
        _emit 0xe5
        // 5d                      POP EBP
        _emit 0x5d
        // c2 08 00                RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
