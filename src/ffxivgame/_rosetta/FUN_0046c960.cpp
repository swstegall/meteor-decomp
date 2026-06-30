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
// FUNCTION: ffxivgame 0x0006c960 — __cdecl 3-arg wrapper that forwards to
//                                  FUN_00464e40 with a hardcoded 4th arg of 0
//                                  (26 B / 0x1a).
//
// Asm (26 bytes @ 0x0006c960):
//   8b 44 24 0c    MOV EAX, [ESP+0xc]      ; arg3
//   8b 4c 24 08    MOV ECX, [ESP+0x8]      ; arg2
//   8b 54 24 04    MOV EDX, [ESP+0x4]      ; arg1
//   6a 00          PUSH 0x0                ; push hardcoded 4th arg = 0
//   50             PUSH EAX               ; push arg3
//   51             PUSH ECX               ; push arg2
//   52             PUSH EDX               ; push arg1
//   e8 ?? ?? ?? ?? CALL FUN_00464e40      ; rel32 reloc
//   83 c4 10       ADD  ESP, 0x10         ; callee-args cleanup (4 × 4 bytes)
//   c3             RET                    ; __cdecl bare return
//
// No stack frame — ESP-relative addressing throughout.
// Return value from FUN_00464e40 (EAX) flows through unchanged.

extern "C" int FUN_00464e40(int, int, int, int);

extern "C" int FUN_0046c960(int arg1, int arg2, int arg3)
{
    return FUN_00464e40(arg1, arg2, arg3, 0);
}
