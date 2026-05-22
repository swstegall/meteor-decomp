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
// FUNCTION: ffxivgame 0x0000f5f0 — __thiscall: double-deref vtable dispatch
//                                   (14 bytes)
//
// Asm (14 bytes @ orig RVA 0x0000f5f0):
//   8b 49 04    MOV ECX, dword ptr [ECX + 0x4]  ; ECX = this->field_0x4 (inner obj)
//   8b 41 04    MOV EAX, dword ptr [ECX + 0x4]  ; EAX = inner->field_0x4 (vtable ptr)
//   8b 50 10    MOV EDX, dword ptr [EAX + 0x10] ; EDX = vtable[4] (function pointer)
//   83 c1 04    ADD ECX, 0x4                     ; ECX = inner + 4 (sub-object this)
//   ff e2       JMP EDX                          ; tail-call through vtable slot
//
// Loads a nested object via this->field_0x4, reads its vtable-like pointer at
// field_0x4, fetches the function at vtable offset 0x10 (slot 4), adjusts ECX
// to the sub-object base (inner + 4), and tail-calls it. No frame, no saved
// registers.

extern "C" __declspec(naked) void FUN_0040f5f0() {
    __asm {
        mov ecx, dword ptr [ecx + 0x4]
        mov eax, dword ptr [ecx + 0x4]
        mov edx, dword ptr [eax + 0x10]
        add ecx, 0x4
        jmp edx
    }
}
