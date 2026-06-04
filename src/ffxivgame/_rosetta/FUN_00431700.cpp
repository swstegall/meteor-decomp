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
// FUNCTION: ffxivgame 0x00431700 — guarded tail-call.
//
// Asm (12 bytes):
//   83 79 14 00     CMP  dword ptr [ECX + 0x14], 0
//   75 05           JNZ  +5                       ; field set -> nothing to do
//   e9 ?? ?? ?? ??  JMP  inner                    ; tail-call (reloc)
//   c3              RET
//
// __thiscall member: if the cached field at +0x14 is non-zero the work
// is already done and it returns immediately; otherwise it tail-jumps to
// the rebuild helper (same `this` in ECX, no args, void return) so MSVC
// folds the trailing call into a JMP.

class C {
    char gap[0x14];
    int flag;
public:
    void run();
    void rebuild();
};

void C::run() {
    if (flag == 0)
        rebuild();
}
