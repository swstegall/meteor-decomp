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
// FUNCTION: ffxivgame 0x00011d20 — store this into field_0x78, return &field_0x74
//                                   (__thiscall, 7 bytes)
//
// Asm (7 bytes @ orig RVA 0x00011d20):
//   89 49 78    MOV dword ptr [ECX + 0x78], ECX  ; this->field_0x78 = this
//   8d 41 74    LEA EAX, [ECX + 0x74]            ; EAX = &this->field_0x74
//   c3          RET
//
// Stores a self-reference (back-pointer) at offset 0x78, then returns the
// address of the field at offset 0x74.

struct FUN_00411d20_C {
    char pad[0x74];        // [+0x00..+0x73]
    void *field_74;        // [+0x74]
    void *field_78;        // [+0x78]

    void *init();
};

void *FUN_00411d20_C::init()
{
    field_78 = this;
    return &field_74;
}
