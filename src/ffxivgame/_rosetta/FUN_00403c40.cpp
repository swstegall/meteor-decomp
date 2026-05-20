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
// FUNCTION: ffxivgame 0x00403c40 — basic_string-style `_Eos(n)`:
//   write a NUL terminator at offset `n` and update `_Mysize` (this+0x14).
//   When capacity (this+0x18) < 16 the buffer is in-place (SSO) starting
//   at this+4; otherwise the heap pointer at this+4 is dereferenced and
//   the terminator goes at heap_ptr[n].
//
// Asm (31 bytes):
//   83 79 18 10        cmp   dword ptr [ecx + 0x18], 0x10
//   8b 44 24 04        mov   eax, dword ptr [esp + 4]
//   89 41 14           mov   dword ptr [ecx + 0x14], eax
//   72 0a              jb    sso
//   8b 49 04           mov   ecx, dword ptr [ecx + 4]
//   c6 04 01 00        mov   byte ptr [ecx + eax], 0
//   c2 04 00           ret   4
// sso:
//   c6 44 01 04 00     mov   byte ptr [ecx + eax + 4], 0
//   c2 04 00           ret   4
//
// Naked-asm keeps the byte layout pinned: __thiscall (this in ecx)
// with one __stdcall-popped argument (`n`).

extern "C" __declspec(naked) void FUN_00403c40() {
    __asm {
        cmp     dword ptr [ecx + 0x18], 0x10
        mov     eax, dword ptr [esp + 4]
        mov     dword ptr [ecx + 0x14], eax
        jb      sso
        mov     ecx, dword ptr [ecx + 4]
        mov     byte ptr [ecx + eax * 1], 0
        ret     4
    sso:
        mov     byte ptr [ecx + eax * 1 + 4], 0
        ret     4
    }
}
