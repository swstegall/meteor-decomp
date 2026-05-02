// [STAMPED] from FUN_004042d0.cpp by tools/stamp_clusters.py
//           sibling at orig RVA 0x0011ba70 (VA 0x0051ba70)
//           same byte-shape cluster — see cluster_shapes.py output
// [SEED] from FUN_00403cb0.cpp by tools/seed_templates.py
//        target VA 0x0051ba70 (RVA 0x000042d0)
//        cross-binary cluster match — same shape hash, same C++ idiom.
//        After seeding, run tools/stamp_clusters.py to fan out to siblings.
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
// FUNCTION: ffxivgame 0x0051ba70 — scalar deleting destructor pattern.
// Reloc-aware cluster template — primary of a 1,137-member cluster.
// Each member's two CALLs (inner dtor + free helper) point at
// per-class targets, but the byte structure is identical.
//
// Asm (27 bytes):
//   56              PUSH ESI
//   8b f1           MOV ESI, ECX           ; cache this in callee-save
//   e8 ?? ?? ?? ??  CALL inner_dtor        ; reloc — per-class destructor
//   f6 44 24 08 01  TEST [ESP+0x8], 0x1    ; test bool arg byte
//   74 09           JZ skip                ; skip free if flag clear
//   56              PUSH ESI               ; arg = this
//   e8 ?? ?? ?? ??  CALL free_helper       ; reloc — __stdcall, self-cleans
//   8b c6  skip:    MOV EAX, ESI           ; return this
//   5e              POP ESI
//   c2 04 00        RET 4                  ; __thiscall + 1 stack arg
//
// MSVC auto-generates this shape for virtual destructors: a "scalar
// deleting destructor" that runs the actual destructor then optionally
// invokes operator delete depending on a low-bit flag passed in by
// the caller. The flag's encoding (1-byte TEST, not a 4-byte CMP)
// constrains the source arg type to `unsigned char` (or `bool`), even
// though the stack slot is still 4 bytes wide per __thiscall.
//
// The free helper's calling convention is `__cdecl` (caller cleans
// the stack via `ADD ESP, 4` after the CALL). Note Ghidra's reported
// function size (27 B) is 3 B short — the true function ends with
// `8b c6 5e c2 04 00` (MOV EAX,ESI; POP ESI; RET 4) at 30 B,
// followed by `cc cc` INT3 padding. Ghidra's flow analysis stops at
// the POP ESI; the YAML / size override layer corrects this. Same
// pattern as FUN_00d36610.

class C {
public:
    void *do_thing(unsigned char flag);
    void inner();
};

extern "C" void __cdecl free_helper(void *p);

void *C::do_thing(unsigned char flag) {
    inner();
    if (flag & 1) {
        free_helper(this);
    }
    return this;
}
