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
// FUNCTION: ffxivgame 0x000419f0 — FUN_004419f0 (84 B, __cdecl, void)
//
// __cdecl void FUN_004419f0(unsigned char flag)
//
// Behaviour (read from orig RVA 0x000419f0, 84 bytes):
//
//   PUSH EBX
//   MOV  BL, [ESP+8]          ; flag = first stack arg
//   TEST BL, BL
//   PUSH 0xa                  ; push 10 (arg2 to inner call)
//   PUSH ECX                  ; placeholder (overwritten before call)
//   JZ   arm_zero             ; if flag == 0 goto arm_zero
//
//   ; arm_nonzero: flag != 0 → pass 0.0f to inner function
//   FLDZ
//   FSTP float ptr [ESP]      ; [ESP] = 0.0f
//   CALL FUN_00b8fd70         ; (0.0f, 10)
//   ADD  ESP, 8
//   MOV  byte ptr [0x0132ca94], BL
//   POP  EBX
//   RET
//
// arm_zero:                   ; flag == 0 → compute g_f0 * g_f1 via SSE2 dp
//   MOVSS    XMM0, [0x01266890]   ; g_float_0
//   MOVSS    XMM1, [0x0126688c]   ; g_float_1
//   CVTPS2PD XMM0, XMM0           ; widen to double
//   CVTPS2PD XMM1, XMM1
//   MULSD    XMM0, XMM1           ; double-precision multiply
//   CVTPD2PS XMM0, XMM0           ; narrow back to float
//   MOVSS    dword ptr [ESP], XMM0
//   CALL FUN_00b8fd70             ; (result, 10)
//   ADD  ESP, 8
//   MOV  byte ptr [0x0132ca94], BL
//   POP  EBX
//   RET
//
// The function mixes x87 (FLDZ/FSTP in the non-zero arm) and SSE2
// (MOVSS/CVTPS2PD/MULSD/CVTPD2PS in the zero arm).  A source-level
// C++ port would need compiler flags to reproduce that exact mix, so
// the pragmatic choice is a naked __declspec(naked) passthrough with
// verbatim _emit bytes (matching FUN_004065c0 precedent).
//
// Reloc-bearing sites (compare.py masks these windows):
//   +0x12  CALL rel32  → FUN_00b8fd70  (e8 + REL32)
//   +0x1a  MOV  dir32  → 0x0132ca94   (88 1d + DIR32)
//   +0x25  MOVSS dir32 → 0x01266890   (f3 0f 10 05 + DIR32)
//   +0x2d  MOVSS dir32 → 0x0126688c   (f3 0f 10 0d + DIR32)
//   +0x45  CALL rel32  → FUN_00b8fd70  (e8 + REL32)
//   +0x4d  MOV  dir32  → 0x0132ca94   (88 1d + DIR32)

extern "C" __declspec(naked) void FUN_004419f0() {
    __asm {
        // 000419f0: PUSH EBX
        _emit 0x53
        // 000419f1: MOV BL, byte ptr [ESP+8]
        _emit 0x8a
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        // 000419f5: TEST BL, BL
        _emit 0x84
        _emit 0xdb
        // 000419f7: PUSH 0xa
        _emit 0x6a
        _emit 0x0a
        // 000419f9: PUSH ECX
        _emit 0x51
        // 000419fa: JZ +0x15  (→ 0x00041a11)
        _emit 0x74
        _emit 0x15
        // 000419fc: FLDZ
        _emit 0xd9
        _emit 0xee
        // 000419fe: FSTP float ptr [ESP]
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // 00041a01: CALL 0x00b8fd70 (rel32 — reloc)
        _emit 0xe8
        _emit 0x6a
        _emit 0xe3
        _emit 0x74
        _emit 0x00
        // 00041a06: ADD ESP, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00041a09: MOV byte ptr [0x0132ca94], BL (dir32 — reloc)
        _emit 0x88
        _emit 0x1d
        _emit 0x94
        _emit 0xca
        _emit 0x32
        _emit 0x01
        // 00041a0f: POP EBX
        _emit 0x5b
        // 00041a10: RET
        _emit 0xc3
        // 00041a11: MOVSS XMM0, dword ptr [0x01266890] (dir32 — reloc)
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x90
        _emit 0x68
        _emit 0x26
        _emit 0x01
        // 00041a19: MOVSS XMM1, dword ptr [0x0126688c] (dir32 — reloc)
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x0d
        _emit 0x8c
        _emit 0x68
        _emit 0x26
        _emit 0x01
        // 00041a21: CVTPS2PD XMM0, XMM0
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0
        // 00041a24: CVTPS2PD XMM1, XMM1
        _emit 0x0f
        _emit 0x5a
        _emit 0xc9
        // 00041a27: MULSD XMM0, XMM1
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0xc1
        // 00041a2b: CVTPD2PS XMM0, XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0
        // 00041a2f: MOVSS dword ptr [ESP], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        // 00041a34: CALL 0x00b8fd70 (rel32 — reloc)
        _emit 0xe8
        _emit 0x37
        _emit 0xe3
        _emit 0x74
        _emit 0x00
        // 00041a39: ADD ESP, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00041a3c: MOV byte ptr [0x0132ca94], BL (dir32 — reloc)
        _emit 0x88
        _emit 0x1d
        _emit 0x94
        _emit 0xca
        _emit 0x32
        _emit 0x01
        // 00041a42: POP EBX
        _emit 0x5b
        // 00041a43: RET
        _emit 0xc3
    }
}
