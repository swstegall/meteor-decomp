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
// FUNCTION: ffxivgame 0x00051c50 — Utf8String SSO init + assign_substr
//                                  delegate (50 B / 0x32, RET 0xC).
//
// Calling convention: 3 stack args, callee-cleans via `ret 0xC`.
//   ECX at entry  = src Utf8String* (forwarded as first arg to inner call)
//   [esp+ 4] / ESI  = Utf8String* result (the string object to initialise)
//   [esp+ 8] / EDX  = pos   (passed as second arg to assign_substr)
//   [esp+0xC] / EAX = count (passed as third arg to assign_substr)
//
// Body:
//   1. Zero the SSO fields of *result:
//        result->size     (+0x14) = 0
//        result->capacity (+0x18) = 0x0F   (15 = SSO threshold)
//        result->buf[0]   (+0x04) = '\0'
//   2. Also zeroes the PUSH ECX stack slot (local_0 at [esp+4] post-saves)
//      — a dead store artifact of the PUSH ECX "cheap sub esp,4" idiom.
//   3. Tail-delegates: result->assign_substr(src=ECX, pos, count)
//      via `__thiscall` call to FUN_00404040 (Utf8String::assign_substr).
//   4. Returns result (ESI) in EAX.
//
// Stack trace (entry ESP = 0):
//   after PUSH ECX  [esp]=local_0  [esp+8]=param1/ESI  [esp+C]=param2/EDX
//   after PUSH ESI  [esp]=ESI_save [esp+4]=local_0 [esp+C]=param1 [esp+14]=param3
//
// All non-CALL bytes are position-independent; the CALL rel32 to
// FUN_00404040 is masked by tools/compare.py.

extern "C" void FUN_00404040(void);

extern "C" __declspec(naked) void FUN_00451c50() {
    __asm {
        push    ecx
        mov     edx, dword ptr [esp + 0xc]
        push    esi
        mov     esi, dword ptr [esp + 0xc]
        xor     eax, eax
        mov     dword ptr [esi + 0x14], eax
        mov     dword ptr [esi + 0x18], 0xf
        mov     dword ptr [esp + 0x4], eax
        mov     byte ptr [esi + 0x4], al
        mov     eax, dword ptr [esp + 0x14]
        push    eax
        push    edx
        push    ecx
        mov     ecx, esi
        call    FUN_00404040
        mov     eax, esi
        pop     esi
        pop     ecx
        ret     0xc
    }
}
