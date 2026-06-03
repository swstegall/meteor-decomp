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
// FUNCTION: ffxivgame 0x00017610 — MSVC scalar deleting destructor, vtable-set form (36 B)
//
// Variant of the 30-byte scalar-deleting-dtor cluster (FUN_00416280) with an
// additional `MOV dword ptr [ESI], vtable_ptr` before the real-destructor call.
// This is the standard MSVC 2005 pattern for a class that stores its own vtable
// in [this+0] before delegating to the base-class real destructor.
//
// Calling convention: __thiscall — ECX = this, one DWORD flag on stack → RET 4.
// Returns: this (EAX = ESI).
//
// Asm (36 bytes @ orig RVA 0x00017610):
//   56                    PUSH ESI
//   8b f1                 MOV ESI, ECX          ; this
//   c7 06 RR RR RR RR     MOV dword ptr [ESI], vtable_ptr   ; set vtable (reloc)
//   e8 RR RR RR RR        CALL FUN_00416990     ; real destructor (thiscall, ECX=ESI)
//   f6 44 24 08 01        TEST byte ptr [ESP+8], 1
//   74 09                 JZ no_delete
//   56                    PUSH ESI
//   e8 RR RR RR RR        CALL FUN_004162c0     ; custom free (cdecl, 1 arg)
//   83 c4 04              ADD ESP, 4
// no_delete:
//   8b c6                 MOV EAX, ESI
//   5e                    POP ESI
//   c2 04 00              RET 4

extern "C" void FUN_00416990();         // real destructor (__thiscall)
extern "C" void FUN_004162c0(void *);   // custom free (__cdecl, 1 arg)
extern "C" int g_vtable_17610;          // vtable data at VA 0x00f5787c

extern "C" __declspec(naked) void FUN_00417610(unsigned int) {
    __asm {
        push esi
        mov esi, ecx
        mov dword ptr [esi], offset g_vtable_17610
        call FUN_00416990
        test byte ptr [esp+8], 1
        jz no_delete
        push esi
        call FUN_004162c0
        add esp, 4
    no_delete:
        mov eax, esi
        pop esi
        ret 4
    }
}
