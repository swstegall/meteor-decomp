// [STAMPED] from FUN_004ef320.cpp by tools/stamp_clusters.py
//           sibling at orig RVA 0x006a2ea0 (VA 0x00aa2ea0)
//           same byte-shape cluster — see cluster_shapes.py output
// [SEED] from FUN_00590920.cpp by tools/seed_templates.py
//        target VA 0x00aa2ea0 (RVA 0x000ef320)
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
// FUNCTION: ffxivgame 0x590920 — return 0 (int) — `__thiscall` 1-arg
// Stub-cluster template — multiple byte-identical members.
//
// Asm: 33 c0 c2 04 00

class C { public: int zero(int); };
int C::zero(int) { return 0; }
