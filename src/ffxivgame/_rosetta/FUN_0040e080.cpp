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
// FUNCTION: ffxivgame 0x0000e080 — __thiscall 3-int field-setter (24 B).
//
// Stores three consecutive DWORD stack arguments into offsets 0x18, 0x1c,
// and 0x20 within the `this` object.  No stack frame (leaf, no locals or
// callee-saved register spills).
//
// Calling convention: __thiscall (ECX = this; callee cleans 12 bytes via
//   RET 0xc).
//
// Asm (24 bytes):
//   8b 44 24 04   MOV EAX,[ESP+0x4]      ; arg a
//   8b 54 24 08   MOV EDX,[ESP+0x8]      ; arg b
//   89 41 18      MOV [ECX+0x18],EAX     ; this->field18 = a
//   8b 44 24 0c   MOV EAX,[ESP+0xc]      ; arg c (reuses EAX)
//   89 51 1c      MOV [ECX+0x1c],EDX     ; this->field1c = b
//   89 41 20      MOV [ECX+0x20],EAX     ; this->field20 = c
//   c2 0c 00      RET 0xc

class C {
    char _pad[0x18];
    int field18;
    int field1c;
    int field20;
    void setThree(int a, int b, int c);
};

void C::setThree(int a, int b, int c)
{
    field18 = a;
    field1c = b;
    field20 = c;
}
