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
// FUNCTION: ffxivgame 0x00001030 — std::bad_alloc::bad_alloc(char const * const &)
//
// The 1-arg form of bad_alloc's constructor. Forwards the message
// reference to std::exception::exception(const char* const&) (the
// 78-byte ctor at RVA 0x5d18da) and then patches the vtable to
// std::bad_alloc::vftable (RVA 0xb54a10, VA 0x00f54a10).
//
// Calling convention: __thiscall (ret 4 — one explicit arg).
// Stack frame: no frame; one callee-saved register (esi = this).
//
// Asm (25 bytes):
//   56                push esi
//   8d 44 24 08       lea  eax, [esp+8]          ; address of incoming msg arg
//   50                push eax                    ; pass &msg to base ctor
//   8b f1             mov  esi, ecx               ; this
//   e8 RR RR RR RR    call std::exception::exception
//   c7 06 RR RR RR RR mov  dword ptr [esi], offset std::bad_alloc::vftable
//   8b c6             mov  eax, esi               ; return this
//   5e                pop  esi
//   c2 04 00          ret  4
//
// Companion to FUN_00401060, which is the matching bad_alloc scalar
// deleting destructor (starts by storing the vftable then calling the
// real exception dtor).

extern "C" int std_exception_ctor_ref();
extern "C" int std_bad_alloc_vftable;

extern "C" __declspec(naked) void bad_alloc_ctor_msg() {
    __asm {
        push esi
        lea eax, [esp+8]
        push eax
        mov esi, ecx
        call std_exception_ctor_ref
        mov dword ptr [esi], offset std_bad_alloc_vftable
        mov eax, esi
        pop esi
        ret 4
    }
}
