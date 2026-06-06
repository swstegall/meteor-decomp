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
// FUNCTION: ffxivgame 0x00410a00 — engine_memory __thiscall method
//                                  (124 B / 0x7c)
//
// __thiscall int FUN_00410a00(void)
//   ECX : this
//
// Shape:
//   EDI = this->field_0x10                  (inner object pointer)
//   EDI->vtable[0x2c]()                     (vtable call — no retval used)
//   bool ok = EDI->vtable[0x34]()           (vtable call — result in AL)
//   EBX = 1
//   if (ok) {
//       if (!(g_flag_01323910 & 1)) {       (lazy init of function pointer)
//           g_flag_01323910 |= 1;
//           g_fnptr_0132390c = 0x0040f8e0;  (log/assert dispatcher)
//       }
//       g_fnptr_0132390c(0xf56974, 0xf54d48, 0xf56988, 0x8c, 0xf569e0);
//   }
//   this->field_0x2c->field_0x34 += 1;
//   this->field_0x30 += 1;
//   EDI->vtable[0x30]()
//   return this->field_0x28 + this->field_0x2c->field_0x18;
//
// Reloc-bearing sites (all absolute addresses baked to the orig image):
//   +0x23   TEST mem8  → 0x01323910  (.data — g_flag_01323910)
//   +0x2b   OR   mem32 → 0x01323910  (.data — g_flag_01323910)
//   +0x31   MOV  mem32 → 0x0132390c, imm32=0x0040f8e0  (.data — g_fnptr)
//   +0x3b   PUSH imm32 → 0xf569e0    (.rdata — string/data literal)
//   +0x40   PUSH imm32 → 0x0000008c  (integer literal)
//   +0x45   PUSH imm32 → 0xf56988    (.rdata)
//   +0x4a   PUSH imm32 → 0xf54d48    (.rdata)
//   +0x4f   PUSH imm32 → 0xf56974    (.rdata)
//   +0x54   CALL mem32 → 0x0132390c  (.data — indirect call via fnptr)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Same idiom as all siblings in this module. The 124 orig bytes are
//   re-emitted verbatim via MASM `_emit` directives so the .obj's .text
//   matches byte-for-byte with no additional relocations.

extern "C" __declspec(naked) void FUN_00410a00() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESI + 0x10]
        _emit 0x7e
        _emit 0x10
        _emit 0x8b              // MOV EAX, dword ptr [EDI]
        _emit 0x07
        _emit 0x8b              // MOV EDX, dword ptr [EAX + 0x2c]
        _emit 0x50
        _emit 0x2c
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV EAX, dword ptr [EDI]
        _emit 0x07
        _emit 0x8b              // MOV EDX, dword ptr [EAX + 0x34]
        _emit 0x50
        _emit 0x34
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0xbb              // MOV EBX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x3a  (→ 0x00410a5d)
        _emit 0x3a
        _emit 0x84              // TEST byte ptr [0x01323910], BL
        _emit 0x1d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ +0x10  (→ 0x00410a3b)
        _emit 0x10
        _emit 0x09              // OR dword ptr [0x01323910], EBX
        _emit 0x1d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [0x0132390c], 0x0040f8e0
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xe0
        _emit 0xf8
        _emit 0x40
        _emit 0x00
        _emit 0x68              // PUSH 0xf569e0
        _emit 0xe0
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x8c
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf56988
        _emit 0x88
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0xf54d48
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0xf56974
        _emit 0x74
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x2c]
        _emit 0x46
        _emit 0x2c
        _emit 0x01              // ADD dword ptr [EAX + 0x34], EBX
        _emit 0x58
        _emit 0x34
        _emit 0x01              // ADD dword ptr [ESI + 0x30], EBX
        _emit 0x5e
        _emit 0x30
        _emit 0x8b              // MOV EAX, dword ptr [EDI]
        _emit 0x07
        _emit 0x8b              // MOV EDX, dword ptr [EAX + 0x30]
        _emit 0x50
        _emit 0x30
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x2c]
        _emit 0x4e
        _emit 0x2c
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x28]
        _emit 0x46
        _emit 0x28
        _emit 0x03              // ADD EAX, dword ptr [ECX + 0x18]
        _emit 0x41
        _emit 0x18
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
