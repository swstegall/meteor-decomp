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
// FUNCTION: ffxivgame 0x0000f550 — __thiscall vtable-install stub
//
// Asm (11 bytes):
//   8b c1              MOV EAX, ECX           ; this → EAX (return value)
//   c7 00 70 65 f5 00  MOV dword ptr [EAX], 0xf56570 ; *this = vtbl ptr
//   c2 04 00           RET 0x4                ; __thiscall, callee pops 1 arg
//
// Stores the constant 0x00f56570 into the first DWORD of the object
// pointed to by ECX (this), then returns this.  The single stack
// argument is unused.  MSVC 2005 /O2 moves ECX→EAX first (setting up
// the return value), uses that same EAX for the store, then RET 4.

class C_0040f550 {
public:
    C_0040f550 *install_vtable(int unused);
};

C_0040f550 *C_0040f550::install_vtable(int) {
    *(int *)this = 0x00f56570;
    return this;
}
