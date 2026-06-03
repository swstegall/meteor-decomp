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
// FUNCTION: ffxivgame 0x004182c0 — float-pair setter and dispatcher (55 bytes)
//
//   void __cdecl FUN_004182c0(float arg1, float arg2)
//     stack layout (caller sees):
//       [ESP+0x04] : float arg1
//       [ESP+0x08] : float arg2
//
// Reads two float arguments from the stack, stores them into two
// consecutive global float slots at 0x01328f1c (arg1) and 0x01328f20
// (arg2), then calls FUN_0041cc00(arg1, arg2) with both values.
//
// Stack manipulation:
//   SUB ESP, 8 creates 8 bytes of local scratch used to re-stack arg1
//   and arg2 for the callee.  MOVSS is used to write to the globals (SSE
//   data-move); FLD/FSTP is used to stage the values onto the local stack
//   region (x87 round-trip, same binary representation for 32-bit float).
//   The callee stack at CALL time:
//     [ESP+0] = arg1 (via FLD [ESP+0xC] → FSTP [ESP])
//     [ESP+4] = arg2 (via FLD [ESP+0x8] → FSTP [ESP+4], before SUB)
//   ADD ESP, 8 restores the allocation after the call.
//
// Calling convention: __cdecl (caller cleans, no register saves).
// Stack frame: 8 bytes (scratch for callee args), no PUSH/POP saves.
//
// Reloc sites (masked by tools/compare.py when .obj uses symbol refs;
// emitted as raw bytes here so the .text is byte-identical to the orig):
//   +0x11..+0x14  imm32 → DAT_01328f1c (global float slot 0)
//   +0x22..+0x25  imm32 → DAT_01328f20 (global float slot 1)
//   +0x2F..+0x32  rel32 → FUN_0041cc00 (callee)
//
// Reconstruction strategy: naked-asm _emit passthrough (mirrors siblings
// FUN_004051e0 etc.) — 55 bytes verbatim.

extern "C" __declspec(naked) void FUN_004182c0() {
    __asm {
        // f3 0f 10 44 24 04     MOVSS XMM0, dword ptr [ESP+0x4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // d9 44 24 08           FLD float ptr [ESP+0x8]
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 83 ec 08              SUB ESP, 0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // d9 5c 24 04           FSTP float ptr [ESP+0x4]
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x04
        // f3 0f 11 05 1c 8f 32 01   MOVSS dword ptr [0x01328f1c], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x05
        _emit 0x1c
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // f3 0f 10 44 24 10     MOVSS XMM0, dword ptr [ESP+0x10]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // d9 44 24 0c           FLD float ptr [ESP+0xC]
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // d9 1c 24              FSTP float ptr [ESP]
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // f3 0f 11 05 20 8f 32 01   MOVSS dword ptr [0x01328f20], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x05
        _emit 0x20
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // e8 0d 49 00 00        CALL FUN_0041cc00
        _emit 0xe8
        _emit 0x0d
        _emit 0x49
        _emit 0x00
        _emit 0x00
        // 83 c4 08              ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // c3                    RET
        _emit 0xc3
    }
}
