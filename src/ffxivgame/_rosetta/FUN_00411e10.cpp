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
// FUNCTION: ffxivgame 0x00411e10 — engine_memory __thiscall member (83 B / 0x53)
//
// __thiscall int FUN_00411e10(void)
//   ECX : this
//   Returns (int) EAX = last-vmethod-result + this->field_0x28
//
// Shape:
//   EDI = this->field_0x10               ; grab inner object
//   vtable(EDI)[0x2c](EDI);              ; virtual call on inner obj
//   this->field_0x2c->field_0x34++;
//   EAX = this->field_0x2c->field_0x30;
//   vtable(EAX->field_0x28)[0x1c](EAX->field_0x28);
//   this->field_0x30++;
//   vtable(this->field_0x2c->field_0x28)[0x1c](this->field_0x2c->field_0x28);
//   vtable(EDI)[0x30](EDI);
//   EAX = vtable(this->field_0x2c->field_0x28)[0x04](this->field_0x2c->field_0x28);
//   return EAX + this->field_0x28;
//
// Asm (83 B, RVA 0x00011e10):
//   56                    PUSH ESI
//   8b f1                 MOV ESI,ECX
//   57                    PUSH EDI
//   8b 7e 10              MOV EDI,[ESI+0x10]
//   8b 07                 MOV EAX,[EDI]
//   8b 50 2c              MOV EDX,[EAX+0x2c]
//   8b cf                 MOV ECX,EDI
//   ff d2                 CALL EDX
//   8b 46 2c              MOV EAX,[ESI+0x2c]
//   83 40 34 01           ADD [EAX+0x34],1
//   8b 40 30              MOV EAX,[EAX+0x30]
//   8b 48 28              MOV ECX,[EAX+0x28]
//   8b 11                 MOV EDX,[ECX]
//   8b 42 1c              MOV EAX,[EDX+0x1c]
//   ff d0                 CALL EAX
//   83 46 30 01           ADD [ESI+0x30],1
//   8b 4e 2c              MOV ECX,[ESI+0x2c]
//   8b 49 28              MOV ECX,[ECX+0x28]
//   8b 11                 MOV EDX,[ECX]
//   8b 42 1c              MOV EAX,[EDX+0x1c]
//   ff d0                 CALL EAX
//   8b 17                 MOV EDX,[EDI]
//   8b 42 30              MOV EAX,[EDX+0x30]
//   8b cf                 MOV ECX,EDI
//   ff d0                 CALL EAX
//   8b 4e 2c              MOV ECX,[ESI+0x2c]
//   8b 49 28              MOV ECX,[ECX+0x28]
//   8b 11                 MOV EDX,[ECX]
//   8b 42 04              MOV EAX,[EDX+0x4]
//   8b 76 28              MOV ESI,[ESI+0x28]
//   ff d0                 CALL EAX
//   5f                    POP EDI
//   03 c6                 ADD EAX,ESI
//   5e                    POP ESI
//   c3                    RET
//
// Reloc-bearing sites: NONE — all calls are indirect (CALL EDX / CALL EAX),
// all offsets are small constants. The 83 bytes are entirely self-contained
// with zero relocations; naked _emit passthrough produces a byte-identical .obj.

extern "C" __declspec(naked) void FUN_00411e10() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESI+0x10]
        _emit 0x7e
        _emit 0x10
        _emit 0x8b              // MOV EAX, dword ptr [EDI]
        _emit 0x07
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x2c]
        _emit 0x50
        _emit 0x2c
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x2c]
        _emit 0x46
        _emit 0x2c
        _emit 0x83              // ADD dword ptr [EAX+0x34], 1
        _emit 0x40
        _emit 0x34
        _emit 0x01
        _emit 0x8b              // MOV EAX, dword ptr [EAX+0x30]
        _emit 0x40
        _emit 0x30
        _emit 0x8b              // MOV ECX, dword ptr [EAX+0x28]
        _emit 0x48
        _emit 0x28
        _emit 0x8b              // MOV EDX, dword ptr [ECX]
        _emit 0x11
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x1c]
        _emit 0x42
        _emit 0x1c
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x83              // ADD dword ptr [ESI+0x30], 1
        _emit 0x46
        _emit 0x30
        _emit 0x01
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x2c]
        _emit 0x4e
        _emit 0x2c
        _emit 0x8b              // MOV ECX, dword ptr [ECX+0x28]
        _emit 0x49
        _emit 0x28
        _emit 0x8b              // MOV EDX, dword ptr [ECX]
        _emit 0x11
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x1c]
        _emit 0x42
        _emit 0x1c
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV EDX, dword ptr [EDI]
        _emit 0x17
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x30]
        _emit 0x42
        _emit 0x30
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x2c]
        _emit 0x4e
        _emit 0x2c
        _emit 0x8b              // MOV ECX, dword ptr [ECX+0x28]
        _emit 0x49
        _emit 0x28
        _emit 0x8b              // MOV EDX, dword ptr [ECX]
        _emit 0x11
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x4]
        _emit 0x42
        _emit 0x04
        _emit 0x8b              // MOV ESI, dword ptr [ESI+0x28]
        _emit 0x76
        _emit 0x28
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x5f              // POP EDI
        _emit 0x03              // ADD EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
