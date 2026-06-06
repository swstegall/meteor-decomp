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
// FUNCTION: ffxivgame 0x00694e9a (VA 0x00a94e9a) — __cdecl, 2 args, 131 B
//
// Searches a virtual container for an item whose 16-byte key at
// data[+0x20] matches arg1.  On a hit, dispatches arg2 to
// item->vtable[0xf4] and returns the result (0 if zero).
// Returns 0 when the container is empty or no item matches.
//
// Calling convention: __cdecl (no `ret N`; two stack args).
//
// Stack layout after the four initial saves (EBX/EBP/ESI/EDI):
//   [ESP+0x14]  arg1 — pointer to 16-byte key
//   [ESP+0x18]  arg2 — forwarded to matched item's vtable slot
//
// Two distinct MSVC-optimised epilogues in the original binary:
//   "not-found" (+0x60):  POP EDI/ESI/EBP; XOR EAX,EAX; POP EBX; RET
//   "done"      (+0x7e):  XOR EAX,EAX; POP EDI/ESI/EBP/EBX; RET
//
// NOP at +0x25 is MSVC /O2 loop-alignment padding.
//
// Relocs masked by compare.py:
//   REL32 +0x05  →  FUN_00a886e0  (singleton getter,  VA 0x00a886e0)
//   REL32 +0x4e  →  FUN_009d5475  (16-byte comparison, VA 0x009d5475)

extern "C" void FUN_00a886e0();   // singleton getter — no args, cdecl
extern "C" int  FUN_009d5475();   // 16-byte comparison — 3 args, cdecl

extern "C" __declspec(naked) void FUN_00a94e9a()
{
    __asm {
        push    ebx
        push    ebp
        push    esi
        push    edi

        // --- get container via singleton ---
        call    FUN_00a886e0               // singleton → EAX
        mov     edx, dword ptr [eax]       // vtable ptr
        mov     ecx, eax
        mov     eax, dword ptr [edx + 0xc] // vtable slot 3
        call    eax                        // → container

        // --- get count ---
        mov     edi, eax                   // EDI = container
        mov     edx, dword ptr [edi]
        mov     eax, dword ptr [edx + 0x40] // vtable slot 0x10
        mov     ecx, edi
        call    eax                        // → count

        mov     ebp, eax                   // EBP = count
        xor     ebx, ebx                   // EBX = 0  (index)
        test    ebp, ebp
        jbe     short return_zero_path     // if count == 0, return 0

        nop                                // loop-alignment pad (MSVC /O2)

    loop_top:
        mov     edx, dword ptr [edi]
        mov     eax, dword ptr [edx + 0x38] // vtable slot 0xe
        push    ebx                         // push index
        mov     ecx, edi
        call    eax                         // → item ptr (thiscall, callee cleans)

        mov     esi, eax                    // ESI = item
        test    esi, esi
        jz      short increment             // null item → skip

        // --- compare item's key ---
        mov     edx, dword ptr [esi]
        mov     eax, dword ptr [edx + 0x160] // vtable slot 0x58
        mov     ecx, esi
        call    eax                          // → data ptr

        mov     ecx, dword ptr [esp + 0x14]  // arg1 (key)
        push    0x10                         // size = 16
        push    ecx                          // arg1
        add     eax, 0x20                    // data + 0x20
        push    eax
        call    FUN_009d5475                 // compare(data+0x20, key, 16)
        add     esp, 0xc                     // cdecl caller cleanup
        test    eax, eax
        jz      short found                  // 0 → match

    increment:
        add     ebx, 1
        cmp     ebx, ebp
        jc      short loop_top              // unsigned: i < count → loop

        // "not-found" epilogue — XOR between POP EBP and POP EBX (MSVC idiom)
        pop     edi
        pop     esi
        pop     ebp
        xor     eax, eax
        pop     ebx
        ret

    found:
        mov     edx, dword ptr [esi]
        mov     eax, dword ptr [esp + 0x18]  // arg2
        mov     edx, dword ptr [edx + 0xf4]  // vtable slot 0x3d
        push    eax                           // push arg2
        mov     ecx, esi
        call    edx                           // item->vtable[0xf4](arg2); callee cleans
        test    eax, eax
        jnz     short done                    // non-zero → return result

    return_zero_path:
        xor     eax, eax                      // also: entry from JBE at top

    done:
        pop     edi
        pop     esi
        pop     ebp
        pop     ebx
        ret
    }
}

// vim: ts=4 sts=4 sw=4 et
