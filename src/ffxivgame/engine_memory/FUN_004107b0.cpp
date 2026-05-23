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
// FUNCTION: ffxivgame 0x000107b0 — two-field DWORD setter at offsets 0x54/0x58
//                                   (__thiscall, 2 stack args, 17 bytes)
//
// Asm:
//   8b 44 24 04    MOV EAX, dword ptr [ESP + 0x4]   ; param_1
//   8b 54 24 08    MOV EDX, dword ptr [ESP + 0x8]   ; param_2
//   89 41 54       MOV dword ptr [ECX + 0x54], EAX  ; this->field_54 = param_1
//   89 51 58       MOV dword ptr [ECX + 0x58], EDX  ; this->field_58 = param_2
//   c2 08 00       RET 0x8

struct FUN_004107b0_C {
    char pad[0x54];    // [+0x00..+0x53]
    int field_54;      // [+0x54]
    int field_58;      // [+0x58]

    void set_fields(int a, int b);
};

void FUN_004107b0_C::set_fields(int a, int b)
{
    field_54 = a;
    field_58 = b;
}
