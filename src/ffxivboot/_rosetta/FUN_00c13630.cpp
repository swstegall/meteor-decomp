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
// FUNCTION: ffxivboot 0xc13630 — chained ptr getter (signed-disp; naked asm)
// this->[ECX-0x24]->field+0x10. Asm: 8b 41 dc 8b 40 10 c3 (7 bytes).
//
// Why naked asm: the outer displacement 0xdc is a *signed* int8 = -0x24.
// MSVC has no clean field-access form for a negative offset (would force
// disp32 encoding → 10 bytes), so we hand-emit the 7-byte form.

extern "C" __declspec(naked) int *chained_ptr_getter() {
    __asm {
        mov eax, [ecx - 0x24]
        mov eax, [eax + 0x10]
        ret
    }
}
