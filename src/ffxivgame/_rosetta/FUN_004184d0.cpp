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
// FUNCTION: ffxivgame 0x000184d0 — 1-arg __cdecl wrapper: pushes arg twice,
//                                  calls FUN_0041b930, clears global flag byte
//                                  (22 B / 0x16)
//
// Asm (22 bytes @ orig RVA 0x000184d0):
//   8b 44 24 04              MOV  EAX, dword ptr [ESP+0x4]    ; load arg
//   50                       PUSH EAX                          ; push arg
//   50                       PUSH EAX                          ; push arg again
//   e8 55 34 00 00           CALL FUN_0041b930                 ; rel32 reloc
//   83 c4 08                 ADD  ESP, 0x8                     ; cdecl cleanup
//   c6 05 d6 8e 32 01 00     MOV  byte ptr [0x01328ed6], 0x0  ; DIR32 reloc
//   c3                       RET
//
// Calling convention: __cdecl (caller-cleans, plain RET). One stack arg.
// No prologue — /Oy; leaf-ish with no callee-saved register usage.
//
// FUN_0041b930 (11 B) is itself a global-object thunk: it loads a `this`
// pointer from [0x0132987c] into ECX and tail-JMPs to FUN_004232c0.
// The two push EAX calls pass the same argument as both stack params to
// the ultimate thiscall target.
//
// The global byte at VA 0x01328ed6 is cleared to 0 on every call. It
// appears to be an enable/dirty flag in the same global allocator-
// statistics block documented in FUN_00416320 (which uses addresses in
// the 0x01328dxx range nearby).

extern "C" void FUN_0041b930(int a, int b);
extern "C" char g_flag_004184d0;   // VA 0x01328ed6

extern "C" __declspec(naked) void FUN_004184d0()
{
    __asm {
        mov  eax, dword ptr [esp+4]
        push eax
        push eax
        call FUN_0041b930
        add  esp, 8
        mov  byte ptr [g_flag_004184d0], 0
        ret
    }
}
