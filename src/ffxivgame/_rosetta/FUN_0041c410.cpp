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
// FUNCTION: ffxivgame 0x0001c410 — __cdecl 2-arg trampoline (40 B)
//
// Selects a context object: if the second argument (param2) is non-NULL it is
// used directly; otherwise the global pointer at 0x01329874 provides the
// fallback.  Reads field_0x34 from that object and forwards to FUN_004231c0 on
// the global receiver at [0x0132987c], passing (param1 + 0x101) and the field.
//
// Calling convention: __cdecl (plain RET, caller cleans 2 args).
// Frame: none (/Oy — no locals on stack).
//
// Asm (40 bytes @ orig RVA 0x0001c410):
//   8b 44 24 08        MOV EAX, [ESP+0x8]         ; EAX = param2
//   85 c0              TEST EAX, EAX               ; NULL check
//   75 05              JNZ +5                      ; skip fallback if non-NULL
//   a1 74 98 32 01     MOV EAX, [g_c410_fallback]  ; EAX = global fallback ptr
//   8b 40 34           MOV EAX, [EAX+0x34]         ; EAX = obj->field_34
//   8b 4c 24 04        MOV ECX, [ESP+0x4]          ; ECX = param1
//   81 c1 01 01 00 00  ADD ECX, 0x101              ; ECX = param1 + 0x101
//   50                 PUSH EAX                    ; push field_34 (arg2 to callee)
//   51                 PUSH ECX                    ; push param1+0x101 (arg1 to callee)
//   8b 0d 7c 98 32 01  MOV ECX, [g_c410_receiver]  ; this = global receiver
//   e8 89 6d 00 00     CALL FUN_004231c0            ; __thiscall (RET 8 cleans 2 args)
//   c3                 RET

struct _c410_CtxObj {
    char pad[0x34];
    int field_34;
};

struct _c410_Receiver {
    void FUN_004231c0(int a, int b);
};

extern _c410_CtxObj*   g_c410_fallback;   // [0x01329874]
extern _c410_Receiver* g_c410_receiver;   // [0x0132987c]

extern "C" void __cdecl FUN_0041c410(int param1, _c410_CtxObj* param2)
{
    if (!param2)
        param2 = g_c410_fallback;
    g_c410_receiver->FUN_004231c0(param1 + 0x101, param2->field_34);
}
