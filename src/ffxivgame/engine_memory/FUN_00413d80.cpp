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
// FUNCTION: ffxivgame 0x00013d80 — __thiscall vtable dispatch through inner member at +0x08,
//                                   slot 8 (offset 0x20)  (10 bytes)
//
// Asm (10 bytes @ orig RVA 0x00013d80):
//   8b 49 08    MOV ECX, dword ptr [ECX + 0x08]  ; ECX = this->field_0x08 (inner object ptr)
//   8b 01       MOV EAX, dword ptr [ECX]          ; EAX = *ECX = vtable ptr
//   8b 50 20    MOV EDX, dword ptr [EAX + 0x20]   ; EDX = vtable[8] = function ptr
//   ff e2       JMP EDX                            ; tail-call through vtable slot 8
//
// Compiler-generated thunk: adjusts `this` (ECX) to the inner sub-object at offset 0x08,
// then dispatches to its vtable slot 8 (0x20 bytes in).  No stack frame, no saved registers.

extern "C" __declspec(naked) void FUN_00413d80()
{
    __asm {
        mov ecx, [ecx + 0x08]
        mov eax, [ecx]
        mov edx, [eax + 0x20]
        jmp edx
    }
}
