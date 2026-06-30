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
// FUNCTION: ffxivgame 0x00069f20 — single-arg __cdecl field-copy helper (23 B)
//
// Takes a pointer to a struct as arg1. Reads a sub-pointer from arg1->field_0x24,
// masks the low 4 bits of that inner struct's field_0x14, OR-assigns them into
// arg1->field_0x14, then copies the inner struct's field_0x18 verbatim into
// arg1->field_0x18.
//
// Asm (23 bytes @ orig RVA 0x00069f20):
//   8b 44 24 04   MOV EAX, dword ptr [ESP+0x4]   ; EAX = arg1
//   8b 48 24      MOV ECX, dword ptr [EAX+0x24]  ; ECX = arg1->field_0x24 (ptr)
//   8b 51 14      MOV EDX, dword ptr [ECX+0x14]  ; EDX = ECX->field_0x14
//   83 e2 0f      AND EDX, 0xf                   ; mask low 4 bits
//   09 50 14      OR  dword ptr [EAX+0x14], EDX  ; arg1->field_0x14 |= EDX
//   8b 49 18      MOV ECX, dword ptr [ECX+0x18]  ; ECX = inner->field_0x18
//   89 48 18      MOV dword ptr [EAX+0x18], ECX  ; arg1->field_0x18 = ECX
//   c3            RET
//
// Calling convention: __cdecl (single pointer arg on stack, bare RET).
// Stack frame: none (leaf function, /Oy frame-pointer omission).

extern "C" __declspec(naked) void FUN_00469f20() {
    __asm {
        mov eax, dword ptr [esp + 4]
        mov ecx, dword ptr [eax + 0x24]
        mov edx, dword ptr [ecx + 0x14]
        and edx, 0xf
        or  dword ptr [eax + 0x14], edx
        mov ecx, dword ptr [ecx + 0x18]
        mov dword ptr [eax + 0x18], ecx
        ret
    }
}
