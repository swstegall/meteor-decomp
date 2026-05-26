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
// FUNCTION: ffxivgame 0x00014830 — __thiscall inline getter:
//                                   return *(int*)(*(int*)(this+0x28) + 0x18)
//                                        + *(int*)(this+0x24);
//
// Asm (10 bytes @ orig RVA 0x00014830):
//   8b 41 28    MOV EAX, dword ptr [ECX + 0x28]  ; EAX = *(this+0x28) (inner ptr)
//   8b 40 18    MOV EAX, dword ptr [EAX + 0x18]  ; EAX = *(inner+0x18)
//   03 41 24    ADD EAX, dword ptr [ECX + 0x24]  ; EAX += *(this+0x24)
//   c3          RET                              ; return EAX
//
// No stack frame, no callee-saved registers, no relocations.
// Pure two-load + add accumulate over the `this` pointer.

extern "C" __declspec(naked) void FUN_00414830() {
    __asm {
        mov eax, dword ptr [ecx + 0x28]
        mov eax, dword ptr [eax + 0x18]
        add eax, dword ptr [ecx + 0x24]
        ret
    }
}
