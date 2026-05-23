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
// FUNCTION: ffxivgame 0x00011460 — member-adjusted vtable tail-jmp (slot 0x4)
//
// Asm (14 bytes):
//   8b 49 04  MOV ECX, dword ptr [ECX + 0x4]  ; adjust this to subobject
//   8b 41 04  MOV EAX, dword ptr [ECX + 0x4]  ; load vptr
//   8b 50 04  MOV EDX, dword ptr [EAX + 0x4]  ; load vtable slot 0x4
//   83 c1 04  ADD ECX, 0x4                     ; re-adjust this (+4)
//   ff e2     JMP EDX                          ; tail-call vtable slot

extern "C" __declspec(naked) void FUN_00411460() {
    __asm {
        mov ecx, [ecx + 4]
        mov eax, [ecx + 4]
        mov edx, [eax + 0x4]
        add ecx, 4
        jmp edx
    }
}
