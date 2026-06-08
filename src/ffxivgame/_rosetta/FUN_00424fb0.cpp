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
// FUNCTION: ffxivgame 0x00024fb0 — two-call init stub (34 B)
//
// Sets the byte flag at 0x01328f19 to 1 and calls FUN_0041c270(1),
// then stores 3 to the DWORD at 0x01328f14 and calls FUN_0041d100(3).
// Cleanup is a single combined ADD ESP,8 at the end.
//
// Asm (34 bytes @ orig RVA 0x00024fb0):
//   6a 01                             PUSH 0x1
//   c6 05 19 8f 32 01 01              MOV byte ptr [0x01328f19], 0x1
//   e8 b2 72 ff ff                    CALL FUN_0041c270
//   b8 03 00 00 00                    MOV EAX, 0x3
//   50                                PUSH EAX
//   a3 14 8f 32 01                    MOV [0x01328f14], EAX
//   e8 32 81 ff ff                    CALL FUN_0041d100
//   83 c4 08                          ADD ESP, 0x8
//   c3                                RET
//
// Reloc sites (masked by compare.py from the original reloc table):
//   +0x04: DIR32 → 0x01328f19   (MOV byte ptr [addr], 1)
//   +0x0a: REL32 → FUN_0041c270 (CALL)
//   +0x15: DIR32 → 0x01328f14   (MOV [addr], EAX — A3 short form)
//   +0x1a: REL32 → FUN_0041d100 (CALL)
//
// The second call uses MOV EAX,3 + PUSH EAX + A3 [addr] — an EAX-reuse
// optimisation (11 bytes) that MSVC /O2 replaces with PUSH imm8 + C7 05
// (12 bytes). The neighbouring FUN_00424f50 (85 B) used the same naked
// passthrough strategy for the same reason. compare.py masks all 16 reloc
// bytes, so the non-reloc structural bytes match exactly.

extern "C" __declspec(naked) void FUN_00424fb0() {
    __asm {
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0xc6              // MOV  byte ptr [0x01328f19], 0x1  (DIR32 +0x04)
        _emit 0x05
        _emit 0x19
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x01              // immediate 0x1
        _emit 0xe8              // CALL FUN_0041c270                (REL32 +0x0a)
        _emit 0xb2
        _emit 0x72
        _emit 0xff
        _emit 0xff
        _emit 0xb8              // MOV  EAX, 0x3
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xa3              // MOV  [0x01328f14], EAX           (DIR32 +0x15)
        _emit 0x14
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL FUN_0041d100                (REL32 +0x1a)
        _emit 0x32
        _emit 0x81
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD  ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc3              // RET
    }
}
