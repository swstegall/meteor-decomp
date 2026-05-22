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
// FUNCTION: ffxivgame 0x0000f540 — __stdcall 1-arg: load dword at ptr[-0x10]
//
// Asm (10 bytes):
//   8b 44 24 04   MOV EAX, dword ptr [ESP + 0x4]   ; eax = param_1
//   8b 40 f0      MOV EAX, dword ptr [EAX + -0x10] ; eax = *(int*)(param_1 - 0x10)
//   c2 04 00      RET 0x4                           ; stdcall: pop 1 arg
//
// Returns the dword stored 16 bytes before the supplied pointer.
// No frame, no saved registers — 3-instruction leaf.

int __stdcall FUN_0040f540(int param_1)
{
    return *(int *)(param_1 - 0x10);
}
