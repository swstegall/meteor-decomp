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
// FUNCTION: ffxivgame 0x0000f620 — __thiscall: double-deref vtable dispatch, slot 0x4
//                                   (10 bytes)
//
// Asm (10 bytes @ orig RVA 0x0000f620):
//   8b 49 04    MOV ECX, dword ptr [ECX + 0x4]  ; ECX = this->field_0x4 (inner obj)
//   8b 01       MOV EAX, dword ptr [ECX]        ; EAX = inner->vtable (field_0x0)
//   8b 50 04    MOV EDX, dword ptr [EAX + 0x4]  ; EDX = vtable[1] (slot offset 4)
//   ff e2       JMP EDX                          ; tail-call through vtable slot
//
// Loads a nested object via this->field_0x4, reads its vtable pointer at
// field_0x0, fetches the function at vtable offset 0x4 (slot 1), and
// tail-calls it with ECX pointing to the inner object. No ECX adjustment,
// no frame, no saved registers.

extern "C" __declspec(naked) void FUN_0040f620() {
    __asm {
        mov ecx, dword ptr [ecx + 0x4]
        mov eax, dword ptr [ecx]
        mov edx, dword ptr [eax + 0x4]
        jmp edx
    }
}
