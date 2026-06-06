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
// FUNCTION: ffxivgame 0x0001c210 — __cdecl 1-arg wrapper that issues 4 calls
//                                  to FUN_004236e0 on the global object at
//                                  [0x0132987c] (78 B / 0x4e).
//
// Takes a single int argument, masks it to the low 4 bits (& 0xf), then
// calls FUN_004236e0 four times with command codes 0xa8, 0xbe, 0xbf, 0xc0
// (the first arg) and the masked value (the second arg).
//
// Calling conventions:
//   - This function : __cdecl  (plain RET; caller reclaims the 1 dword arg)
//   - Callee FUN_004236e0 : __thiscall (ECX = this, RET 8 cleans 2 args)
//   - Global at 0x0132987c : holds a pointer to the receiver object
//
// Asm shape (78 bytes @ RVA 0x0001c210):
//
//   0001c210  8b 0d 7c 98 32 01   MOV ECX,[0x0132987c]   ; preload for 1st call
//   0001c216  56                  PUSH ESI                ; save callee-saved ESI
//   0001c217  8b 74 24 08         MOV ESI,[ESP+0x8]       ; load arg (after PUSH ESI)
//   0001c21b  83 e6 0f            AND ESI,0xf             ; mask to low 4 bits
//   0001c21e  56                  PUSH ESI                ; arg2 for 1st call
//   0001c21f  68 a8 00 00 00      PUSH 0xa8               ; arg1 for 1st call
//   0001c224  e8 b7 74 00 00      CALL FUN_004236e0
//   0001c229  8b 0d 7c 98 32 01   MOV ECX,[0x0132987c]
//   0001c22f  56                  PUSH ESI
//   0001c230  68 be 00 00 00      PUSH 0xbe
//   0001c235  e8 a6 74 00 00      CALL FUN_004236e0
//   0001c23a  8b 0d 7c 98 32 01   MOV ECX,[0x0132987c]
//   0001c240  56                  PUSH ESI
//   0001c241  68 bf 00 00 00      PUSH 0xbf
//   0001c246  e8 95 74 00 00      CALL FUN_004236e0
//   0001c24b  8b 0d 7c 98 32 01   MOV ECX,[0x0132987c]
//   0001c251  56                  PUSH ESI
//   0001c252  68 c0 00 00 00      PUSH 0xc0
//   0001c257  e8 84 74 00 00      CALL FUN_004236e0
//   0001c25c  5e                  POP ESI
//   0001c25d  c3                  RET

struct _Global_0132987c_Host {
    void FUN_004236e0(int a, int b);
};

extern _Global_0132987c_Host* g_0132987c;

extern "C" void __cdecl FUN_0041c210(int param)
{
    int masked = param & 0xf;
    g_0132987c->FUN_004236e0(0xa8, masked);
    g_0132987c->FUN_004236e0(0xbe, masked);
    g_0132987c->FUN_004236e0(0xbf, masked);
    g_0132987c->FUN_004236e0(0xc0, masked);
}
