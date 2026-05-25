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
// FUNCTION: ffxivgame 0x00014b30 — __thiscall: store two int args at +0x2c / +0x30
//                                   (17 bytes)
//
// Asm (17 bytes @ orig RVA 0x00014b30):
//   8b 44 24 04       MOV EAX, dword ptr [ESP + 0x4]   ; load param_1
//   8b 54 24 08       MOV EDX, dword ptr [ESP + 0x8]   ; load param_2
//   89 41 2c          MOV dword ptr [ECX + 0x2c], EAX  ; this->m_a = param_1
//   89 51 30          MOV dword ptr [ECX + 0x30], EDX  ; this->m_b = param_2
//   c2 08 00          RET 0x8                          ; __thiscall, callee cleans 8 bytes
//
// Plain __thiscall two-field setter — no frame, no callee-save spills.

struct FUN_00414b30_C {
    char pad[0x2c];     // [+0x00..+0x2b]
    int  m_a;           // [+0x2c]
    int  m_b;           // [+0x30]

    void set(int a, int b);
};

void FUN_00414b30_C::set(int a, int b)
{
    m_a = a;
    m_b = b;
}
