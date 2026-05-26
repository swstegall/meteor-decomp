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
// FUNCTION: ffxivgame 0x000110f0 — engine_memory allocator-stage dispatcher
//                                  (__thiscall, 1 stack arg, 415 bytes)
//
// Calls a `this` vtable probe (vtbl[0xb]/[0xd]), lazily binds a logging
// callback at 0x0132390c on first reachable failure, then walks the
// arg0 allocator chain (vtbl[5] → vtbl[1]) to obtain a memory block,
// computes an aligned end (`(base + size - 1) & ~(size - 1)`), iterates
// the intrusive doubly-linked list anchored at &this->field_0x44
// invoking FUN_00410730 on each non-flagged node, then inserts the
// freshly-allocated block back into the list and ends with a vtbl[0xc]
// tail-call.
//
// Internal branches and the three relative-CALL targets (FUN_00410730,
// FUN_004109a0, FUN_00410510) are too intertwined with the stack-frame
// reload pattern that MSVC 2005 picks here to express cleanly in C++
// without losing the byte layout, so this is written as a naked-asm
// transcription of the prologue + flow.

extern "C" void FUN_00410730();
extern "C" void FUN_004109a0();
extern "C" void FUN_00410510();

extern "C" unsigned char DAT_01323910;
extern "C" void *DAT_0132390C;

extern "C" __declspec(naked) void FUN_004110f0() {
    __asm {
        sub     esp, 8
        push    ebx
        push    ebp
        push    esi
        push    edi
        mov     edi, ecx
        mov     eax, [edi]
        mov     edx, [eax+2Ch]
        call    edx
        mov     eax, [edi]
        mov     edx, [eax+34h]
        mov     ecx, edi
        call    edx
        test    al, al
        jz      lbl_11146
        test    byte ptr [DAT_01323910], 1
        jnz     lbl_11127
        or      dword ptr [DAT_01323910], 1
        mov     dword ptr [DAT_0132390C], 040F8E0h
    lbl_11127:
        push    0F56C60h
        push    6Ch
        push    0F56BE8h
        push    0F54D48h
        push    0F56BD8h
        call    dword ptr [DAT_0132390C]
        add     esp, 14h
    lbl_11146:
        mov     ecx, [esp+1Ch]
        mov     eax, [ecx]
        mov     edx, [eax+14h]
        call    edx
        mov     edx, [eax]
        mov     ecx, eax
        mov     eax, [edx+4]
        call    eax
        mov     esi, eax
        mov     eax, [esi+20h]
        mov     ecx, [esi+1Ch]
        mov     edx, [esi+30h]
        mov     ebx, [edx+18h]
        add     ebx, [esi+2Ch]
        lea     ebp, [ecx+eax-1]
        mov     ecx, [esi+10h]
        add     eax, -1
        not     eax
        and     ebp, eax
        cmp     dword ptr [edi+58h], 0
        mov     eax, [esi+0Ch]
        mov     [esp+10h], eax
        mov     [esp+14h], ecx
        jz      lbl_111AA
        mov     ecx, [esp+1Ch]
        mov     edx, [ecx]
        mov     eax, [edx+10h]
        call    eax
        mov     ecx, [esp+1Ch]
        mov     edx, [ecx]
        push    eax
        mov     eax, [edx+8]
        call    eax
        mov     ecx, [edi+58h]
        push    eax
        call    ecx
        add     esp, 8
    lbl_111AA:
        mov     edx, [edi+24h]
        push    ebp
        push    0CCh
        push    ebx
        call    edx
        mov     ecx, [esp+1Ch]
        lea     eax, [edi+44h]
        add     esp, 0Ch
        cmp     ecx, eax
        jz      lbl_11200
        mov     eax, [ecx]
        mov     edx, [eax+4]
        call    edx
        cmp     byte ptr [eax+28h], 0
        jnz     lbl_11200
        mov     ecx, [esp+10h]
        mov     edx, [ecx+4]
        mov     ecx, [eax+30h]
        mov     ebx, [ecx+18h]
        mov     ecx, [eax+20h]
        add     ebx, [eax+2Ch]
        mov     [esp+10h], edx
        mov     edx, [eax+1Ch]
        lea     edx, [edx+ecx-1]
        add     ecx, -1
        not     ecx
        and     edx, ecx
        push    eax
        mov     ecx, edi
        add     ebp, edx
        call    FUN_00410730
    lbl_11200:
        mov     ecx, [esp+14h]
        lea     eax, [edi+44h]
        cmp     ecx, eax
        jz      lbl_11233
        mov     eax, [ecx]
        mov     edx, [eax+4]
        call    edx
        cmp     byte ptr [eax+28h], 0
        jnz     lbl_11233
        mov     ecx, [eax+20h]
        mov     edx, [eax+1Ch]
        lea     edx, [edx+ecx-1]
        add     ecx, -1
        not     ecx
        and     edx, ecx
        push    eax
        mov     ecx, edi
        add     ebp, edx
        call    FUN_00410730
    lbl_11233:
        push    esi
        mov     ecx, edi
        call    FUN_00410730
        mov     ecx, [edi+10h]
        call    FUN_004109a0
        test    eax, eax
        jz      lbl_11264
        push    0
        push    0
        push    0
        push    0
        push    1
        push    ebp
        push    ebx
        push    edi
        mov     ecx, eax
        call    FUN_00410510
        test    eax, eax
        jz      lbl_11264
        add     eax, 8
        jmp     lbl_11266
    lbl_11264:
        xor     eax, eax
    lbl_11266:
        mov     ecx, [esp+10h]
        mov     edx, [ecx+8]
        mov     [edx+4], eax
        mov     edx, [ecx+8]
        mov     [eax+8], edx
        mov     [eax+4], ecx
        mov     [ecx+8], eax
        mov     eax, [edi]
        mov     edx, [eax+30h]
        mov     ecx, edi
        call    edx
        pop     edi
        pop     esi
        pop     ebp
        pop     ebx
        add     esp, 8
        ret     4
    }
}
