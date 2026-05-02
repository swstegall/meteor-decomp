// [STAMPED] from FUN_004087e0.cpp by tools/stamp_clusters.py
//           sibling at orig RVA 0x006209f0 (VA 0x00a209f0)
//           same byte-shape cluster — see cluster_shapes.py output
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
// FUNCTION: ffxivgame 0x00a209f0 — `this`-cached call-and-return-self
// shape. First *relocation-aware* cluster template — primary of a
// 322-member cluster (reloc-aware, vs 107 in the exact-byte cluster);
// each member calls a different inner method but the byte structure
// is identical mod the CALL displacement.
//
// Asm (14 bytes):
//   56              PUSH ESI
//   8b f1           MOV ESI, ECX        ; cache `this` in callee-save
//   e8 ?? ?? ?? ??  CALL inner          ; reloc — different per member
//   8b c6           MOV EAX, ESI        ; recover `this` for return
//   5e              POP ESI
//   c2 04 00        RET 4               ; __thiscall, 1 stack arg
//
// Standard C++ idiom: `__thiscall` member function that takes one
// stack arg, calls a sibling member (ECX = `this`, no args), and
// returns `this`. ECX gets clobbered by the inner call, so MSVC
// caches `this` in ESI before the call and recovers it for the
// final MOV EAX, ESI.

class C {
public:
    C *do_thing(int unused);
    void inner();
};

C *C::do_thing(int) {
    inner();
    return this;
}
