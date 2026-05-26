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
// FUNCTION: ffxivgame 0x00012b60 — __thiscall member-vtable tail-jmp via EDX
//           Follow member pointer at [ECX+0x14], load vtable, jump slot 0xC.
//
// Asm: 8b 49 14 8b 01 8b 50 0c ff e2

extern "C" __declspec(naked) void FUN_00412b60() {
    __asm {
        mov ecx, [ecx + 0x14]
        mov eax, [ecx]
        mov edx, [eax + 0xc]
        jmp edx
    }
}
