// [STAMPED] from FUN_00522450.cpp by tools/stamp_clusters.py
//           sibling at orig RVA 0x005664b0 (VA 0x009664b0)
//           same byte-shape cluster — see cluster_shapes.py output
// [SEED] from FUN_00548060.cpp by tools/seed_templates.py
//        target VA 0x009664b0 (RVA 0x00122450)
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
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// FUNCTION: ffxivgame 0x548060 — int field getter at offset 0x4c.
// Exact-byte cluster template — one of 8 byte-identical members.
//
// Asm: 8b 41 4c c3
//   8b 41 4c   MOV EAX, dword ptr [ECX + 0x4c]
//   c3            RET

class C {
    int padding[19];
    int field;
public:
    int get_field();
};

int C::get_field() {
    return field;
}
