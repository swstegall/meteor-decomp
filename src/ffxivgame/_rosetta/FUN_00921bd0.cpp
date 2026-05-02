// [STAMPED] from FUN_0040e350.cpp by tools/stamp_clusters.py
//           sibling at orig RVA 0x00521bd0 (VA 0x00921bd0)
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
// FUNCTION: ffxivgame 0x40e350 — Return `this` — MOV EAX,ECX; RET.
// Stub-cluster template — one of {many} byte-identical members.
// Run tools/stamp_clusters.py after committing to stamp the rest.
//
// Asm: 8b c1 c3

class C { public: C *identity(); };
C *C::identity() { return this; }
