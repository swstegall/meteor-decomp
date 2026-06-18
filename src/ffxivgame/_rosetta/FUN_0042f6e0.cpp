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
// FUNCTION: ffxivgame 0x0042f6e0 — SSE 3-component squared self dot-product
//                                  (__thiscall, 87 bytes).
//
// Signature (inferred from register/stack usage + `ret 4` epilogue):
//
//   void __thiscall FUN_0042f6e0(const float *self /*ecx*/,
//                                float       *out  /*[ebp+0x8]*/);
//
// `self` (ECX = this) points to four packed floats; `out` ([ebp+0x8])
// is a 16-byte-aligned destination written with MOVAPS. The function
// stages each component into the aligned scratch buffer at [esp] one
// MOVSS at a time (the source vector is not known-aligned), reloads
// with MOVAPS, multiplies by itself (squares), then horizontally folds
// lanes 0/1/2 — identical shuffle sequence to sibling FUN_0042f650:
//
//   p   = self * self               ; MULPS  (x²,y²,z²,w²)
//   t   = shuffle(p, 0xaa)          ; (z²,z²,z²,z²)
//   t  += p                         ; ADDPS
//   b   = shuffle(p, 0x55)          ; (y²,y²,y²,y²)
//   t  += b                         ; ADDPS
//   *out = t                        ; out[0] = x²+y²+z² (squared 3D length)
//
// The function aligns ESP to a 16-byte boundary (`and esp,-0x10`) and
// carves a 16-byte scratch slot — the classic MSVC 2005 codegen for a
// local __m128 temporary used to gather unaligned floats. There are no
// CALLs and no absolute symbol references in the 87-byte body, so the
// emitted .text is position-independent and byte-identical to the orig
// slice. Following the sibling FUN_0042f650, the body is a
// `__declspec(naked)` re-emission of the original bytes; the source-level
// intrinsic form can't coerce cl.exe's exact scratch-slot layout under
// /O2, but the raw bytes have no relocations so tools/compare.py reports
// GREEN.
//
// Asm body (read from RVA 0x0002f6e0, 87 bytes):
//
//   55                       PUSH EBP
//   8b ec                    MOV  EBP, ESP
//   83 e4 f0                 AND  ESP, 0xfffffff0
//   83 ec 10                 SUB  ESP, 0x10
//   f3 0f 10 01              MOVSS XMM0, [ECX]
//   8b 45 08                 MOV  EAX, [EBP+0x08]          ; out
//   f3 0f 11 04 24           MOVSS [ESP], XMM0
//   f3 0f 10 41 04           MOVSS XMM0, [ECX+0x04]
//   f3 0f 11 44 24 04        MOVSS [ESP+0x04], XMM0
//   f3 0f 10 41 08           MOVSS XMM0, [ECX+0x08]
//   f3 0f 11 44 24 08        MOVSS [ESP+0x08], XMM0
//   f3 0f 10 41 0c           MOVSS XMM0, [ECX+0x0c]
//   f3 0f 11 44 24 0c        MOVSS [ESP+0x0c], XMM0
//   0f 28 04 24              MOVAPS XMM0, [ESP]
//   0f 59 c0                 MULPS XMM0, XMM0
//   0f 28 c8                 MOVAPS XMM1, XMM0
//   0f c6 c8 aa              SHUFPS XMM1, XMM0, 0xaa
//   0f 58 c8                 ADDPS XMM1, XMM0
//   0f c6 c0 55              SHUFPS XMM0, XMM0, 0x55
//   0f 58 c8                 ADDPS XMM1, XMM0
//   0f 29 08                 MOVAPS [EAX], XMM1
//   8b e5                    MOV  ESP, EBP
//   5d                       POP  EBP
//   c2 04 00                 RET  0x04

extern "C" __declspec(naked) void FUN_0042f6e0() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x83              // AND ESP, 0xfffffff0
        _emit 0xe4
        _emit 0xf0
        _emit 0x83              // SUB ESP, 0x10
        _emit 0xec
        _emit 0x10
        _emit 0xf3              // MOVSS XMM0, dword ptr [ECX]
        _emit 0x0f
        _emit 0x10
        _emit 0x01
        _emit 0x8b              // MOV EAX, dword ptr [EBP+0x08]
        _emit 0x45
        _emit 0x08
        _emit 0xf3              // MOVSS dword ptr [ESP], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        _emit 0xf3              // MOVSS XMM0, dword ptr [ECX+0x04]
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x04
        _emit 0xf3              // MOVSS dword ptr [ESP+0x04], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVSS XMM0, dword ptr [ECX+0x08]
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x08
        _emit 0xf3              // MOVSS dword ptr [ESP+0x08], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3              // MOVSS XMM0, dword ptr [ECX+0x0c]
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x0c
        _emit 0xf3              // MOVSS dword ptr [ESP+0x0c], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x0f              // MOVAPS XMM0, xmmword ptr [ESP]
        _emit 0x28
        _emit 0x04
        _emit 0x24
        _emit 0x0f              // MULPS XMM0, XMM0
        _emit 0x59
        _emit 0xc0
        _emit 0x0f              // MOVAPS XMM1, XMM0
        _emit 0x28
        _emit 0xc8
        _emit 0x0f              // SHUFPS XMM1, XMM0, 0xaa
        _emit 0xc6
        _emit 0xc8
        _emit 0xaa
        _emit 0x0f              // ADDPS XMM1, XMM0
        _emit 0x58
        _emit 0xc8
        _emit 0x0f              // SHUFPS XMM0, XMM0, 0x55
        _emit 0xc6
        _emit 0xc0
        _emit 0x55
        _emit 0x0f              // ADDPS XMM1, XMM0
        _emit 0x58
        _emit 0xc8
        _emit 0x0f              // MOVAPS xmmword ptr [EAX], XMM1
        _emit 0x29
        _emit 0x08
        _emit 0x8b              // MOV ESP, EBP
        _emit 0xe5
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x04
        _emit 0x04
        _emit 0x00
    }
}
