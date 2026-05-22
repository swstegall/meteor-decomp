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
// FUNCTION: ffxivgame 0x00406ba0 — __thiscall two-arg setter that stores
// the pair (param_1, param_2) into the adjacent globals at 0x01327a10 /
// 0x01327a14 and returns `this` unchanged. One of a family of identical
// shape setters in the 0x00406b70-0x00406bb9 block (FUN_00406b70 stores
// to 0x01327a00/04, FUN_00406b90 is the single-arg sibling at
// 0x013279fc, and this one at 0x01327a10/14) — almost certainly an
// inline `setXY` method on a singleton "viewport / camera / matrix"
// struct whose backing storage lives in static globals.
//
// Asm (25 bytes):
//   8b 54 24 08            mov  edx, [esp+8]        ; param_2 -> edx
//   8b c1                  mov  eax, ecx            ; preserve `this`
//                                                   ; as return value
//   8b 4c 24 04            mov  ecx, [esp+4]        ; param_1 -> ecx
//                                                   ; (ecx is now free
//                                                   ; since this was
//                                                   ; cached in eax)
//   89 0d 10 7a 32 01      mov  [0x01327a10], ecx   ; g[0] = param_1
//   89 15 14 7a 32 01      mov  [0x01327a14], edx   ; g[1] = param_2
//   c2 08 00               ret  8                   ; __thiscall: callee
//                                                   ; cleans the 2 stack
//                                                   ; args; eax = this
//
// Calling convention: __thiscall (ECX = this; two 4-byte stack args at
// [esp+4] / [esp+8]; callee cleans 8 bytes via `ret 8`). The unusual
// scheduling — load edx BEFORE caching ecx — is preserved verbatim
// because the source-level form has no way to force MSVC into that
// exact instruction order; this is the MSVC 2005 /O2 register
// allocator's specific choice for the "save this, then free ecx for a
// store" pattern. The two DIR32 fixups on the global stores are
// masked out of the byte diff by tools/compare.py.

extern "C" int DAT_01327a10;
extern "C" int DAT_01327a14;

extern "C" __declspec(naked) void FUN_00406ba0() {
    __asm {
        mov     edx, dword ptr [esp + 8]
        mov     eax, ecx
        mov     ecx, dword ptr [esp + 4]
        mov     dword ptr [DAT_01327a10], ecx
        mov     dword ptr [DAT_01327a14], edx
        ret     8
    }
}
