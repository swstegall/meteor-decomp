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
// FUNCTION: ffxivgame 0x000184b0 — 0-arg __cdecl wrapper: pushes two constant
//                                  1s, calls FUN_0041b930(1,1), clears global
//                                  flag byte (20 B / 0x14)
//
// Asm (20 bytes @ orig RVA 0x000184b0):
//   6a 01                        PUSH 0x1                         ; arg2 = 1
//   6a 01                        PUSH 0x1                         ; arg1 = 1
//   e8 77 34 00 00                CALL FUN_0041b930                ; rel32 reloc
//   83 c4 08                     ADD  ESP, 0x8                    ; cdecl cleanup
//   c6 05 d6 8e 32 01 00         MOV  byte ptr [0x01328ed6], 0x0  ; DIR32 reloc
//   c3                           RET
//
// Calling convention: __cdecl (caller-cleans, plain RET). No stack args.
// No prologue — /Oy; no callee-saved register usage.
//
// This is the simplest variant in the FUN_0041b930 wrapper family: unlike
// FUN_004184d0 (pushes one arg twice) and FUN_00418290 (pushes two distinct
// args), this variant always hard-codes (1, 1). All three wrappers clear
// the same enable/dirty flag byte at VA 0x01328ed6 (g_flag_004184d0) on
// return.

extern "C" void FUN_0041b930(int a, int b);
extern "C" char g_flag_004184d0;   // VA 0x01328ed6

extern "C" __declspec(naked) void FUN_004184b0()
{
    __asm {
        push 1
        push 1
        call FUN_0041b930
        add  esp, 8
        mov  byte ptr [g_flag_004184d0], 0
        ret
    }
}
