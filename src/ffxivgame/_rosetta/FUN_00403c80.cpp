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
// FUNCTION: ffxivgame 0x403c80 — 4-arg __cdecl wrapper around _memmove_s
// that returns the dest pointer (1st arg). Sits at orig RVA 0x00003c80
// (33 B). The inner call lands on the `_memmove_s` slot at orig RVA
// 0x005d186e — adjacent siblings include the unmatched `_memcpy_s`
// neighbor at 0x005d17f3, so this is part of the MSVC 2005 secure-CRT
// memmove-family thunks compiled in.
//
// Behaviour:
//
//     void *FUN_00403c80(void *dest, rsize_t destsz,
//                        const void *src, rsize_t count) {
//         memmove_s(dest, destsz, src, count);
//         return dest;
//     }
//
// MSVC 2005 emits this exact prologue (eax/ecx/edx pre-loads followed
// by push esi / mov esi, [esp+8] for the saved-arg-via-esi idiom that
// preserves the dest pointer across the call) when the inlined-helper
// pattern lowers through this particular wrapper.
//
// Asm: 8b 44 24 10 8b 4c 24 0c 8b 54 24 08 56 8b 74 24 08
//      50 51 52 56 e8 RR RR RR RR 83 c4 10 8b c6 5e c3

extern "C" int memmove_s_thunk();

extern "C" __declspec(naked) void FUN_00403c80() {
    __asm {
        mov     eax, [esp+0x10]
        mov     ecx, [esp+0x0c]
        mov     edx, [esp+0x08]
        push    esi
        mov     esi, [esp+0x08]
        push    eax
        push    ecx
        push    edx
        push    esi
        call    memmove_s_thunk
        add     esp, 0x10
        mov     eax, esi
        pop     esi
        ret
    }
}
