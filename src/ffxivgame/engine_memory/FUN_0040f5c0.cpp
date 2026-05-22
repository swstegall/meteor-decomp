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
// FUNCTION: ffxivgame 0x0000f5c0 — __thiscall virtual dispatch thunk:
//                                   load inner object at this->field_0x4, read
//                                   vtable[1] from it, adjust ECX by +4,
//                                   tail-call through the method pointer.
//
// Asm (14 bytes @ orig RVA 0x0000f5c0):
//   8b 49 04    MOV ECX, dword ptr [ECX + 0x4]  ; ECX = this->field_0x4
//   8b 41 04    MOV EAX, dword ptr [ECX + 0x4]  ; EAX = *(ECX+4) (vtable ptr)
//   8b 50 04    MOV EDX, dword ptr [EAX + 0x4]  ; EDX = vtable[1] (method ptr)
//   83 c1 04    ADD ECX, 0x4                    ; adjust this-ptr for callee
//   ff e2       JMP EDX                          ; tail-call
//
// No stack frame, no callee-saved registers, no relocations.
// Pure register-manipulation + indirect tail-call virtual dispatch thunk.

extern "C" __declspec(naked) void FUN_0040f5c0() {
    __asm {
        mov ecx, dword ptr [ecx + 0x4]
        mov eax, dword ptr [ecx + 0x4]
        mov edx, dword ptr [eax + 0x4]
        add ecx, 0x4
        jmp edx
    }
}
