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
// FUNCTION: ffxivgame 0x00012b70 — __thiscall vtable dispatch through inner member at +0x14,
//                                   slot 4 (offset 0x10)  (10 bytes)
//
// Asm (10 bytes @ orig RVA 0x00012b70):
//   8b 49 14    MOV ECX, dword ptr [ECX + 0x14]  ; ECX = this->field_0x14 (inner object ptr)
//   8b 01       MOV EAX, dword ptr [ECX]          ; EAX = *ECX = vtable ptr
//   8b 50 10    MOV EDX, dword ptr [EAX + 0x10]   ; EDX = vtable[4] = function ptr
//   ff e2       JMP EDX                            ; tail-call through vtable slot 4
//
// Compiler-generated thunk: adjusts `this` (ECX) to the inner sub-object at offset 0x14,
// then dispatches to its vtable slot 4 (0x10 bytes in).  No stack frame, no saved registers.

extern "C" __declspec(naked) void FUN_00412b70()
{
    __asm {
        mov ecx, [ecx + 0x14]
        mov eax, [ecx]
        mov edx, [eax + 0x10]
        jmp edx
    }
}
