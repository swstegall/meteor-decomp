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
// FUNCTION: ffxivgame 0x00401150 — thiscall: *this = 0xf54a1c; return this
//                                  (1-arg, unused — ret 0x4).
//
// Sibling FUN_00406ab0 is the full constructor for this class (writes the
// same 0xf54a1c into [this] before initialising fields +4 through +0x54
// and tail-calling FUN_00406750), so 0xf54a1c is the class's vtable
// address in .rdata. This 11-byte stub mirrors the engine_framework
// stamped-cluster shape `*(int *)this = <const>; return this;` with a
// `(int)` parameter that MSVC keeps on the stack (RET 0x4) but never
// references — the body only sets field0 and returns `this`.
//
// Asm: 8b c1 c7 00 1c 4a f5 00 c2 04 00

class C { public: C *set_vtable(int); };

C *C::set_vtable(int) {
    *(int *)this = 0xf54a1c;
    return this;
}
