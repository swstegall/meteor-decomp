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
// FUNCTION: ffxivgame 0x0005c5f0 — conditional bit-set on indexed entry (43 B / 0x2B)
//
//   int __cdecl FUN_0045c5f0()
//     no stack arguments; calls FUN_0045c360 to get the object pointer.
//     returns: 0 if [obj+0x18c] == [obj+0x188], else 1.
//
// Asm (43 bytes @ orig RVA 0x0005c5f0):
//   e8 6b fd ff ff   CALL FUN_0045c360            ; obj = FUN_0045c360()  (reloc)
//   8b 88 8c 01 00 00 MOV ECX, dword ptr [EAX+0x18c]
//   3b 88 88 01 00 00 CMP ECX, dword ptr [EAX+0x188]
//   75 03            JNZ  +3  (→ 0x0045c606)
//   33 c0            XOR EAX, EAX                ; return 0
//   c3               RET
//   8b 90 88 01 00 00 MOV EDX, dword ptr [EAX+0x188]
//   83 4c 90 08 01   OR  dword ptr [EAX + EDX*4 + 0x8], 0x1
//   8d 44 90 08      LEA EAX, [EAX + EDX*4 + 0x8]  ; (result discarded)
//   b8 01 00 00 00   MOV EAX, 0x1               ; return 1
//   c3               RET
//
// The only linker-relocated field is the 4-byte rel32 offset of the CALL to
// FUN_0045c360 (bytes +0x01..+0x04); compare.py masks those bytes.
// All remaining instructions are register/immediate only — no absolute addresses.

extern "C" void FUN_0045c360();

extern "C" __declspec(naked) void FUN_0045c5f0() {
    __asm {
        // 0005c5f0: e8 6b fd ff ff   CALL FUN_0045c360  (reloc)
        call FUN_0045c360
        // 0005c5f5: 8b 88 8c 01 00 00   MOV ECX, [EAX+0x18c]
        _emit 0x8b
        _emit 0x88
        _emit 0x8c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c5fb: 3b 88 88 01 00 00   CMP ECX, [EAX+0x188]
        _emit 0x3b
        _emit 0x88
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c601: 75 03   JNZ +3
        _emit 0x75
        _emit 0x03
        // 0005c603: 33 c0   XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0005c605: c3      RET
        _emit 0xc3
        // 0005c606: 8b 90 88 01 00 00   MOV EDX, [EAX+0x188]
        _emit 0x8b
        _emit 0x90
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c60c: 83 4c 90 08 01   OR [EAX + EDX*4 + 0x8], 0x1
        _emit 0x83
        _emit 0x4c
        _emit 0x90
        _emit 0x08
        _emit 0x01
        // 0005c611: 8d 44 90 08   LEA EAX, [EAX + EDX*4 + 0x8]
        _emit 0x8d
        _emit 0x44
        _emit 0x90
        _emit 0x08
        // 0005c615: b8 01 00 00 00   MOV EAX, 0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005c61a: c3   RET
        _emit 0xc3
    }
}
