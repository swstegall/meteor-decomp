// [SEED] from FUN_00a01540.cpp by tools/seed_templates.py
//        target VA 0x0094d930 (RVA 0x0054d930)
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
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// FUNCTION: ffxivgame 0xa01540 — int field getter at offset 0x6c.
// Exact-byte cluster template — one of 3 byte-identical members.
//
// Asm: 8b 41 6c c3
//   8b 41 6c   MOV EAX, dword ptr [ECX + 0x6c]
//   c3            RET

class C {
    int padding[27];
    int field;
public:
    int get_field();
};

int C::get_field() {
    return field;
}
