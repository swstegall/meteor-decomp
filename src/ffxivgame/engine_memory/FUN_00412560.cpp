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
// FUNCTION: ffxivgame 0x00012560 — __thiscall: double-deref nonzero check
//                                   (14 bytes)
//
// Asm (14 bytes @ orig RVA 0x00012560):
//   8b 41 04          MOV EAX, dword ptr [ECX + 0x4]   ; EAX = this->field_0x4 (inner obj ptr)
//   33 c9             XOR ECX, ECX                      ; ECX = 0
//   39 48 2c          CMP dword ptr [EAX + 0x2c], ECX   ; inner->field_0x2c != 0?
//   0f 95 c1          SETNZ CL                          ; CL = (field_0x2c != 0)
//   8a c1             MOV AL, CL                        ; AL = result
//   c3                RET
//
// Reads this->field_0x4 (inner object pointer), then returns whether
// inner->field_0x2c is non-zero. Plain __thiscall bool getter, no stack
// frame, no callee-saves.

struct Inner_00412560 {
    char _pad[0x2c];
    int field_0x2c;
};

struct Outer_00412560 {
    char _pad[0x4];
    Inner_00412560 *field_0x4;

    bool FUN_00412560();
};

bool Outer_00412560::FUN_00412560() {
    return field_0x4->field_0x2c != 0;
}
