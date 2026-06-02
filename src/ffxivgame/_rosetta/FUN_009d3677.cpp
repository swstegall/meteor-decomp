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
// FUNCTION: ffxivgame 0x005d3677 (VA 0x009d3677) — swap-and-invoke helper (34 B)
//
// Saves the current value of a global slot, calls FUN_009df187 with it,
// then calls FUN_009df110 with arg1 and writes the result back to the
// global, returning the first call's return value.
//
// Calling convention: __cdecl, 1 parameter (int arg1).
// Frame: none (/Oy). ESI used to preserve FUN_009df187's return value
// across the second call.
//
// Deferred-cleanup pattern: the first PUSH ([g_009d3677_slot]) is not
// cleaned until after the second CALL; both are scrubbed together with
// two consecutive POP ECX instructions.
//
// Asm (34 bytes @ orig RVA 0x005d3677):
//   56                 PUSH ESI
//   ff 35 <addr>       PUSH DWORD PTR [g_009d3677_slot]  ; old global value
//   e8 <rel>           CALL FUN_009df187
//   ff 74 24 0c        PUSH DWORD PTR [ESP+0x0c]         ; arg1
//   8b f0              MOV ESI, EAX
//   e8 <rel>           CALL FUN_009df110
//   59                 POP ECX                            ; scrub arg1
//   59                 POP ECX                            ; scrub slot push
//   a3 <addr>          MOV DWORD PTR [g_009d3677_slot], EAX
//   8b c6              MOV EAX, ESI
//   5e                 POP ESI
//   c3                 RETN

extern "C" int FUN_009df187(int);
extern "C" int FUN_009df110(int);
extern "C" int g_009d3677_slot;  // at VA 0x01364654

extern "C" __declspec(naked) void FUN_009d3677()
{
    __asm {
        push esi
        push dword ptr [g_009d3677_slot]
        call FUN_009df187
        push dword ptr [esp + 0x0c]
        mov esi, eax
        call FUN_009df110
        pop ecx
        pop ecx
        mov dword ptr [g_009d3677_slot], eax
        mov eax, esi
        pop esi
        ret
    }
}
