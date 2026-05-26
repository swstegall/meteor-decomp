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
// FUNCTION: ffxivgame 0x00012550 — Return true (bool, 8-bit form) — MOV AL,1; RET.
// Stub-cluster template — same byte-shape as FUN_004114a0.
//
// Asm: b0 01 c3

class C { public: unsigned char yes() const; };
unsigned char C::yes() const {
    unsigned char r = 1;
    return r;
}
