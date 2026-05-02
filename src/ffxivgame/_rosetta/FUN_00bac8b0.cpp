// [STAMPED] from FUN_004b68a0.cpp by tools/stamp_clusters.py
//           sibling at orig RVA 0x007ac8b0 (VA 0x00bac8b0)
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
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// FUNCTION: ffxivgame 0x4b68a0 — int field getter at offset 0x20.
// Exact-byte cluster template — one of 8 byte-identical members.
//
// Asm: 8b 41 20 c3
//   8b 41 20   MOV EAX, dword ptr [ECX + 0x20]
//   c3            RET

class C {
    int padding[8];
    int field;
public:
    int get_field();
};

int C::get_field() {
    return field;
}
