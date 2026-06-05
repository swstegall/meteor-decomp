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
// FUNCTION: ffxivgame 0x459ae0 — subtract 4 from first stack arg then tail-jmp to FUN_00459610
//
// Asm: 83 6c 24 04 04 e9 26 fb ff ff

extern "C" void FUN_00459610();

extern "C" __declspec(naked) void FUN_00459ae0() {
    __asm {
        sub dword ptr [esp + 4], 4
        jmp FUN_00459610
    }
}
