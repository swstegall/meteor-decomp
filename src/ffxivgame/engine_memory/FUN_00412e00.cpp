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
// FUNCTION: ffxivgame 0x00012e00 — member-vtable tail-jmp via EDX (slot 0x20)
//
// Asm (10 bytes): 8b 49 04 8b 01 8b 50 20 ff e2
//
// Loads this->inner (ECX+4), reads its vtable, then tail-jumps through
// vtable slot 0x20 (8th pointer).  Same shape as the slot-0x4 cluster
// (FUN_0040f620) but targeting a different dispatch offset.

extern "C" __declspec(naked) void FUN_00412e00() {
    __asm {
        mov ecx, [ecx + 4]
        mov eax, [ecx]
        mov edx, [eax + 0x20]
        jmp edx
    }
}
