// [STAMPED] from FUN_0040e360.cpp by tools/stamp_clusters.py
//           sibling at orig RVA 0x0062b120 (VA 0x00a2b120)
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
// FUNCTION: ffxivgame 0x00a2b120 — int field getter at offset 4
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
