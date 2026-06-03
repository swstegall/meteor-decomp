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
// FUNCTION: ffxivgame 0x000169b0 — `__thiscall` zero-field-at-0xC setter (8 bytes)
//
// A trivial `__thiscall` member function (no frame, no stack args) that
// clears the DWORD at offset 0xC of `this` (ECX) to zero and returns void.
//
// Asm (8 bytes, RVA 0x000169b0..0x000169b7):
//   000169b0: c7 41 0c 00 00 00 00  MOV dword ptr [ECX+0xC], 0x0
//   000169b7: c3                    RET
//
// No relocations. No frame. No stack args. Pure store-and-return.
// Naked byte passthrough produces byte-identical .text.

extern "C" __declspec(naked) void FUN_004169b0() {
    __asm {
        _emit 0xc7    // MOV dword ptr [ECX+0xC], 0x0
        _emit 0x41
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3    // RET
    }
}
