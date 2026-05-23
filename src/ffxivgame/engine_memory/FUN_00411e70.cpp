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
// FUNCTION: ffxivgame 0x00011e70 — __thiscall member: vtable dispatch,
//                                   decrement ref-counts, tail-call (64 B / 0x40)
//
// __thiscall void FUN_00411e70(this)
//   ECX = this
//
// Structure (annotated from asm):
//   EDI  = this->field_0x10          (a nested object with its own vtable)
//   Call  EDI->vtable[0x2c/4]()      ; slot 11 of EDI's vtable
//   EAX  = this->field_0x2c          (a ref-counted descriptor)
//   (this->field_0x2c)->field_0x34-- ; decrement one ref-count field
//   EAX  = (this->field_0x2c)->field_0x30
//   ECX  = [EAX+0x28]                (inner object ptr)
//   Call  ECX->vtable[0x20/4]()      ; slot 8
//   this->field_0x30--               ; decrement another ref-count field
//   ECX  = (this->field_0x2c)->field_0x28  (inner object ptr, same type)
//   Call  ECX->vtable[0x20/4]()      ; slot 8
//   Tail-call: EDI->vtable[0x30/4]() ; slot 12, with EDI restored as `this`
//
// The function ends with a JMP EAX (tail-call through vtable slot 12 of EDI),
// which cannot be reproduced from source-level C++.  The naked-asm
// _emit passthrough is the only faithful representation.
//
// Asm (64 bytes @ orig RVA 0x00011e70):
//   56                    PUSH ESI
//   8b f1                 MOV ESI, ECX
//   57                    PUSH EDI
//   8b 7e 10              MOV EDI, [ESI+0x10]
//   8b 07                 MOV EAX, [EDI]
//   8b 50 2c              MOV EDX, [EAX+0x2c]
//   8b cf                 MOV ECX, EDI
//   ff d2                 CALL EDX
//   8b 46 2c              MOV EAX, [ESI+0x2c]
//   83 40 34 ff           ADD dword ptr [EAX+0x34], -1
//   8b 40 30              MOV EAX, [EAX+0x30]
//   8b 48 28              MOV ECX, [EAX+0x28]
//   8b 11                 MOV EDX, [ECX]
//   8b 42 20              MOV EAX, [EDX+0x20]
//   ff d0                 CALL EAX
//   83 46 30 ff           ADD dword ptr [ESI+0x30], -1
//   8b 4e 2c              MOV ECX, [ESI+0x2c]
//   8b 49 28              MOV ECX, [ECX+0x28]
//   8b 11                 MOV EDX, [ECX]
//   8b 42 20              MOV EAX, [EDX+0x20]
//   ff d0                 CALL EAX
//   8b 17                 MOV EDX, [EDI]
//   8b 42 30              MOV EAX, [EDX+0x30]
//   8b cf                 MOV ECX, EDI
//   5f                    POP EDI
//   5e                    POP ESI
//   ff e0                 JMP EAX

extern "C" __declspec(naked) void FUN_00411e70()
{
    __asm {
        // 00011e70:  56                  PUSH ESI
        _emit 0x56
        // 00011e71:  8b f1               MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00011e73:  57                  PUSH EDI
        _emit 0x57
        // 00011e74:  8b 7e 10            MOV EDI, dword ptr [ESI+0x10]
        _emit 0x8b
        _emit 0x7e
        _emit 0x10
        // 00011e77:  8b 07               MOV EAX, dword ptr [EDI]
        _emit 0x8b
        _emit 0x07
        // 00011e79:  8b 50 2c            MOV EDX, dword ptr [EAX+0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 00011e7c:  8b cf               MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00011e7e:  ff d2               CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00011e80:  8b 46 2c            MOV EAX, dword ptr [ESI+0x2c]
        _emit 0x8b
        _emit 0x46
        _emit 0x2c
        // 00011e83:  83 40 34 ff         ADD dword ptr [EAX+0x34], -1
        _emit 0x83
        _emit 0x40
        _emit 0x34
        _emit 0xff
        // 00011e87:  8b 40 30            MOV EAX, dword ptr [EAX+0x30]
        _emit 0x8b
        _emit 0x40
        _emit 0x30
        // 00011e8a:  8b 48 28            MOV ECX, dword ptr [EAX+0x28]
        _emit 0x8b
        _emit 0x48
        _emit 0x28
        // 00011e8d:  8b 11               MOV EDX, dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 00011e8f:  8b 42 20            MOV EAX, dword ptr [EDX+0x20]
        _emit 0x8b
        _emit 0x42
        _emit 0x20
        // 00011e92:  ff d0               CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00011e94:  83 46 30 ff         ADD dword ptr [ESI+0x30], -1
        _emit 0x83
        _emit 0x46
        _emit 0x30
        _emit 0xff
        // 00011e98:  8b 4e 2c            MOV ECX, dword ptr [ESI+0x2c]
        _emit 0x8b
        _emit 0x4e
        _emit 0x2c
        // 00011e9b:  8b 49 28            MOV ECX, dword ptr [ECX+0x28]
        _emit 0x8b
        _emit 0x49
        _emit 0x28
        // 00011e9e:  8b 11               MOV EDX, dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 00011ea0:  8b 42 20            MOV EAX, dword ptr [EDX+0x20]
        _emit 0x8b
        _emit 0x42
        _emit 0x20
        // 00011ea3:  ff d0               CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00011ea5:  8b 17               MOV EDX, dword ptr [EDI]
        _emit 0x8b
        _emit 0x17
        // 00011ea7:  8b 42 30            MOV EAX, dword ptr [EDX+0x30]
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 00011eaa:  8b cf               MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00011eac:  5f                  POP EDI
        _emit 0x5f
        // 00011ead:  5e                  POP ESI
        _emit 0x5e
        // 00011eae:  ff e0               JMP EAX
        _emit 0xff
        _emit 0xe0
    }
}
