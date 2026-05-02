// [STAMPED] from FUN_0040de00.cpp by tools/stamp_clusters.py
//           sibling at orig RVA 0x000ef1e0 (VA 0x004ef1e0)
//           same byte-shape cluster — see cluster_shapes.py output
// [SEED] from FUN_0040e360.cpp by tools/seed_templates.py
//        target VA 0x004ef1e0 (RVA 0x0000de00)
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
// FUNCTION: ffxivgame 0x004ef1e0 — int field getter at offset 4
// Stub-cluster template — one of 33 byte-identical members.
//
// Asm: 8b 41 04 c3
//   8b 41 04   MOV EAX, dword ptr [ECX + 0x4]
//   c3         RET

class C {
public:
    int get_field();
private:
    int padding;       // [this+0]
    int field;         // [this+4]
};

int C::get_field() {
    return field;
}
