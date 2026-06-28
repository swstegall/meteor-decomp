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
// FUNCTION: ffxivgame 0x0004d740 — guarded virtual dispatch + global save (35 B)
//
// Null-guards a __thiscall virtual call to vtable slot 17 (offset 0x44)
// of the global object pointer at 0x0132cf40.  Before the call, stores
// param_1 into a global DWORD at 0x01266dfc.  The first virtual-call
// argument is the address of a global at 0x0132cf60; the second is param_1.
//
// Calling convention: __stdcall — 1 DWORD arg (param_1), RET 0x4.
// Frame: none (/Oy — ECX holds 'this' live across the entire body).
//
// Asm (35 bytes @ orig RVA 0x0004d740):
//   8b 0d 40 cf 32 01     MOV ECX, [g_d740_obj]
//   85 c9                 TEST ECX, ECX
//   74 16                 JZ → RET
//   8b 44 24 04           MOV EAX, [ESP+4]         ; param_1
//   8b 11                 MOV EDX, [ECX]            ; vtable ptr
//   50                    PUSH EAX                  ; push param_1 (arg2)
//   a3 fc 6d 26 01        MOV [g_d740_saved], EAX
//   8b 42 44              MOV EAX, [EDX+0x44]       ; vtable[17]
//   68 60 cf 32 01        PUSH OFFSET g_d740_arg    ; push &g_d740_arg (arg1)
//   ff d0                 CALL EAX
//   c2 04 00              RET 0x4

struct FUN_0044d740_target {
    virtual void pad_00();   // vtable[0]  @ +0x00
    virtual void pad_01();   // vtable[1]  @ +0x04
    virtual void pad_02();   // vtable[2]  @ +0x08
    virtual void pad_03();   // vtable[3]  @ +0x0c
    virtual void pad_04();   // vtable[4]  @ +0x10
    virtual void pad_05();   // vtable[5]  @ +0x14
    virtual void pad_06();   // vtable[6]  @ +0x18
    virtual void pad_07();   // vtable[7]  @ +0x1c
    virtual void pad_08();   // vtable[8]  @ +0x20
    virtual void pad_09();   // vtable[9]  @ +0x24
    virtual void pad_10();   // vtable[10] @ +0x28
    virtual void pad_11();   // vtable[11] @ +0x2c
    virtual void pad_12();   // vtable[12] @ +0x30
    virtual void pad_13();   // vtable[13] @ +0x34
    virtual void pad_14();   // vtable[14] @ +0x38
    virtual void pad_15();   // vtable[15] @ +0x3c
    virtual void pad_16();   // vtable[16] @ +0x40
    virtual void SomeFn(void* arg1, void* arg2);  // vtable[17] @ +0x44
};

extern "C" FUN_0044d740_target* g_d740_obj;   // [0x0132cf40]
extern "C" void*                g_d740_saved;  // [0x01266dfc]
extern "C" char                 g_d740_arg;    // [0x0132cf60]

void __stdcall FUN_0044d740(void* param_1)
{
    FUN_0044d740_target* obj = g_d740_obj;
    if (obj != NULL) {
        g_d740_saved = param_1;
        obj->SomeFn(&g_d740_arg, param_1);
    }
}
