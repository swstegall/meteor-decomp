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
// FUNCTION: ffxivgame 0x4344b0 — allocate-and-dispatch member fn (__thiscall, 158 B, ret 0x10).
//
// A larger sibling of FUN_00433f50: reads the same global manager pointer
// at 0x01328d90, indexes its element table (`tbl->base + (tbl->index*7)*4`
// — a 28-byte stride formed by `(index*8 - index)*4`), and uses that
// element twice. First it transforms two of the function's stack args via
// FUN_00435440(this=this) then FUN_00417a70(this=element, ...). Then it
// re-reads the global, re-indexes, and allocates a 0x14-byte object via
// FUN_00417ab0(this=element, size=0x14).
//
// On success the new object gets a vtable (0x00f649b0) and stashes four
// values (stack arg, EBP arg, EBX arg, and the FUN_00417a70 result) in its
// fields, then is handed to FUN_0043c2d0(this=this->field_0xc, obj). On
// allocation failure the same dispatch is invoked with a null pointer.
//
// Calling convention: __thiscall (ECX = this) with four stack args
// (callee-cleans 0x10 → `ret 0x10`).
//
// Naked asm: two DIR32 sites (global ptr loaded twice + vtable literal)
// plus four REL32 calls, and the `lea ?,[eax*8]` scaled-index-with-zero
// -disp form that MSVC 2005's /O2 emits. compare.py reloc-masks the
// 4-byte windows.
//
// Reloc-bearing sites:
//   +0x0e  MOV ECX,[data_01328d90]      (dir32, manager global)
//   +0x2f  CALL FUN_00435440            (rel32)
//   +0x3c  CALL FUN_00417a70            (rel32)
//   +0x41  MOV ECX,[data_01328d90]      (dir32, manager global)
//   +0x5d  CALL FUN_00417ab0            (rel32)
//   +0x6a  MOV [EAX], offset 0xf649b0   (dir32, vtable)
//   +0x80  CALL FUN_0043c2d0            (rel32)
//   +0x92  CALL FUN_0043c2d0            (rel32)

extern "C" {
    // .text — internal direct-call targets within the binary (REL32).
    int FUN_00435440();   // __thiscall(this) transform
    int FUN_00417a70();   // __thiscall(this=element, arg, arg)
    int FUN_00417ab0();   // __thiscall(this=element, size) -> void* allocator
    int FUN_0043c2d0();   // __thiscall(this, obj) dispatch
    // .data — manager table pointer.
    extern int data_01328d90;
    // .rdata — vtable literal stored into the new object.
    extern int data_00f649b0;
}

extern "C" __declspec(naked) void FUN_004344b0() {
    __asm {
        push    ebx
        mov     ebx, dword ptr [esp + 0x14]
        push    ebp
        mov     ebp, dword ptr [esp + 0x10]
        push    esi
        push    edi
        mov     edi, ecx
        mov     ecx, dword ptr [data_01328d90]
        movzx   eax, byte ptr [ecx]
        lea     edx, [eax*8]
        sub     edx, eax
        mov     eax, dword ptr [ecx + 4]
        mov     ecx, dword ptr [esp + 0x14]
        push    ebx
        push    ebp
        push    ecx
        mov     ecx, edi
        lea     esi, [eax + edx*4]
        call    FUN_00435440
        mov     edx, dword ptr [esp + 0x1c]
        push    eax
        push    edx
        mov     ecx, esi
        call    FUN_00417a70
        mov     ecx, dword ptr [data_01328d90]
        mov     esi, eax
        movzx   eax, byte ptr [ecx]
        lea     edx, [eax*8]
        sub     edx, eax
        mov     eax, dword ptr [ecx + 4]
        lea     ecx, [eax + edx*4]
        push    0x14
        call    FUN_00417ab0
        test    eax, eax
        jz      zero_path
        mov     ecx, dword ptr [esp + 0x14]
        mov     dword ptr [eax], offset data_00f649b0
        mov     dword ptr [eax + 4], ecx
        mov     dword ptr [eax + 8], ebp
        mov     dword ptr [eax + 0xc], ebx
        mov     dword ptr [eax + 0x10], esi
        mov     ecx, dword ptr [edi + 0xc]
        push    eax
        call    FUN_0043c2d0
        pop     edi
        pop     esi
        pop     ebp
        pop     ebx
        ret     0x10
    zero_path:
        mov     ecx, dword ptr [edi + 0xc]
        xor     eax, eax
        push    eax
        call    FUN_0043c2d0
        pop     edi
        pop     esi
        pop     ebp
        pop     ebx
        ret     0x10
    }
}
