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
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// FUNCTION: ffxivgame 0x411dc0 — multi-inheritance `this`-adjustor (SUB ECX, 0x4; JMP)
//
// Asm: 83 e9 04 e9 RR RR RR RR

extern "C" int FUN_00412230();

extern "C" __declspec(naked) void FUN_00411dc0() {
    __asm {
        sub ecx, 4
        jmp FUN_00412230
    }
}
