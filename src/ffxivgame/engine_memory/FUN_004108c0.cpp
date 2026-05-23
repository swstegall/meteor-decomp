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
// FUNCTION: ffxivgame 0x004108c0 — `this`-cached call-and-return-self
// Same 14-byte __thiscall pattern as FUN_004087e0 (_rosetta cluster
// representative); only the CALL target differs.
//
// Asm (14 bytes):
//   56              PUSH ESI
//   8b f1           MOV ESI, ECX        ; cache `this` in callee-save
//   e8 38 ff ff ff  CALL FUN_00410800   ; reloc — inner method
//   8b c6           MOV EAX, ESI        ; recover `this` for return
//   5e              POP ESI
//   c2 04 00        RET 4               ; __thiscall, 1 stack arg

class C {
public:
    C *do_thing(int unused);
    void inner();
};

C *C::do_thing(int) {
    inner();
    return this;
}
