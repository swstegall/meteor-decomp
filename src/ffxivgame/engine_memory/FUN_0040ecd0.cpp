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
// FUNCTION: ffxivgame 0x0000ecd0 — __stdcall indexed-read via negative-offset base
//
// Asm (14 bytes):
//   8b 44 24 08     MOV EAX, dword ptr [ESP + 0x8]       ; EAX = param_2
//   8b 48 f8        MOV ECX, dword ptr [EAX + -0x8]      ; ECX = *(param_2 - 8)
//   8b 44 01 04     MOV EAX, dword ptr [ECX + EAX*1 + 4] ; EAX = *(ECX + param_2 + 4)
//   c2 08 00        RET 0x8                               ; __stdcall, pops 2 dwords
//
// param_1 (ESP+4) is unused; param_2 (ESP+8) serves as both the base for
// the negative-offset header dereference and the index into the resulting
// array (or vtable-like block). Returns the dword at
//   *(*(param_2 - 8) + param_2 + 4).

int __stdcall FUN_0040ecd0(int param_1, int param_2) {
    return *(int *)(*(int *)(param_2 - 8) + param_2 + 4);
}
