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
// FUNCTION: ffxivgame 0x0000f550 — __thiscall 1-stack-arg: store vtable ptr into *this
//                                   (SQEX::CDev::Engine::Memory::IModule ctor stub)
//
// Asm (11 bytes @ orig RVA 0x0000f550):
//   8b c1              MOV EAX, ECX           ; EAX = this (thiscall)
//   c7 00 70 65 f5 00  MOV dword ptr [EAX], 0xf56570 ; *this = IModule::vftable
//   c2 04 00           RET 0x4                ; __thiscall, callee pops 1 stack arg
//
// Stores the IModule vtable pointer at offset 0 of `this`.
// Takes one unused stack argument (cleaned by RET 4).

extern "C" __declspec(naked) void FUN_0040f550() {
    __asm {
        mov eax, ecx
        mov dword ptr [eax], 0x00f56570
        ret 4
    }
}
