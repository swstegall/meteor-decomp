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
// FUNCTION: ffxivgame 0x0004b2a0 — descriptor binary search (__cdecl, 94 B).
//
// void* FUN_0044b2a0(unsigned short key)
//
// Binary-searches a sorted array of descriptors. The array element count is
// stored at global data_0132cb94. For each midpoint probe the function calls
// (*data_0132cb88)->vtable[2](mid) to obtain a pointer to that element, then
// compares the uint16_t at element+0x8 against `key`.
//
// On a match: calls vtable[2](mid) a second time and returns the result
//             (the matched element pointer).
// On no match (or empty array): returns NULL (XOR EAX,EAX).
//
// Calling convention: __cdecl — argument at [ESP+4] before prologue,
// [ESP+0x14] inside (after 4 callee-saves: EBX, EBP, ESI, EDI).
// The vtable method is __thiscall (ECX = object, 1 int arg), callee
// cleans with RET 4; no extra ADD ESP needed in the caller.
//
// Globals:
//   0x0132cb94 — count of descriptors
//   0x0132cb88 — pointer to manager object (vtable at *[obj])
//
// Sibling FUN_0044b350 calls this function as its first substantive step.

extern "C" {
    extern int data_0132cb94;
    extern int data_0132cb88;
}

extern "C" __declspec(naked) void* FUN_0044b2a0() {
    __asm {
        push    ebx
        push    ebp
        push    esi
        push    edi
        mov     edi, dword ptr [data_0132cb94]
        xor     ebx, ebx
        add     edi, -1
        js      not_found
        mov     ebp, dword ptr [esp + 0x14]
    loop_top:
        mov     ecx, dword ptr [data_0132cb88]
        lea     eax, [edi + ebx]
        cdq
        sub     eax, edx
        mov     esi, eax
        mov     eax, dword ptr [ecx]
        mov     edx, dword ptr [eax + 8]
        sar     esi, 1
        push    esi
        call    edx
        movzx   eax, word ptr [eax + 8]
        cmp     ax, bp
        jz      found
        jbe     go_lo
        lea     edi, [esi - 1]
        jmp     check_loop
    go_lo:
        lea     ebx, [esi + 1]
    check_loop:
        cmp     ebx, edi
        jle     loop_top
    not_found:
        pop     edi
        pop     esi
        pop     ebp
        xor     eax, eax
        pop     ebx
        ret
    found:
        mov     ecx, dword ptr [data_0132cb88]
        mov     eax, dword ptr [ecx]
        mov     edx, dword ptr [eax + 8]
        push    esi
        call    edx
        pop     edi
        pop     esi
        pop     ebp
        pop     ebx
        ret
    }
}
