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
// FUNCTION: ffxivgame 0x00014bc0 — __stdcall thunk forwarding to
//                                  __cdecl __aligned_malloc with swapped args
//                                  (21 bytes)
//
// Asm (21 bytes @ orig RVA 0x00014bc0):
//   8b 44 24 04       MOV EAX, dword ptr [ESP + 0x4]   ; eax = param_1 (alignment)
//   8b 4c 24 08       MOV ECX, dword ptr [ESP + 0x8]   ; ecx = param_2 (size)
//   50                PUSH EAX                         ; push alignment (2nd arg)
//   51                PUSH ECX                         ; push size      (1st arg)
//   e8 XX XX XX XX    CALL __aligned_malloc            ; __cdecl, target RVA 0x005d5712
//   83 c4 08          ADD ESP, 0x8                     ; __cdecl caller cleanup
//   c2 08 00          RET 0x8                          ; __stdcall, callee pops 2 dwords
//
// __stdcall wrapper around the CRT-style __aligned_malloc(size, alignment).
// The wrapper takes (alignment, size) on the stack but calls the inner
// function as __aligned_malloc(size, alignment) — note the deliberate
// argument swap. MSVC 2005 loads both args into EAX/ECX before pushing
// rather than `push [esp+8]; push [esp+8]` (the latter would shave 2 bytes).
//
// Relocations (masked by compare.py):
//   REL32: __aligned_malloc at func+11

extern "C" void __cdecl __aligned_malloc(unsigned int size, unsigned int alignment);

extern "C" __declspec(naked) void FUN_00414bc0()
{
    __asm {
        mov eax, dword ptr [esp + 4]
        mov ecx, dword ptr [esp + 8]
        push eax
        push ecx
        call __aligned_malloc
        add esp, 8
        ret 8
    }
}
