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
// FUNCTION: ffxivgame 0x00014bf0 — __thiscall 9-byte Param constructor
//                                   (20 B / 0x14).
//
// Asm (20 bytes @ 0x00014bf0):
//   8b c1                 MOV  EAX, ECX           ; return value = this
//   c7 00 00 00 00 00     MOV  dword ptr [EAX],    0x0  ; f0 = 0
//   c7 40 04 00 00 00 00  MOV  dword ptr [EAX+4],  0x0  ; f4 = 0
//   c6 40 08 01           MOV  byte  ptr [EAX+8],  0x1  ; f8 = 1
//   c3                    RET
//
// Calling convention: __thiscall (ECX = this; no stack args; returns this
// in EAX — standard MSVC 2005 constructor return convention).
//
// The `MOV EAX, ECX` at the function entry is the canonical MSVC 2005
// constructor prologue: the compiler copies ECX (this) to EAX early so
// it can use EAX as both the base register for all field writes and as
// the return value, avoiding a redundant final MOV EAX, ECX.
//
// This constructor initialises a 9-byte (12-byte with trailing pad) Param
// struct used by the refcount-guarded singleton shim FUN_00414c10 — see
// decomp-notes/types/ffxivgame/0x00014c10.md for the calling context.
// The caller (FUN_00a55660) allocates a Param on its stack, constructs it
// via this function, then passes it by pointer to FUN_00414c10.

struct Param {
    int  f0;
    int  f4;
    char f8;
    Param();
};

Param::Param() {
    f0 = 0;
    f4 = 0;
    f8 = 1;
}
