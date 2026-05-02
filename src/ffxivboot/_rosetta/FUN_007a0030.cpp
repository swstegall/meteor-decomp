// [STAMPED] from FUN_00534610.cpp by tools/stamp_clusters.py
//           sibling at orig RVA 0x003a0030 (VA 0x007a0030)
//           same byte-shape cluster — see cluster_shapes.py output
// [SEED] from FUN_00609000.cpp by tools/seed_templates.py
//        target VA 0x007a0030 (RVA 0x00134610)
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
// FUNCTION: ffxivgame 0x609000 — return true (bool, 8-bit) — `__thiscall` 1-arg
// Stub-cluster template — multiple byte-identical members.
//
// Asm: b0 01 c2 04 00

class C { public: unsigned char yes(int); };
unsigned char C::yes(int) { unsigned char r = 1; return r; }
