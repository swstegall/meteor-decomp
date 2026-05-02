// [STAMPED] from FUN_0082a890.cpp by tools/stamp_clusters.py
//           sibling at orig RVA 0x00685ac0 (VA 0x00a85ac0)
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
// FUNCTION: ffxivgame 0x00a85ac0 — chained pointer dereference getter
// Stub-cluster template — one of 30 byte-identical members.
//
// Asm: 8b 41 14 8b 40 10 c3
//   8b 41 14   MOV EAX, dword ptr [ECX + 0x14]   ; this->inner
//   8b 40 10   MOV EAX, dword ptr [EAX + 0x10]   ; inner->field
//   c3         RET

class Inner {
    int padding[4];      // [this+0..0xf] — 16 bytes
public:
    int *field;          // [this+0x10]
};

class Outer {
    int padding[5];      // [this+0..0x13] — 20 bytes
    Inner *inner;        // [this+0x14]
public:
    int *get_inner_field();
};

int *Outer::get_inner_field() {
    return inner->field;
}
