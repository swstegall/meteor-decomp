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
// FUNCTION: ffxivgame 0x0000ee80 — __stdcall indexed read via negative-offset base (offset 0xC)
//
// Asm (14 bytes):
//   8b 44 24 04   MOV EAX, dword ptr [ESP + 0x4]          ; EAX = param_1
//   8b 48 f8      MOV ECX, dword ptr [EAX + -0x8]         ; ECX = *(param_1 - 8)
//   8b 44 01 0c   MOV EAX, dword ptr [ECX + EAX*1 + 0xc]  ; EAX = *(ECX + param_1 + 0xc)
//   c2 04 00      RET 0x4                                  ; __stdcall, pops 1 dword
//
// Reads a hidden header field 8 bytes before the pointer, then uses both
// the header value and the pointer itself as components of a computed
// address with fixed displacement 0xC.

int __stdcall FUN_0040ee80(int param_1) {
    return *(int *)(*(int *)(param_1 - 8) + param_1 + 0xc);
}
