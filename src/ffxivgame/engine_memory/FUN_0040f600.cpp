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
// FUNCTION: ffxivgame 0x0000f600 — __thiscall: double-deref byte load
//                                   (7 bytes)
//
// Asm (7 bytes @ orig RVA 0x0000f600):
//   8b 41 04    MOV EAX, dword ptr [ECX + 0x4]  ; EAX = this->field_0x4 (inner obj ptr)
//   8a 40 28    MOV AL, byte ptr [EAX + 0x28]   ; AL = inner->field_0x28 (byte load)
//   c3          RET
//
// Reads this->field_0x4 (an inner object pointer) then returns the byte
// at offset 0x28 of that inner object. Plain __thiscall getter, no
// stack frame, no callee-saves.

struct Inner {
    char _pad[0x28];
    char field_0x28;
};

struct Outer {
    char _pad[0x4];
    Inner *field_0x4;

    char FUN_0040f600();
};

char Outer::FUN_0040f600() {
    return field_0x4->field_0x28;
}
