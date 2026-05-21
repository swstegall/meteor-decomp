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
// FUNCTION: ffxivgame 0x404e10 — thiscall-then-tail-jmp wrapper around a
// global singleton at 0x01323898. Forwards the caller's first arg to a
// thiscall method on the singleton, then overwrites its own arg slot with
// the singleton pointer and tail-JMPs to a cdecl helper (FUN_00452d00)
// with that pointer as the sole argument.
//
// Asm:
//   8b 44 24 04            mov  eax, [esp+4]
//   50                     push eax
//   b9 RR RR RR RR         mov  ecx, offset g_obj
//   e8 RR RR RR RR         call thiscall_helper  ; FUN_00447450
//   c7 44 24 04 RR RR RR RR  mov dword ptr [esp+4], offset g_obj
//   e9 RR RR RR RR         jmp  tail_helper      ; FUN_00452d00

extern void *g_obj;
extern "C" int  thiscall_helper();
extern "C" void __cdecl tail_helper();

extern "C" __declspec(naked) void __cdecl wrapper() {
    __asm {
        mov eax, dword ptr [esp+4]
        push eax
        mov ecx, offset g_obj
        call thiscall_helper
        mov dword ptr [esp+4], offset g_obj
        jmp tail_helper
    }
}
