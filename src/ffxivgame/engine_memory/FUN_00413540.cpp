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
// FUNCTION: ffxivgame 0x00013540 — __thiscall member-vtable tail-jmp via EDX
//           Follow member pointer at [ECX+0x4], load vtable, jump slot 0x4.
//
// Asm: 8b 49 04 8b 01 8b 50 04 ff e2

extern "C" __declspec(naked) void FUN_00413540() {
    __asm {
        mov ecx, [ecx + 0x4]
        mov eax, [ecx]
        mov edx, [eax + 0x4]
        jmp edx
    }
}
