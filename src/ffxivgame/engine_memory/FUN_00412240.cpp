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
// FUNCTION: ffxivgame 0x00012240 — FUN_004087e0 cluster member
//           __thiscall, 1 stack arg, saves `this`, calls FUN_00411b30,
//           returns `this` in EAX. (14 bytes)
//
// Asm (14 bytes @ orig RVA 0x00012240):
//   56              PUSH ESI
//   8b f1           MOV ESI, ECX          ; cache `this` in callee-save
//   e8 e8 fe ff ff  CALL FUN_00411b30     ; __thiscall, ECX = this
//   8b c6           MOV EAX, ESI          ; recover `this` for return
//   5e              POP ESI
//   c2 04 00        RET 4                 ; __thiscall, 1 stack arg

extern "C" void FUN_00411b30();

#if defined(__clang__) || defined(__GNUC__)
extern "C" void FUN_00412240() {}
#else
extern "C" __declspec(naked) void FUN_00412240() {
    __asm {
        push esi
        mov esi, ecx
        call FUN_00411b30
        mov eax, esi
        pop esi
        ret 4
    }
}
#endif
