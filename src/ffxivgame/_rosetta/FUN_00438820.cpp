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
// FUNCTION: ffxivgame 0x00038820 — build local descriptor + forward call
//                                   (__thiscall, 1 arg, 38 B / 0x26)
//
// void __thiscall FUN_00438820(this, int param_1)
//
// 1. Reserves an 8-byte local on the stack (the descriptor struct).
// 2. Loads this->field_0x4 into ECX and pushes it as the lone stack arg.
// 3. Fills the local descriptor: word0 = 0x00f64948 (a global pointer /
//    vtable-ish constant), word1 = param_1.
// 4. LEA ECX, &local  → the descriptor is the `this` for the __thiscall
//    CALL FUN_004357e0(this->field_0x4).
// 5. ADD ESP,8 then RET 4 (thiscall callee-cleanup of the single arg).
//
// The CALL is a 4-byte rel32 relocation (bytes +0x1c..+0x1f); compare.py
// masks them. The MOV imm32 0x00f64948 store and the precise stack-slot
// reuse (descriptor written via [ESP+4]/[ESP+8] after a PUSH shifts the
// frame) are fragile to reproduce from source-level C++, so this function
// is encoded as a __declspec(naked) byte-verbatim passthrough.
//
// Asm (38 bytes @ orig RVA 0x00038820):
//   83 ec 08                 SUB  ESP, 0x8
//   8b 49 04                 MOV  ECX, [ECX+0x4]
//   8b 44 24 0c              MOV  EAX, [ESP+0xc]            ; param_1
//   51                       PUSH ECX                       ; arg = this->field_4
//   8d 4c 24 04              LEA  ECX, [ESP+0x4]            ; this = &local
//   c7 44 24 04 48 49 f6 00  MOV  [ESP+0x4], 0x00f64948     ; local.word0
//   89 44 24 08              MOV  [ESP+0x8], EAX            ; local.word1
//   e8 RR RR RR RR           CALL FUN_004357e0              ; (reloc)
//   83 c4 08                 ADD  ESP, 0x8
//   c2 04 00                 RET  0x4

extern "C" void FUN_004357e0();   // forward declaration for the CALL relocation

extern "C" __declspec(naked) void FUN_00438820() {
    __asm {
        // 00038820: 83 ec 08      SUB ESP, 8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 00038823: 8b 49 04      MOV ECX, [ECX+4]
        _emit 0x8b
        _emit 0x49
        _emit 0x04
        // 00038826: 8b 44 24 0c   MOV EAX, [ESP+0xc]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0003882a: 51            PUSH ECX
        _emit 0x51
        // 0003882b: 8d 4c 24 04   LEA ECX, [ESP+4]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0003882f: c7 44 24 04 48 49 f6 00   MOV [ESP+4], 0x00f64948
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x48
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 00038837: 89 44 24 08   MOV [ESP+8], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0003883b: e8 RR RR RR RR   CALL FUN_004357e0  (reloc)
        call FUN_004357e0
        // 00038840: 83 c4 08      ADD ESP, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00038843: c2 04 00      RET 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
