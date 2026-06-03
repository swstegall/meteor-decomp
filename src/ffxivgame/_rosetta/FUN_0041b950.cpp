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
// FUNCTION: ffxivgame 0x0001b950 — null-guarded double-dereference getter
//                                  (__cdecl, 26 B).
//
// Loads a singleton pointer from the global at VA 0x01329428, null-guards it,
// dereferences the sub-pointer at +0x150, null-guards again, then returns the
// 4-byte value at +0x1C in the inner object. Both null-guard paths share a
// single merged XOR EAX,EAX / RET epilogue at the END of the function, with
// two forward JZ branches.
//
// Calling convention: __cdecl (no arguments, no frame, bare RET).
// Return type:        void * (4-byte EAX).
//
// Asm (26 bytes @ 0x0041b950):
//   A1 28 94 32 01         MOV  EAX, [g_singleton]   ; load global ptr (A1 moffs32)
//   85 C0                  TEST EAX, EAX
//   74 0E                  JZ   null_ret              ; forward +0x0E
//   8B 80 50 01 00 00      MOV  EAX, [EAX+0x150]     ; load nested ptr (disp32)
//   85 C0                  TEST EAX, EAX
//   74 04                  JZ   null_ret              ; forward +0x04
//   8B 40 1C               MOV  EAX, [EAX+0x1C]      ; load result (disp8)
//   C3                     RET
//   33 C0   (null_ret:)    XOR  EAX, EAX             ; merged null return
//   C3                     RET

// Global singleton pointer at VA 0x01329428 (RVA 0x00F29428, section .data).
extern "C" void *g_FUN_0041b950_singleton;

extern "C" __declspec(naked) void *FUN_0041b950()
{
    __asm {
        mov  eax, dword ptr [g_FUN_0041b950_singleton]
        test eax, eax
        jz   null_ret
        mov  eax, dword ptr [eax + 0x150]
        test eax, eax
        jz   null_ret
        mov  eax, dword ptr [eax + 0x1c]
        ret
    null_ret:
        xor  eax, eax
        ret
    }
}
