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
// FUNCTION: ffxivgame 0x00013d90 — member-vtable tail-jmp via EDX (slot 0x24)
//
// Asm (10 bytes): 8b 49 08 8b 01 8b 50 24 ff e2
//
// Loads this->inner (ECX+8), reads its vtable, then tail-jumps through
// vtable slot 0x24 (10th pointer).  Same shape as the slot-0x20 sibling
// (FUN_00412e00) but reads the inner pointer from a different offset
// and targets a different dispatch slot.

extern "C" __declspec(naked) void FUN_00413d90() {
    __asm {
        mov ecx, [ecx + 8]
        mov eax, [ecx]
        mov edx, [eax + 0x24]
        jmp edx
    }
}
