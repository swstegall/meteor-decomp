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
// FUNCTION: ffxivgame 0x00418290 — 2-arg __cdecl wrapper: loads two stack
//                                  args into EAX/ECX, pushes both, calls
//                                  FUN_0041b930, clears global flag byte
//                                  (26 B / 0x1a)
//
// Structurally identical to its 1-arg sibling FUN_004184d0 (22 B) except
// this variant loads two *different* arguments from [ESP+4] and [ESP+8] and
// forwards them independently to FUN_0041b930, whereas FUN_004184d0 pushes
// the single arg twice. Both clear the same enable/dirty flag byte at
// VA 0x01328ed6 on return.
//
// Asm (26 bytes @ orig RVA 0x00018290):
//   8b 44 24 08              MOV  EAX, dword ptr [ESP+0x8]    ; load arg2
//   8b 4c 24 04              MOV  ECX, dword ptr [ESP+0x4]    ; load arg1
//   50                       PUSH EAX                          ; push arg2
//   51                       PUSH ECX                          ; push arg1
//   e8 91 36 00 00           CALL FUN_0041b930                 ; rel32 reloc
//   83 c4 08                 ADD  ESP, 0x8                     ; cdecl cleanup
//   c6 05 d6 8e 32 01 00     MOV  byte ptr [0x01328ed6], 0x0  ; DIR32 reloc
//   c3                       RET
//
// Calling convention: __cdecl (caller-cleans, plain RET). Two stack args.
// No prologue — /Oy; no callee-saved register usage.
//
// FUN_0041b930 (11 B) is a global-object thunk: loads `this` from
// [0x0132987c] into ECX and tail-JMPs to FUN_004232c0.
// The global byte at VA 0x01328ed6 is the same enable/dirty flag cleared
// by FUN_004184d0 — see that file's comment for context.

extern "C" void FUN_0041b930(int a, int b);
extern "C" char g_flag_004184d0;   // VA 0x01328ed6

extern "C" __declspec(naked) void FUN_00418290()
{
    __asm {
        mov  eax, dword ptr [esp+8]
        mov  ecx, dword ptr [esp+4]
        push eax
        push ecx
        call FUN_0041b930
        add  esp, 8
        mov  byte ptr [g_flag_004184d0], 0
        ret
    }
}
