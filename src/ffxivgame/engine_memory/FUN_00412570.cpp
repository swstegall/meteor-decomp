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
// FUNCTION: ffxivgame 0x00012570 — __thiscall: single-deref vtable dispatch
//                                   (10 bytes)
//
// Asm (10 bytes @ orig RVA 0x00012570):
//   8b 49 04    MOV ECX, dword ptr [ECX + 0x4]  ; ECX = this->field_0x4 (inner obj)
//   8b 01       MOV EAX, dword ptr [ECX]         ; EAX = inner->vtable (vtable ptr at slot 0)
//   8b 50 04    MOV EDX, dword ptr [EAX + 0x4]  ; EDX = vtable[1] (function pointer)
//   ff e2       JMP EDX                          ; tail-call through vtable slot
//
// Loads a nested object via this->field_0x4, reads its vtable pointer at
// offset 0, fetches the function at vtable offset 0x4 (slot 1), and
// tail-calls it. No frame, no saved registers.

extern "C" __declspec(naked) void FUN_00412570() {
    __asm {
        mov ecx, dword ptr [ecx + 0x4]
        mov eax, dword ptr [ecx]
        mov edx, dword ptr [eax + 0x4]
        jmp edx
    }
}
