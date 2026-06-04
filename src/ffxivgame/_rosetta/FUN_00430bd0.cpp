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
// FUNCTION: ffxivgame 0x00430bd0 — `__thiscall` guarded tail-call dispatcher
//                                   (18 B / 0x12).
//
// Behaviour read from the disassembly at orig RVA 0x00030bd0:
//
//   __thiscall void FUN_00430bd0(C *this) {
//       if (this->field_0x10 == 0 && this->field_0x38 == 0)
//           this->run();           // tail call -> FUN_00430aa0
//   }
//
//   Two memory compares against zero gate a tail call: if either guard
//   field is non-zero the function returns immediately; only when both
//   are zero does it forward (as a /O2 tail-call JMP) to the
//   `__thiscall` callee at 0x00430aa0, passing the unchanged `this` in
//   ECX. No stack frame, no callee-saves, no security cookie.
//
//   Asm shape (18 bytes total):
//
//     83 79 10 00        cmp  dword ptr [ecx+0x10], 0   ; field_0x10
//     75 0b              jnz  +0x0b                       ; -> ret
//     83 79 38 00        cmp  dword ptr [ecx+0x38], 0   ; field_0x38
//     75 05              jnz  +0x05                       ; -> ret
//     e9 RR RR RR RR     jmp  FUN_00430aa0                ; tail call
//     c3                 ret
//
//   The only reloc-bearing site is the `e9` REL32 tail-call to
//   FUN_00430aa0; tools/compare.py masks that 4-byte offset window
//   during the byte diff.

class C {
    char pad0[0x10];
    int field_0x10;        // offset 0x10
    char pad1[0x24];       // 0x14 .. 0x37
    int field_0x38;        // offset 0x38
public:
    void check();
    void run();
};

void C::check() {
    if (field_0x10 == 0 && field_0x38 == 0)
        run();
}
