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
// FUNCTION: ffxivgame 0x401030 — std::bad_alloc::bad_alloc(char const *const &)
//
// Standard MSVC 2005 STL exception-derived ctor. ECX holds `this`,
// the single stack arg is a `char const *` (passed by reference to
// the std::exception base ctor at RVA 0x5d18da, VA 0x9d18da), and
// after the base call we install std::bad_alloc's vftable (RVA
// 0xb54a10, VA 0xf54a10) into slot 0. The function returns `this`
// in EAX and pops 4 bytes from the caller's stack (thiscall, 1 arg).
//
// Asm (25 bytes):
//   56                  PUSH ESI
//   8d 44 24 08         LEA EAX,[ESP+8]
//   50                  PUSH EAX
//   8b f1               MOV ESI,ECX
//   e8 RR RR RR RR      CALL std::exception::exception
//   c7 06 RR RR RR RR   MOV dword ptr [ESI], offset std::bad_alloc::vftable
//   8b c6               MOV EAX,ESI
//   5e                  POP ESI
//   c2 04 00            RET 4

extern "C" int exception_base_ctor();
extern "C" int bad_alloc_vftable;

extern "C" __declspec(naked) void bad_alloc_ctor() {
    __asm {
        push esi
        lea eax, [esp + 8]
        push eax
        mov esi, ecx
        call exception_base_ctor
        mov dword ptr [esi], offset bad_alloc_vftable
        mov eax, esi
        pop esi
        ret 4
    }
}
