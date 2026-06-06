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
// FUNCTION: ffxivgame 0x0001a1f0 — MSVC scalar deleting destructor,
//                                   vtable-set + thiscall-free form (36 B)
//
// Variant of the 36-byte scalar-deleting-dtor family (same shape as
// FUN_00419e40 / FUN_004167e0):
//   - resets vtable at [this+0] before delegating to the real destructor
//   - frees memory via __thiscall FUN_0040df70 with this = *(this-4), i.e.
//     the allocator pointer is stored one slot before the object
//
// Calling convention: __thiscall — ECX = this on entry; one DWORD flag on
// stack (bit 0 = free memory).  Returns: this in EAX.  Callee pops the one
// stack arg (RET 4).
//
// Asm (36 bytes @ orig RVA 0x0001a1f0):
//   56                    PUSH ESI
//   8b f1                 MOV ESI, ECX                ; this
//   c7 06 RR RR RR RR     MOV dword ptr [ESI], vtable ; set vtable (reloc)
//   e8 RR RR RR RR        CALL FUN_00431860            ; real dtor (__thiscall)
//   f6 44 24 08 01        TEST byte ptr [ESP+8], 1
//   74 09                 JZ no_delete
//   8b 4e fc              MOV ECX, dword ptr [ESI-4]   ; allocator = *(this-4)
//   56                    PUSH ESI                    ; arg = this
//   e8 RR RR RR RR        CALL FUN_0040df70            ; __thiscall free (RET 4)
// no_delete:
//   8b c6                 MOV EAX, ESI
//   5e                    POP ESI
//   c2 04 00              RET 4

extern "C" void FUN_00431860();      // real destructor (__thiscall, no stack args)
extern "C" void FUN_0040df70();      // thiscall free, one stack arg (callee RET 4)
extern "C" int g_vtable_1a1f0;       // vtable data at VA 0x0105d59c

extern "C" __declspec(naked) void FUN_0041a1f0(unsigned int) {
    __asm {
        push esi
        mov esi, ecx
        mov dword ptr [esi], offset g_vtable_1a1f0
        call FUN_00431860
        test byte ptr [esp+8], 1
        jz no_delete
        mov ecx, dword ptr [esi-4]
        push esi
        call FUN_0040df70
    no_delete:
        mov eax, esi
        pop esi
        ret 4
    }
}
