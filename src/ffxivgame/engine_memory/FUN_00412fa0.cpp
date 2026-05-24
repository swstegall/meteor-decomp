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
// FUNCTION: ffxivgame 0x00012fa0 — __thiscall return-self wrapper (14 bytes)
//
// Asm (14 bytes @ orig RVA 0x00012fa0):
//   56                  PUSH ESI
//   8B F1               MOV ESI, ECX           ; cache 'this' in callee-save
//   E8 C8 F6 FF FF      CALL FUN_00412670       ; __thiscall, ECX = this (REL32 reloc)
//   8B C6               MOV EAX, ESI           ; return this
//   5E                  POP ESI
//   C2 04 00            RET 4                  ; __thiscall, pop 1 stack arg
//
// MSVC 2005 pattern: member function that takes one unused stack argument,
// calls a sibling __thiscall method (FUN_00412670) with ECX = this, and
// returns this in EAX. ECX is not clobbered by the MOV so the inner CALL
// still sees the correct this pointer.

extern "C" void FUN_00412670();

extern "C" __declspec(naked) void FUN_00412fa0()
{
    __asm {
        push esi
        mov esi, ecx
        call FUN_00412670
        mov eax, esi
        pop esi
        ret 4
    }
}
