// [STAMPED] from FUN_004b1440.cpp by tools/stamp_clusters.py
//           sibling at orig RVA 0x001f0cc0 (VA 0x005f0cc0)
//           same byte-shape cluster — see cluster_shapes.py output
// [SEED] from FUN_005483b0.cpp by tools/seed_templates.py
//        target VA 0x005f0cc0 (RVA 0x000b1440)
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
// FUNCTION: ffxivgame 0x5483b0 — int field getter at offset 0x44.
// Exact-byte cluster template — one of 6 byte-identical members.
//
// Asm: 8b 41 44 c3
//   8b 41 44   MOV EAX, dword ptr [ECX + 0x44]
//   c3            RET

class C {
    int padding[17];
    int field;
public:
    int get_field();
};

int C::get_field() {
    return field;
}
