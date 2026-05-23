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
// FUNCTION: ffxivgame 0x00012a70 — __thiscall: store self-pointer at +0x54, return &field_0x50
//                                   (7 bytes)
//
// Asm (7 bytes @ orig RVA 0x00012a70):
//   89 49 54          MOV dword ptr [ECX + 0x54], ECX  ; this->self = this
//   8d 41 50          LEA EAX, [ECX + 0x50]            ; return &this->field_0x50
//   c3                RET
//
// Stores a back-pointer to self at offset 0x54, then returns a pointer to
// the sub-object / field at offset 0x50. Plain __thiscall, no stack frame.

struct FUN_00412a70_C {
    char              pad[0x50];        // [+0x00..+0x4f]
    int               field_0x50;      // [+0x50]
    FUN_00412a70_C   *self;             // [+0x54] — back-pointer to outer

    int *init();
};

int *FUN_00412a70_C::init()
{
    self = this;
    return &field_0x50;
}
