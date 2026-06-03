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
// FUNCTION: ffxivgame 0x009c916c — struct field initializer (44 B / 0x2C)
//
// __thiscall member function that takes one int parameter and initialises
// seven consecutive DWORD fields starting at offset 0x4 on the object.
//
//   struct SomeObj {
//     // +0x00  (vtable / something before these fields)
//     int field_04;   // ← param1
//     int field_08;   // ← 0
//     int field_0c;   // ← 0
//     int field_10;   // ← 7
//     int field_14;   // ← 1
//     int field_18;   // ← 0
//     int field_1c;   // ← 1
//   };
//
//   void SomeObj::init(int param1) {  // __thiscall, one arg, RET 4
//       field_04 = param1;
//       field_08 = 0;
//       field_0c = 0;
//       field_10 = 7;
//       field_14 = 1;
//       field_18 = 0;
//       field_1c = 1;
//   }
//
// Calling convention: __thiscall (ECX = this; one 4-byte stack arg;
//   callee cleans via RET 4).
//
// Frame: standard EBP frame, no locals. MSVC 2005 adds MOV EDI,EDI
//   hotpatch NOP at function entry.
//
// Register allocation:
//   EAX — this pointer (saved from ECX early, since ECX is reused)
//   ECX — param1 (loaded from [EBP+0x8]), then repurposed to hold 1
//          via XOR ECX,ECX / INC ECX
//   EDX — zero (via XOR EDX,EDX), used for the zero-stores
//
// Reconstruction: __declspec(naked) _emit byte passthrough. No
//   relocations in this function.
//
// Byte sequence (44 bytes):
//   8b ff          MOV EDI, EDI          (hotpatch NOP)
//   55             PUSH EBP
//   8b ec          MOV EBP, ESP
//   8b c1          MOV EAX, ECX          (save this)
//   8b 4d 08       MOV ECX, [EBP+0x8]   (param1)
//   89 48 04       MOV [EAX+0x04], ECX  (field_04 = param1)
//   33 d2          XOR EDX, EDX         (EDX = 0)
//   33 c9          XOR ECX, ECX         (ECX = 0)
//   41             INC ECX              (ECX = 1)
//   89 50 08       MOV [EAX+0x08], EDX  (field_08 = 0)
//   89 50 0c       MOV [EAX+0x0c], EDX  (field_0c = 0)
//   c7 40 10 07 00 00 00  MOV [EAX+0x10], 7  (field_10 = 7)
//   89 48 14       MOV [EAX+0x14], ECX  (field_14 = 1)
//   89 50 18       MOV [EAX+0x18], EDX  (field_18 = 0)
//   89 48 1c       MOV [EAX+0x1c], ECX  (field_1c = 1)
//   5d             POP EBP
//   c2 04 00       RET 4

extern "C" __declspec(naked) void FUN_00dc916c() {
    __asm {
        _emit 0x8b  // MOV EDI, EDI       (hotpatch NOP)
        _emit 0xff
        _emit 0x55  // PUSH EBP
        _emit 0x8b  // MOV EBP, ESP
        _emit 0xec
        _emit 0x8b  // MOV EAX, ECX      (save this)
        _emit 0xc1
        _emit 0x8b  // MOV ECX, [EBP+0x8]
        _emit 0x4d
        _emit 0x08
        _emit 0x89  // MOV [EAX+0x4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x33  // XOR EDX, EDX
        _emit 0xd2
        _emit 0x33  // XOR ECX, ECX
        _emit 0xc9
        _emit 0x41  // INC ECX
        _emit 0x89  // MOV [EAX+0x8], EDX
        _emit 0x50
        _emit 0x08
        _emit 0x89  // MOV [EAX+0xc], EDX
        _emit 0x50
        _emit 0x0c
        _emit 0xc7  // MOV dword ptr [EAX+0x10], 7
        _emit 0x40
        _emit 0x10
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89  // MOV [EAX+0x14], ECX
        _emit 0x48
        _emit 0x14
        _emit 0x89  // MOV [EAX+0x18], EDX
        _emit 0x50
        _emit 0x18
        _emit 0x89  // MOV [EAX+0x1c], ECX
        _emit 0x48
        _emit 0x1c
        _emit 0x5d  // POP EBP
        _emit 0xc2  // RET 4
        _emit 0x04
        _emit 0x00
    }
}
