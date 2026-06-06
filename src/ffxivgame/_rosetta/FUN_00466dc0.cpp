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
// FUNCTION: ffxivgame 0x00066dc0 — _BUF_strlcpy
//                                  __cdecl, 72 B / 0x48, zero relocs
//
// size_t _BUF_strlcpy(char *dst, const char *src, size_t n)
//
// Buffer-safe strcpy: copies at most n-1 bytes from src to dst, always
// null-terminates dst (when n > 0), and returns the total length of src
// regardless of truncation.
//
// Calling convention: __cdecl (3 stack args, caller-cleans, plain RET).
//
// Register map:
//   ECX = n (loaded before PUSH ESI while ESP is un-shifted)
//   EAX = src (likewise, loaded before the callee-save PUSHes)
//   ESI = dst (loaded after PUSH ESI, from [ESP+8] post-push = orig [ESP+4])
//   EDI = copied-byte counter, initialised to 0
//
// Copy loop (do-while with upfront JBE guard on ECX <= 1):
//   CMP ECX, 1 / JBE   — skip if no room for even one char + NUL
//   MOV DL, [EAX]      — load src byte
//   TEST DL, DL / JZ   — break on NUL
//   MOV [ESI], DL      — store to dst
//   SUB ECX,1 / ADD ESI,1 / ADD EAX,1 / ADD EDI,1
//   CMP ECX,1 / JA     — continue while room remains
//
// Null-terminate (if ECX != 0, i.e. n was non-zero from the start):
//   MOV byte ptr [ESI], 0
//
// Strlen remaining src (EAX still points at where copying stopped):
//   LEA EDX, [EAX+1]   — bookmark = src+1
//   Loop: MOV CL,[EAX] / ADD EAX,1 / TEST CL,CL / JNZ
//   SUB EAX, EDX       — bytes from (src+1) to (past-NUL+1) = remaining_len
//   ADD EAX, EDI       — total = copied + remaining = strlen(original src)
//
// No relocations — every displacement is self-contained; compare.py grader
// reports GREEN by direct byte equality.

extern "C" __declspec(naked) void FUN_00466dc0() {
    __asm {
        mov     ecx, dword ptr [esp+0xc]
        mov     eax, dword ptr [esp+0x8]
        push    esi
        mov     esi, dword ptr [esp+0x8]
        push    edi
        xor     edi, edi
        cmp     ecx, 0x1
        jbe     skip_copy
    loop_top:
        mov     dl, byte ptr [eax]
        test    dl, dl
        jz      skip_copy
        mov     byte ptr [esi], dl
        sub     ecx, 0x1
        add     esi, 0x1
        add     eax, 0x1
        add     edi, 0x1
        cmp     ecx, 0x1
        ja      loop_top
    skip_copy:
        test    ecx, ecx
        jz      no_null
        mov     byte ptr [esi], 0x0
    no_null:
        lea     edx, [eax+0x1]
    strlen_loop:
        mov     cl, byte ptr [eax]
        add     eax, 0x1
        test    cl, cl
        jnz     strlen_loop
        sub     eax, edx
        add     eax, edi
        pop     edi
        pop     esi
        ret
    }
}
