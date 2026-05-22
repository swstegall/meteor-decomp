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
// FUNCTION: ffxivgame 0x0040a610 — lazy one-time initialiser (35 B)
//
// Guards a single call to FUN_0040b840 (a __thiscall method on the
// global object at 0x01327c48) behind a byte-sized flag at 0x01327c45.
// The first arg is a global DWORD at 0x01327c38; the second is 0.
//
// Calling convention: __cdecl — no args, void return, plain RET.
// Frame: none (/Oy — function has no locals).
//
// Asm (35 bytes @ orig RVA 0x0040a610):
//   80 3d <flag> 00    CMP byte ptr [g_a610_flag], 0x0
//   75 19              JNZ short → RET           ; already initialised
//   a1 <value>         MOV EAX, [g_a610_value]
//   6a 00              PUSH 0                    ; arg2
//   50                 PUSH EAX                  ; arg1
//   b9 <obj>           MOV ECX, OFFSET g_a610_obj ; this (thiscall)
//   e8 <rel>           CALL FUN_0040b840
//   c6 05 <flag> 01    MOV byte ptr [g_a610_flag], 0x1
//   c3                 RET

// Minimal class stub — only the calling convention matters;
// the mangled CALL target is a reloc wildcard in compare.py.
struct UNK_0040b840_receiver {
    void FUN_0040b840(int arg1, int arg2);
};

extern "C" char                    g_a610_flag;   // [0x01327c45]
extern "C" int                     g_a610_value;  // [0x01327c38]
extern "C" UNK_0040b840_receiver   g_a610_obj;    // [0x01327c48]

void __cdecl FUN_0040a610()
{
    if (g_a610_flag == '\0') {
        g_a610_obj.FUN_0040b840(g_a610_value, 0);
        g_a610_flag = '\x01';
    }
}
