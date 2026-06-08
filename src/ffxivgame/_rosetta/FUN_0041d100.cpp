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
// FUNCTION: ffxivgame 0x0001d100 — __cdecl wrapper that table-looks-up its
//                                  index arg and forwards to FUN_004236e0
//                                  (26 B / 0x1a).
//
// Asm (26 bytes @ 0x0001d100):
//
//   8b 44 24 04              MOV  EAX, [ESP+0x4]           ; EAX = index arg
//   8b 0c 85 c0 96 f5 00     MOV  ECX, [EAX*4 + 0xf596c0]  ; ECX = g_table[index]
//   51                       PUSH ECX                       ; push as 2nd arg
//   8b 0d 7c 98 32 01        MOV  ECX, [0x0132987c]         ; ECX = this (thiscall)
//   6a 17                    PUSH 0x17                      ; push 23 as 1st arg
//   e8 c7 65 00 00           CALL FUN_004236e0              ; __thiscall, 2 int args
//   c3                       RET                            ; __cdecl, caller cleans
//
// Calling convention: __cdecl (bare RET; caller cleans the one stack arg).
// Stack frame: none.
//
// Semantics: looks up a value from the global DWORD table at VA 0x00f596c0
// using the supplied index, then calls FUN_004236e0 on the singleton object
// at VA 0x0132987c with (0x17, g_table[index]).

class CSomeManager {
public:
    void FUN_004236e0(int a, int b);
};

extern "C" {
    extern int g_table_f596c0[];        // DWORD table at VA 0x00f596c0
    extern CSomeManager *g_mgr_132987c; // singleton pointer at VA 0x0132987c
}

extern "C" void FUN_0041d100(int index)
{
    g_mgr_132987c->FUN_004236e0(0x17, g_table_f596c0[index]);
}
