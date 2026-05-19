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
// FUNCTION: ffxivgame 0x401150 — thiscall 1-arg method that stamps a
// constant 32-bit immediate (the SQEX::CDev::Engine::Fw::Framework::
// InitialConfiguration vftable VA, 0x00f54a1c) into *this and returns
// `this`. Same shape as the FUN_00d26570 set_field0 cluster, but the
// stored value is a compile-time immediate rather than a stack arg.
//
// Asm: 8b c1 c7 00 1c 4a f5 00 c2 04 00

class C { public: C *set_vtable(int); };

C *C::set_vtable(int) {
    *(void **)this = (void *)0xf54a1c;
    return this;
}
