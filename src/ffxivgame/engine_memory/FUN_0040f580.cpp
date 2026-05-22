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
// FUNCTION: ffxivgame 0x0000f580 — __thiscall member: store `this` into field_0x60,
//                                   return &this->field_0x5c
//
// Asm (7 bytes @ orig RVA 0x0000f580):
//   89 49 60    MOV dword ptr [ECX + 0x60], ECX  ; this->_field_60 = this
//   8d 41 5c    LEA EAX, [ECX + 0x5c]            ; EAX = &this->_field_5c
//   c3          RET                               ; __thiscall, no stack args
//
// 3-instruction leaf: no prologue, no callee-saved registers, no locals.
// Stores a back-pointer to `this` at offset 0x60, then returns the address
// of the embedded node/sub-object pointer at offset 0x5c.

struct FUN_0040f580_C {
    char _pad[0x5c];          // offset 0x00..0x5b  (92 bytes)
    FUN_0040f580_C *_field_5c; // offset 0x5c        (4 bytes)
    FUN_0040f580_C *_field_60; // offset 0x60        (4 bytes)

    FUN_0040f580_C **FUN_0040f580();
};

FUN_0040f580_C **FUN_0040f580_C::FUN_0040f580()
{
    _field_60 = this;   // MOV [ECX+0x60], ECX
    return &_field_5c;  // LEA EAX, [ECX+0x5c]
}
