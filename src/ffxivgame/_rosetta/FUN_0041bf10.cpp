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
// FUNCTION: ffxivgame 0x0001bf10 — __cdecl 3-arg trampoline that forwards
//                                  to FUN_00423050 on the global object
//                                  at [0x0132987c] (27 B / 0x1b).
//
// Asm (27 bytes @ 0x0001bf10):
//   8b 44 24 0c           MOV EAX, dword ptr [ESP+0xc]  ; arg3
//   8b 4c 24 08           MOV ECX, dword ptr [ESP+0x8]  ; arg2
//   8b 54 24 04           MOV EDX, dword ptr [ESP+0x4]  ; arg1
//   50                    PUSH EAX                       ; push arg3 (rightmost)
//   51                    PUSH ECX                       ; push arg2
//   8b 0d 7c 98 32 01     MOV ECX, [0x0132987c]         ; load global object pointer
//   52                    PUSH EDX                       ; push arg1 (leftmost)
//   e8 26 71 00 00        CALL FUN_00423050              ; __thiscall (ECX = this)
//   c3                    RET                            ; __cdecl: no stack cleanup
//
// Conventions:
//   - This function: __cdecl (plain RET, three int args)
//   - Callee FUN_00423050: __thiscall (ECX = this, callee-cleans 3 stack args)
//   - Global at 0x0132987c: holds a pointer to the receiver object

struct _Global_0132987c_Host {
    void FUN_00423050(int arg1, int arg2, int arg3);
};

extern _Global_0132987c_Host* g_0132987c;

extern "C" void __cdecl FUN_0041bf10(int arg1, int arg2, int arg3) {
    g_0132987c->FUN_00423050(arg1, arg2, arg3);
}
