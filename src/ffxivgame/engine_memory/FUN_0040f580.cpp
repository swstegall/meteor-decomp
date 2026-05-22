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
// FUNCTION: ffxivgame 0x0000f580 — __thiscall: store `this` at offset 0x60, return &field_0x5c
//
// Asm (7 bytes):
//   89 49 60  MOV dword ptr [ECX + 0x60], ECX  ; this->field_0x60 = this
//   8d 41 5c  LEA EAX, [ECX + 0x5c]            ; EAX = &this->field_0x5c
//   c3        RET

extern "C" __declspec(naked) void FUN_0040f580() {
    __asm {
        mov dword ptr [ecx + 0x60], ecx
        lea eax, [ecx + 0x5c]
        ret
    }
}
