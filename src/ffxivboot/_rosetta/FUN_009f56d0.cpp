// [STAMPED] from FUN_00451ef0.cpp by tools/stamp_clusters.py
//           sibling at orig RVA 0x005f56d0 (VA 0x009f56d0)
//           same byte-shape cluster — see cluster_shapes.py output
// [SEED] from FUN_00458ce0.cpp by tools/seed_templates.py
//        target VA 0x009f56d0 (RVA 0x00051ef0)
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
// FUNCTION: ffxivgame 0x458ce0 — return 0 (int) — `__thiscall` 2-arg
// Stub-cluster template — multiple byte-identical members.
//
// Asm: 33 c0 c2 08 00

class C { public: int zero(int, int); };
int C::zero(int, int) { return 0; }
