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
// FUNCTION: ffxivgame 0x004134f0 — member-adjusted vtable tail-jmp (slot 0x8)
// Same shape as _rosetta/FUN_0040f5d0: load sub-object at [ecx+4], load its
// vtable at [ecx+4], fetch function pointer at vtable slot 2 (offset 0x8),
// adjust this to ecx+4, then tail-jump.
//
// Asm: 8b 49 04 8b 41 04 8b 50 08 83 c1 04 ff e2

extern "C" __declspec(naked) void FUN_004134f0() {
    __asm {
        mov ecx, [ecx + 4]
        mov eax, [ecx + 4]
        mov edx, [eax + 0x8]
        add ecx, 4
        jmp edx
    }
}
