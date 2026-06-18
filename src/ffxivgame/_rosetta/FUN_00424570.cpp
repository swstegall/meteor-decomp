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
// FUNCTION: ffxivgame 0x00024570 — scalar deleting destructor, vtable-resetting
//                                   (__thiscall, 1 arg, 36 B / 0x24)
//
// void* __thiscall FUN_00424570(this /* ECX */, unsigned int flags)
//
// Sets this->vptr to 0x00f5bda0, calls the destructor body (FUN_004243e0),
// then — if bit 0 of flags is set — frees the object by dispatching
// FUN_0040df70 as __thiscall on *(this - 4) with this as the argument.
// Returns this in EAX.
//
// This is the MSVC 2005 "scalar deleting destructor" pattern:
//   1. Re-seat the vtable to the destructor-class's own table (ensures
//      RTTI/virtual dispatch is correct for the destructor body).
//   2. Call the real destructor body (FUN_004243e0).
//   3. If flags & 1 ("scalar delete"), free the allocation block via the
//      allocator stored just before the object (*(this - 4)).
//
// Contrast with FUN_00424540 (same module, 34 B): that variant is the
// array-deleting form — it skips the vtable reset and adds a null-guard on
// this before freeing. Here there is no null-guard, and the vtable IS set.
//
// Asm (36 bytes @ orig RVA 0x00024570):
//   56                   PUSH ESI
//   8b f1                MOV ESI, ECX                ; this
//   c7 06 a0 bd f5 00    MOV dword ptr [ESI], 0xf5bda0 ; reset vtable
//   e8 RR RR RR RR       CALL FUN_004243e0            ; destructor body
//   f6 44 24 08 01       TEST byte ptr [ESP+0x8], 0x1 ; flags & 1?
//   74 09                JZ +9  → epilogue
//   8b 4e fc             MOV ECX, dword ptr [ESI-0x4] ; allocator this
//   56                   PUSH ESI                     ; arg = this
//   e8 RR RR RR RR       CALL FUN_0040df70            ; free
//   8b c6                MOV EAX, ESI                 ; return this
//   5e                   POP ESI
//   c2 04 00             RET 0x4                      ; __thiscall, pop 1 arg

extern "C" void FUN_004243e0();   // destructor body (__thiscall, this in ECX)
extern "C" void FUN_0040df70();   // deallocation helper (__thiscall, this in ECX, arg on stack)

extern "C" __declspec(naked) void FUN_00424570()
{
    __asm {
        push esi
        mov esi, ecx
        mov dword ptr [esi], 0xf5bda0
        call FUN_004243e0
        test byte ptr [esp+8], 1
        jz no_delete
        mov ecx, dword ptr [esi - 4]
        push esi
        call FUN_0040df70
    no_delete:
        mov eax, esi
        pop esi
        ret 4
    }
}
