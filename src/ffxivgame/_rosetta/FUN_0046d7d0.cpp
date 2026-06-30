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
// FUNCTION: ffxivgame 0x0006d7d0 — `__cdecl` byte-indexed DWORD lookup:
//                                   dereference arg1 as byte-offset, read
//                                   DWORD from arg2->field_0x4 at that offset
//                                   (17 B / 0x11).
//
// Asm (17 bytes @ orig RVA 0x0006d7d0):
//   8b 44 24 08   MOV EAX, [ESP+8]          ; EAX = arg2 (char**)
//   8b 48 04      MOV ECX, [EAX+4]          ; ECX = arg2[1] (char* at offset 4)
//   8b 54 24 04   MOV EDX, [ESP+4]          ; EDX = arg1 (int*)
//   8b 02         MOV EAX, [EDX]            ; EAX = *arg1 (byte offset)
//   8b 04 01      MOV EAX, [ECX + EAX*0x1] ; EAX = *(int*)(arg2[1] + *arg1)
//   c3            RET                       ; __cdecl, caller cleans stack
//
// The scale-1 SIB byte (0x01) on the final load confirms byte-offset
// pointer arithmetic: arg2[1] is a `char*` and *arg1 is added as a raw
// byte count before dereferencing as a DWORD.

extern "C" int __cdecl FUN_0046d7d0(int* a, char** b) {
    return *(int*)(b[1] + *a);
}
