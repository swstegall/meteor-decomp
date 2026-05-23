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
// FUNCTION: ffxivgame 0x00412240 — RemovableHeapSpace destructor wrapper (14 bytes / 0xe)
//           __thiscall with 1 stack arg (RET 0x4)
//
// ECX = this.  Calls FUN_00411b30 (RemovableHeapSpace destructor,
// __thiscall) with ECX = this, then returns this in EAX.
//
// Asm (14 bytes @ orig RVA 0x00012240):
//   56              PUSH ESI
//   8b f1           MOV ESI, ECX        ; cache `this` in callee-save
//   e8 e8 f8 ff ff  CALL FUN_00411b30   ; __thiscall, ECX still = this
//   8b c6           MOV EAX, ESI        ; recover `this` for return
//   5e              POP ESI
//   c2 04 00        RET 4               ; __thiscall, 1 stack arg

class RemovableHeapSpace_00412240 {
public:
    RemovableHeapSpace_00412240 *FUN_00412240(int);
    void FUN_00411b30();
};

RemovableHeapSpace_00412240 *RemovableHeapSpace_00412240::FUN_00412240(int)
{
    FUN_00411b30();
    return this;
}
