// [STAMPED] from FUN_0073c8d0.cpp by tools/stamp_clusters.py
//           sibling at orig RVA 0x0036f530 (VA 0x0076f530)
//           same byte-shape cluster — see cluster_shapes.py output
// [SEED] from FUN_0080a4b0.cpp by tools/seed_templates.py
//        target VA 0x0076f530 (RVA 0x0033c8d0)
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
// FUNCTION: ffxivgame 0x80a4b0 — int field getter at offset 0x64.
// Exact-byte cluster template — one of 4 byte-identical members.
//
// Asm: 8b 41 64 c3
//   8b 41 64   MOV EAX, dword ptr [ECX + 0x64]
//   c3            RET

class C {
    int padding[25];
    int field;
public:
    int get_field();
};

int C::get_field() {
    return field;
}
