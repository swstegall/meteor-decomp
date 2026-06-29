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
// FUNCTION: ffxivgame 0x00049430 — FUN_00449430 (__stdcall, 26 B / 0x1a).
//
// Two-argument __stdcall wrapper. Pre-loads arg2 into EAX and arg1 into ECX
// before the push sequence, doubles arg1 via LEA EDX,[ECX+ECX*1], then
// forwards to FUN_0044d500 (__cdecl) with fixed third argument 12 (0xc).
//
// Asm (26 bytes):
//   8b 44 24 08      MOV EAX, [ESP+0x8]        ; arg2 → EAX
//   8b 4c 24 04      MOV ECX, [ESP+0x4]        ; arg1 → ECX
//   6a 0c            PUSH 0xc                   ; push 12 (3rd arg, right-to-left)
//   50               PUSH EAX                   ; push arg2 (2nd arg)
//   8d 14 09         LEA EDX, [ECX+ECX*0x1]    ; EDX = arg1*2
//   52               PUSH EDX                   ; push arg1*2 (1st arg)
//   e8 xx xx xx xx   CALL FUN_0044d500          ; rel32 reloc (masked by diff)
//   83 c4 0c         ADD ESP, 0xc              ; caller cleans 3 args (__cdecl)
//   c2 08 00         RET 0x8                   ; __stdcall: callee cleans 2 args

extern "C" int FUN_0044d500(int a, int b, int c);

extern "C" int __stdcall FUN_00449430(int arg1, int arg2)
{
    return FUN_0044d500(arg1 * 2, arg2, 12);
}
