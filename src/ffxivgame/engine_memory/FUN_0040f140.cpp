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
// FUNCTION: ffxivgame 0x0000f140 — __stdcall 2-arg: return *(int*)(param_2 - 0x10)
//
// Asm (10 bytes):
//   8b 44 24 08   MOV EAX, dword ptr [ESP + 0x8]   ; EAX = param_2
//   8b 40 f0      MOV EAX, dword ptr [EAX + -0x10] ; EAX = *(param_2 - 0x10)
//   c2 08 00      RET 0x8                           ; __stdcall, pop 2 args
//
// Reads a field at -0x10 (offset -16) from the pointer in param_2.
// param_1 is not used.

extern "C" int __stdcall FUN_0040f140(int param_1, int *param_2) {
    return *(int *)((char *)param_2 - 0x10);
}
