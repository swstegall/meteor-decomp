// [STAMPED] from FUN_00416050.cpp by tools/stamp_clusters.py
//           sibling at orig RVA 0x0027b500 (VA 0x0067b500)
//           same byte-shape cluster — see cluster_shapes.py output
// [SEED] from FUN_004165d0.cpp by tools/seed_templates.py
//        target VA 0x0067b500 (RVA 0x00016050)
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
// FUNCTION: ffxivgame 0x4165d0 — return false (bool, 8-bit) — `__thiscall` 2-arg
// Stub-cluster template — multiple byte-identical members.
//
// Asm: 32 c0 c2 08 00

class C { public: unsigned char no(int, int); };
unsigned char C::no(int, int) { unsigned char r = 0; return r; }
