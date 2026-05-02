// [SEED] from FUN_009f17e2.cpp by tools/seed_templates.py
//        target VA 0x00715930 (RVA 0x00315930)
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
// FUNCTION: ffxivgame 0x9f17e2 — int field getter at offset 0xc.
// Exact-byte cluster template — one of 7 byte-identical members.
//
// Asm: 8b 41 0c c3
//   8b 41 0c   MOV EAX, dword ptr [ECX + 0x0c]
//   c3            RET

class C {
    int padding[3];
    int field;
public:
    int get_field();
};

int C::get_field() {
    return field;
}
