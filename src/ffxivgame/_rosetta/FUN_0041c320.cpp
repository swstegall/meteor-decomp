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
// FUNCTION: ffxivgame 0x0001c320 — __cdecl 2-arg trampoline that forwards
//                                  to FUN_004236e0 on the global object
//                                  at [0x0132987c] (19 B / 0x13).
//
// Asm (19 bytes @ 0x0001c320):
//   8b 44 24 08        MOV EAX, dword ptr [ESP+8]    ; load second arg (int)
//   8b 0d 7c 98 32 01  MOV ECX, [0x0132987c]         ; load global object pointer
//   50                 PUSH EAX                       ; push arg2 (rightmost)
//   6a 3b              PUSH 0x3b                      ; push 59 (leftmost)
//   e8 ae 73 00 00     CALL FUN_004236e0              ; __thiscall (ECX = this)
//   c3                 RET                            ; __cdecl: no stack cleanup
//
// Conventions:
//   - This function: __cdecl (plain RET, ignores arg1, uses arg2 as int)
//   - Callee FUN_004236e0: __thiscall (ECX = this, RET 8 cleans 2 args)
//   - Global at 0x0132987c: holds a pointer to the receiver object
//
// Siblings: FUN_0041c270 and FUN_0041c2b0 use the identical structure but
// load a single unsigned char from [ESP+4] via MOVZX and push different
// constants (7 and 14 respectively). This function instead loads a full int
// from [ESP+8] (the second argument, arg1 being unused) and pushes 0x3b.

struct _Global_0132987c_Host {
    void FUN_004236e0(int a, int b);
};

extern _Global_0132987c_Host* g_0132987c;

extern "C" void __cdecl FUN_0041c320(int, int arg2) {
    g_0132987c->FUN_004236e0(0x3b, arg2);
}
