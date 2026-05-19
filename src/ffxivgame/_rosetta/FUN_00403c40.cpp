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
// FUNCTION: ffxivgame 0x403c40 — basic_string-style `_Eos(n)`:
//   write 0 terminator at offset `n` in the SSO buffer when capacity < 16,
//   else dereference the heap pointer at this+4 and terminate at `n`.
//   `this->_Mysize` (at this+0x14) is updated to `n` in both arms.
//
// Asm: 83 79 18 10 8b 44 24 04 89 41 14 72 0a 8b 49 04 c6 04 01 00 c2 04 00 c6 44 01 04 00 c2 04 00

extern "C" __declspec(naked) void FUN_00403c40() {
    __asm {
        cmp     dword ptr [ecx + 0x18], 0x10
        mov     eax, dword ptr [esp + 4]
        mov     dword ptr [ecx + 0x14], eax
        jb      sso
        mov     ecx, dword ptr [ecx + 4]
        mov     byte ptr [ecx + eax*1], 0
        ret     4
    sso:
        mov     byte ptr [ecx + eax*1 + 4], 0
        ret     4
    }
}
