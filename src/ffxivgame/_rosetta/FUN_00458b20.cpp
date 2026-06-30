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
// FUNCTION: ffxivgame 0x00458b20 — __stdcall pre-increment field at offset 4
//
// Asm (14 bytes):
//   8b 44 24 04   MOV EAX, [ESP+4]       ; load pointer arg
//   83 40 04 01   ADD dword [EAX+4], 1   ; pre-increment field at offset 4
//   8b 40 04      MOV EAX, [EAX+4]       ; return new value
//   c2 04 00      RET 4                  ; __stdcall, 1 stack arg

struct FUN_00458b20_S {
    int unknown0;
    int field;
};

int __stdcall FUN_00458b20(FUN_00458b20_S *p) {
    return ++p->field;
}
