// [SEED] from FUN_004ec8a0.cpp by tools/seed_templates.py
//        target VA 0x004ef330 (RVA 0x000ef330)
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
// FUNCTION: ffxivgame 0x004ef330 — empty `__thiscall` 5-arg method (RET 0x14).
// Exact-byte cluster template — one of 16 byte-identical members.
// Asm: c2 14 00  (RET 0x14 = pop 20 bytes = 5x 4-byte args)

class C { public: void empty5(int, int, int, int, int); };
void C::empty5(int, int, int, int, int) {}
