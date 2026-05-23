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
// FUNCTION: ffxivgame 0x00012230 — FUN_004087e0 cluster member
//           __thiscall, 1 stack arg, saves `this`, calls FUN_00412150,
//           returns `this` in EAX. (14 bytes)
//
// This is a multi-inheritance adjustor-thunk target: FUN_00411dd0 is the
// paired adjustor (SUB ECX, 8; JMP FUN_00412230) that adjusts `this` from
// a secondary sub-object base before forwarding here.
//
// Asm (14 bytes @ orig RVA 0x00012230):
//   56              PUSH ESI
//   8b f1           MOV ESI, ECX          ; cache `this` in callee-save
//   e8 18 ff ff ff  CALL FUN_00412150     ; __thiscall, ECX = this
//   8b c6           MOV EAX, ESI          ; recover `this` for return
//   5e              POP ESI
//   c2 04 00        RET 4                 ; __thiscall, 1 stack arg

extern "C" void FUN_00412150();

#if defined(__clang__) || defined(__GNUC__)
extern "C" void FUN_00412230() {}
#else
extern "C" __declspec(naked) void FUN_00412230() {
    __asm {
        push esi
        mov esi, ecx
        call FUN_00412150
        mov eax, esi
        pop esi
        ret 4
    }
}
#endif
