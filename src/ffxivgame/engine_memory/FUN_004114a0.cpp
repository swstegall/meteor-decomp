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
// FUNCTION: ffxivgame 0x000114a0 — returns true (3 bytes)
//
// Asm (3 bytes @ orig RVA 0x000114a0):
//   b0 01   MOV AL, 0x1   ; return value = 1 (true)
//   c3      RET
//
// Trivial __cdecl function with no parameters; returns bool true.

bool FUN_004114a0()
{
    return true;
}
