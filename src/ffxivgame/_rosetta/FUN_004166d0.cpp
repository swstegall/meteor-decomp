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
// FUNCTION: ffxivgame 0x4166d0 — __thiscall stub: zero field at offset 0xC
//
// Asm (8 bytes):
//   c7 41 0c 00 00 00 00    MOV dword ptr [ECX+0Ch], 0
//   c3                      RET
//
// Single __thiscall member, no stack args, returns void.
// Stores the constant 0 to [this+0xC].

struct FUN_004166d0_Obj {
    int field_00;
    int field_04;
    int field_08;
    int field_0c;

    void clear_field_0c();
};

void FUN_004166d0_Obj::clear_field_0c() {
    field_0c = 0;
}
