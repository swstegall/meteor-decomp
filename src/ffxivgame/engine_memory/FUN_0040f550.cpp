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
// FUNCTION: ffxivgame 0x0000f550 — __thiscall vtable-install stub
//
// Asm (11 bytes):
//   8b c1              MOV EAX, ECX           ; this → EAX
//   c7 00 70 65 f5 00  MOV dword ptr [EAX], 0xf56570 ; *this = vtbl ptr
//   c2 04 00           RET 0x4                ; __thiscall, 1 stack arg
//
// Stores the vtable pointer 0xf56570 into the object pointed to by ECX
// (this), then returns cleaning one stack argument.

extern "C" __declspec(naked) void FUN_0040f550() {
    __asm {
        // 0000f550:  8b c1              MOV EAX, ECX
        _emit 0x8b
        _emit 0xc1
        // 0000f552:  c7 00 70 65 f5 00  MOV dword ptr [EAX], 0xf56570
        _emit 0xc7
        _emit 0x00
        _emit 0x70
        _emit 0x65
        _emit 0xf5
        _emit 0x00
        // 0000f558:  c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
