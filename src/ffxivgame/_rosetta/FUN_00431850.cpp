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
// FUNCTION: ffxivgame 0x00431850 — __thiscall guard that conditionally
//                                  tail-calls the teardown routine.
//
// Asm (12 bytes):
//   83 79 14 00      CMP dword ptr [ECX+0x14],0x0
//   75 05            JNZ 0x0043185b           ; field nonzero → return
//   e9 65 ff ff ff   JMP 0x004317c0           ; tail call teardown
//   c3               RET
//
// __thiscall member: if the guard flag at [this+0x14] is zero, the object
// has not yet been torn down, so MSVC /O2 emits a tail-call JMP to the
// sibling teardown routine FUN_004317c0 (which releases the embedded
// vector + members at +0x30/+0x34/+0x3c). Otherwise it returns immediately.
// No stack args → plain `c3` ret; the tail call keeps ECX = this.

class C {
    int pad[5];
    int field_14;
public:
    void guard();
    void teardown();   // FUN_004317c0
};

void C::guard() {
    if (field_14 == 0)
        teardown();
}
