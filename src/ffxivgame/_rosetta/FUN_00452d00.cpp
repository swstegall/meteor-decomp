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
// FUNCTION: ffxivgame 0x00052d00 — strip trailing separator character
//                                  (__cdecl, 71 B / 0x47)
//
// void __cdecl FUN_00452d00(SomeObj *arg1)
//
//   Checks if the CountedStr object referenced by *arg1 ends with a specific
//   character identified by the literal pointer at DAT_0132d030 and the
//   starting-position global at DAT_00f67298.  If the find (FUN_00446fd0)
//   result equals (length - 1), erases that last character via
//   FUN_004460a0(pos = length-1, count = 1).
//
//   arg1 points to an object whose first field ([ESI]) is a char* (string
//   data pointer).  An early-exit tests the first byte of that pointer for
//   NUL (empty string).
//
// Calling convention: __cdecl (one stack arg, caller-cleans, void return).
// Stack frame: -4 (PUSH ESI only).
//
// Reloc sites (masked by tools/compare.py):
//   off +0x0e  DIR32 → DAT_00f67298
//   off +0x15  DIR32 → DAT_0132d030
//   off +0x1c  REL32 → FUN_00446fd0
//   off +0x25  REL32 → FUN_00445e50
//   off +0x36  REL32 → FUN_00445e50
//   off +0x41  REL32 → FUN_004460a0
//
// FUN_00446fd0 is a __thiscall 2-arg trampoline that ends with `ret 8`,
// so it pops its own two stack args; no caller-side add-esp needed.
// FUN_004460a0 is similarly expected to clean its two pushed args.

extern "C" {
    int DAT_00f67298;
    int DAT_0132d030;
    void FUN_00446fd0();
    void FUN_00445e50();
    void FUN_004460a0();
}

extern "C" __declspec(naked) void FUN_00452d00() {
    __asm {
        push    esi
        mov     esi, dword ptr [esp+0x8]
        mov     eax, dword ptr [esi]
        cmp     byte ptr [eax], 0
        jz      tail
        mov     ecx, dword ptr [DAT_00f67298]
        push    edi
        push    ecx
        push    OFFSET DAT_0132d030
        mov     ecx, esi
        call    FUN_00446fd0
        mov     ecx, esi
        mov     edi, eax
        call    FUN_00445e50
        sub     eax, 1
        cmp     edi, eax
        pop     edi
        jnz     tail
        push    1
        mov     ecx, esi
        call    FUN_00445e50
        sub     eax, 1
        push    eax
        mov     ecx, esi
        call    FUN_004460a0
    tail:
        pop     esi
        ret
    }
}
