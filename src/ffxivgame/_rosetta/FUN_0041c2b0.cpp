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
// FUNCTION: ffxivgame 0x0001c2b0 — __cdecl 1-arg trampoline that forwards
//                                  to FUN_004236e0 on the global object
//                                  at [0x0132987c] (20 B / 0x14).
//
// Asm (20 bytes @ 0x0001c2b0):
//   0f b6 44 24 04     MOVZX EAX, byte ptr [ESP+4]   ; zero-extend unsigned char arg
//   8b 0d 7c 98 32 01  MOV ECX, [0x0132987c]         ; load global object pointer
//   50                 PUSH EAX                       ; push arg (rightmost)
//   6a 0e              PUSH 0xe                       ; push 14 (leftmost)
//   e8 1d 74 00 00     CALL FUN_004236e0              ; __thiscall (ECX = this)
//   c3                 RET                            ; __cdecl: no stack cleanup
//
// Conventions:
//   - This function: __cdecl (plain RET, single unsigned char arg)
//   - Callee FUN_004236e0: __thiscall (ECX = this, RET 8 cleans 2 args)
//   - Global at 0x0132987c: holds a pointer to the receiver object
//
// Sibling: identical structure to FUN_0041c270 (same global, same callee,
// same arg pattern — only the command constant differs: 7 there, 14 here).

struct _Global_0132987c_Host {
    void FUN_004236e0(int a, int b);
};

extern _Global_0132987c_Host* g_0132987c;

extern "C" void __cdecl FUN_0041c2b0(unsigned char arg) {
    g_0132987c->FUN_004236e0(0xe, arg);
}
