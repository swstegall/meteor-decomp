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
// FUNCTION: ffxivgame 0x0000dfd0 — __thiscall method; 2 stack args (RET 8).
//
// Saves EBX/EBP/ESI; ESI = this, EBP = &this->field_0x24.
// Calls IAT[0x00f3e16c](&this->field_0x24), loads arg1 into EBX.
// If this->field_0x20 != 0:
//   r1 = (*this->field_0)->vtable[8](arg1)             [thiscall, RETN 4]
//   r2 = (*this->field_0)->vtable[7](arg1, arg2, r1)   [cdecl, no cleanup]
//   ((stdcall fn*)this->field_0x20)(r2)                 [stdcall, RETN 4]
//   ADD ESP, 0xC  — batch-cleans the 3 cdecl leftovers from vtable[7]
// r3 = (*this->field_0)->vtable[3](this, arg1, arg2)   [thiscall, RETN 12]
// Calls IAT[0x00f3e168](&this->field_0x24).
// Returns r3.
//
// Raw byte passthrough (97 bytes, RVA 0x0000dfd0):
//   IAT slots at +0x0b (0x00f3e16c) and +0x53 (0x00f3e168) bear
//   DIR32 relocations that compare.py masks during the diff.

extern "C" __declspec(naked) void FUN_0040dfd0() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8d              // LEA EBP, [ESI+0x24]
        _emit 0x6e
        _emit 0x24
        _emit 0x55              // PUSH EBP
        _emit 0xff              // CALL dword ptr [0x00f3e16c]
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x83              // CMP dword ptr [ESI+0x20], 0
        _emit 0x7e
        _emit 0x20
        _emit 0x00
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x10]
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x74              // JZ +0x27 (skip)
        _emit 0x27
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESI]
        _emit 0x3e
        _emit 0x8b              // MOV EAX, dword ptr [EDI]
        _emit 0x07
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x20]
        _emit 0x50
        _emit 0x20
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV EDX, dword ptr [EDI]
        _emit 0x17
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x1c]
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x1c]
        _emit 0x42
        _emit 0x1c
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x20]
        _emit 0x4e
        _emit 0x20
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL ECX
        _emit 0xd1
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x5f              // POP EDI
                                // skip:
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV ECX, dword ptr [ESI]
        _emit 0x0e
        _emit 0x8b              // MOV EDX, dword ptr [ECX]
        _emit 0x11
        _emit 0x8b              // MOV EDX, dword ptr [EDX+0xc]
        _emit 0x52
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0xff              // CALL dword ptr [0x00f3e168]
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 8
        _emit 0x08
        _emit 0x00
    }
}
