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
// FUNCTION: ffxivgame 0x0000ee70 — __stdcall negative-offset header read
//
// Asm (10 bytes):
//   8b 44 24 04     MOV EAX, dword ptr [ESP + 0x4]   ; EAX = param_1
//   8b 40 f8        MOV EAX, dword ptr [EAX + -0x8]  ; EAX = *(param_1 - 8)
//   c2 04 00        RET 0x4                           ; __stdcall, pops 1 dword
//
// Takes a pointer and reads the dword stored 8 bytes before it — a typical
// allocator header field access (e.g. block size or reference-count stored
// in a hidden prefix header). Same family as FUN_0040ecd0 in this module.

int __stdcall FUN_0040ee70(int param_1) {
    return *(int *)(param_1 - 8);
}
