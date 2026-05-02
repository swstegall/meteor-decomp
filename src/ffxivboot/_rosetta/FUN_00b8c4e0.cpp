// [STAMPED] from FUN_009769e0.cpp by tools/stamp_clusters.py
//           sibling at orig RVA 0x0078c4e0 (VA 0x00b8c4e0)
//           same byte-shape cluster — see cluster_shapes.py output
// [SEED] from FUN_00406620.cpp by tools/seed_templates.py
//        target VA 0x00b8c4e0 (RVA 0x005769e0)
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
// FUNCTION: ffxivgame 0x00006620 — int field setter at offset 0x18.
// Third easy-wins queue match candidate.
//
// Behavior (read from the asm @ 0x00006620, 10 bytes):
//   8b 44 24 04   MOV EAX, dword ptr [ESP + 0x4]
//   89 41 18      MOV dword ptr [ECX + 0x18], EAX
//   c2 04 00      RET 0x4
//
// Standard `__thiscall` int-field setter. Loads the 4-byte arg from
// [ESP+4], stores it at [this+0x18], cleans up 4 bytes on return.

class IntFieldSetter {
public:
    void set_field(int v);
private:
    int padding[6];   // [this+0..0x17] — 24 bytes of padding
    int field;        // [this+0x18]
};

void IntFieldSetter::set_field(int v) {
    field = v;
}
