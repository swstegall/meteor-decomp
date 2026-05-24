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
// FUNCTION: ffxivgame 0x00013f80 — `this`-cached call-and-return-self
// wrapper that forwards into the inner sibling at RVA 0x00013e50 (a
// SQEX::CDev::Engine::Memory::Alternative::DetachableHeapBlock member
// routine — see decomp-notes/types/ffxivgame/0x000139d0.md for the
// surrounding class layout). 14 bytes.
//
// This is the canonical `class C { C *do_thing(int); }` shape — first
// member of a 322-member cluster (reloc-aware) that MSVC 2005 emits
// for any `__thiscall` member fn that takes one stack arg, calls a
// single sibling member (ECX = this, no args), and returns `this`.
// Every instance differs only by the CALL displacement.
//
// Asm (14 bytes):
//   56              PUSH ESI
//   8b f1           MOV  ESI, ECX        ; cache `this` in callee-save
//   e8 c8 fe ff ff  CALL FUN_00413e50    ; REL32 → -0x138 → 0x00013e50
//   8b c6           MOV  EAX, ESI        ; recover `this` for return
//   5e              POP  ESI
//   c2 04 00        RET  4               ; __thiscall, 1 stack arg
//
// Why ECX must be cached: the inner call clobbers ECX (and any other
// caller-save), so MSVC stashes `this` in callee-save ESI across the
// call. The `MOV EAX, ESI` epilogue then materialises the customary
// "return this" for chained-call idioms.
//
// Reloc-bearing position (masked by compare.py via the COFF REL32 mask):
//   off 0x04 : REL32 → inner (FUN_00413e50)

class C {
public:
    C *do_thing(int unused);
    void inner();
};

C *C::do_thing(int) {
    inner();
    return this;
}
