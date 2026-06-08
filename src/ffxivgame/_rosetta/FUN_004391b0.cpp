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
// FUNCTION: ffxivgame 0x004391b0 — four-arg pass-through + global snapshot (54 bytes)
//
// void __cdecl FUN_004391b0(int a, int b, int c, int d)
//
// Saves the four incoming parameters into four global slots, then forwards
// them to FUN_0041d050.  No return value.  Only ESI is callee-saved
// (used to hold param d across the store/call sequence).
//
// Globals written (in order of store, not address):
//   [0x01328ef8] = a   (EAX — short A3 encoding, 5 bytes)
//   [0x01328efc] = b   (ECX — 89 /r encoding, 6 bytes)
//   [0x01328f04] = c   (EDX — 89 /r encoding, 6 bytes; note 8-byte gap from prev)
//   [0x01328f08] = d   (ESI — 89 /r encoding, 6 bytes)
//
// Instruction shape (MSVC 2005 scheduler idiom):
//   1. Pre-load c→EDX, b→ECX, a→EAX from the original stack frame.
//   2. PUSH ESI to save the callee-save register.
//   3. Load d→ESI from [ESP+0x14] (shifted by the ESI push).
//   4. Push d, c, b, a onto the stack (call arguments in reverse).
//   5. Store EAX, ECX, EDX, ESI to globals while registers are still live.
//   6. CALL FUN_0041d050.
//   7. ADD ESP, 0x10 (clean 4 call args — __cdecl, caller cleans inner call).
//   8. POP ESI.
//   9. RET (outer frame is __cdecl; callee does nothing).
//
// Naked __asm so the load-order (c,b,a then d), the store-after-push
// interleave, and the A3 vs 89-form encoding for the four MOV-to-global
// instructions all pin to the original 54-byte sequence.
// tools/compare.py masks the four abs32 global relocations and the
// CALL rel32 for FUN_0041d050.

extern "C" void FUN_0041d050();
extern "C" int DAT_01328ef8;
extern "C" int DAT_01328efc;
extern "C" int DAT_01328f04;
extern "C" int DAT_01328f08;

extern "C" __declspec(naked) void FUN_004391b0() {
    __asm {
        mov     edx, dword ptr [esp + 0xc]
        mov     ecx, dword ptr [esp + 0x8]
        mov     eax, dword ptr [esp + 0x4]
        push    esi
        mov     esi, dword ptr [esp + 0x14]
        push    esi
        push    edx
        push    ecx
        push    eax
        mov     dword ptr [DAT_01328ef8], eax
        mov     dword ptr [DAT_01328efc], ecx
        mov     dword ptr [DAT_01328f04], edx
        mov     dword ptr [DAT_01328f08], esi
        call    FUN_0041d050
        add     esp, 0x10
        pop     esi
        ret
    }
}
