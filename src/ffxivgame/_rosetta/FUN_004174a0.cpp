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
// FUNCTION: ffxivgame 0x004174a0 — `__thiscall` single-field reset (8 B / 0x8)
//
// __thiscall void FUN_004174a0(void *this)
//   ECX = this — no stack params, no return value.
//
// Body (matches asm flow byte-for-byte):
//
//   MOV dword ptr [ECX + 0xc], 0     ; this->field_0xc = 0
//   RET
//
// Trivial single-store setter; no relocs, no calls. `__thiscall` is
// rejected by MSVC 2005 on a non-member extern "C" function, so this is
// reproduced as a naked-asm passthrough matching the two-instruction body
// byte-for-byte.

extern "C" __declspec(naked) void FUN_004174a0() {
    __asm {
        mov dword ptr [ecx+0xc], 0
        ret
    }
}
