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
// FUNCTION: ffxivgame 0x00010620 — __thiscall: double-deref field getter with addition
//                                   (10 bytes)
//
// Asm (10 bytes @ orig RVA 0x00010620):
//   8b 41 2c    MOV EAX, dword ptr [ECX + 0x2c]  ; EAX = this->field_0x2c (a pointer)
//   8b 40 18    MOV EAX, dword ptr [EAX + 0x18]  ; EAX = *field_0x2c->field_0x18
//   03 41 28    ADD EAX, dword ptr [ECX + 0x28]  ; EAX += this->field_0x28
//   c3          RET
//
// Reads a pointer from this+0x2c, dereferences it at offset 0x18, then
// adds this->field_0x28 and returns the sum. No frame, no saved registers.

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
// The real implementation is the MSVC __declspec(naked) + __asm block below,
// which clang cannot parse. Production builds always use cl.exe (MSVC 2005).
extern "C" int FUN_00410620() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) int FUN_00410620() {
    __asm {
        mov eax, dword ptr [ecx + 0x2c]
        mov eax, dword ptr [eax + 0x18]
        add eax, dword ptr [ecx + 0x28]
        ret
    }
}
#endif
