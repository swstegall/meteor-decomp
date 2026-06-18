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
// FUNCTION: ffxivgame 0x00035370 — __thiscall COM spin-wait (69 B / 0x45).
//
// Calling convention: __thiscall (ECX = this, 1 stack arg ignored, RET 4).
//
// this->field_4 is a COM-style C-interface object (C-style vtable where
// `this` is the first explicit stack argument).  vtable[7] (offset 0x1c)
// is a __stdcall function with signature:
//
//   HRESULT __stdcall Method7(obj*, int, int, int);
//
// called as Method7(field_4, 0, 0, 1).  The callee cleans all 4 args via
// its own RET 16.
//
// Flow:
//   1. Call field_4->vtable[7](field_4, 0, 0, 1).
//   2. If S_OK (0)             → return.
//   3. If any non-zero result  → enter wait loop:
//        a. If EAX == 0x88760868 (D3DERR_DEVICELOST) → break → return.
//        b. Sleep(0).
//        c. Call field_4->vtable[7](field_4, 0, 0, 1) again.
//        d. If result != 0     → goto 3a.
//        e. (result == 0)      → fall through → return.
//
// MSVC 2005 codegen detail: the literal 1 (rightmost / last arg for the
// virtual call) is pre-pushed at function entry (offset +0x01) BEFORE
// MOV ESI, ECX.  The scheduler hoists the constant push ahead of the
// this-save because it has no register dependency on ECX.
//
// The IAT slot at 0x00f3e1c8 is Sleep — the same slot used by siblings
// FUN_00424820 and FUN_00405080.
//
// Reloc-bearing sites (4-byte windows masked by tools/compare.py):
//   off 0x1b  IMAGE_REL_I386_DIR32 → __imp__Sleep@4  (IAT @ 0xf3e1c8)

extern "C" {

__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);

__declspec(naked) void FUN_00435370() {
    __asm {
        push    esi                          ; 56
        push    0x1                          ; 6a 01
        mov     esi, ecx                     ; 8b f1
        mov     eax, dword ptr [esi + 0x4]   ; 8b 46 04
        mov     ecx, dword ptr [eax]         ; 8b 08
        mov     edx, dword ptr [ecx + 0x1c]  ; 8b 51 1c
        push    0x0                          ; 6a 00
        push    0x0                          ; 6a 00
        push    eax                          ; 50
        call    edx                          ; ff d2
        test    eax, eax                     ; 85 c0
        jz      end_fn                       ; 74 29
        push    edi                          ; 57
        mov     edi, dword ptr [Sleep]       ; 8b 3d [IAT]
        nop                                  ; 90
    loop_top:
        cmp     eax, 0x88760868              ; 3d 68 08 76 88
        jz      pop_edi_end                  ; 74 19
        push    0x0                          ; 6a 00
        call    edi                          ; ff d7
        mov     eax, dword ptr [esi + 0x4]   ; 8b 46 04
        mov     ecx, dword ptr [eax]         ; 8b 08
        mov     edx, dword ptr [ecx + 0x1c]  ; 8b 51 1c
        push    0x1                          ; 6a 01
        push    0x0                          ; 6a 00
        push    0x0                          ; 6a 00
        push    eax                          ; 50
        call    edx                          ; ff d2
        test    eax, eax                     ; 85 c0
        jnz     loop_top                     ; 75 e0
    pop_edi_end:
        pop     edi                          ; 5f
    end_fn:
        pop     esi                          ; 5e
        ret     4                            ; c2 04 00
    }
}

} // extern "C"
