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
// FUNCTION: ffxivgame 0x000139c0 — __thiscall: self-stash + embedded-member-pointer
//                                   return (7 bytes)
//
// Asm (7 bytes @ orig RVA 0x000139c0):
//   89 49 58    MOV dword ptr [ECX + 0x58], ECX  ; this->field_0x58 = this
//   8d 41 54    LEA EAX, [ECX + 0x54]            ; return &this->field_0x54
//   c3          RET
//
// Stores `this` into its own field at offset 0x58 (typical embedded
// list-head / parent-back-pointer init) and returns the address of an
// inline sub-object at offset 0x54. No frame, no saved registers.

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
// The real implementation is the MSVC __declspec(naked) + __asm block below,
// which clang cannot parse. Production builds always use cl.exe (MSVC 2005).
extern "C" void FUN_004139c0() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_004139c0() {
    __asm {
        mov dword ptr [ecx + 0x58], ecx
        lea eax, [ecx + 0x54]
        ret
    }
}
#endif
