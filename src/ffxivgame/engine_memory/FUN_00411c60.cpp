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
// FUNCTION: ffxivgame 0x00011c60 — thiscall: *this = 0xf56ccc; return this
//                                  (1-arg, unused — ret 0x4).
//
// Ghidra identifies *this = SQEX::CDev::Engine::Memory::Alternative::
// IHandleListener::vftable (abs addr 0x00f56ccc in the image).  This 11-byte
// stub mirrors the engine_framework stamped-cluster shape
// `*(int *)this = <const>; return this;` with a `(int)` parameter that MSVC
// keeps on the stack (RET 0x4) but never references — the body only sets
// field0 and returns `this`.
//
// Asm: 8b c1 c7 00 cc 6c f5 00 c2 04 00

class C { public: C *set_vtable(int); };

C *C::set_vtable(int) {
    *(int *)this = 0xf56ccc;
    return this;
}
