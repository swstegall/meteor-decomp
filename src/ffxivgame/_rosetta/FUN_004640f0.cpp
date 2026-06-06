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
// FUNCTION: ffxivgame 0x004640f0 — sk_delete_ptr
//                                  OpenSSL STACK delete-by-value: linear scan
//                                  for a pointer value, remove it by shifting
//                                  subsequent elements, decrement count, and
//                                  return the removed element (0/NULL if not
//                                  found). (92 B / 0x5c, __cdecl, no calls.)
//
// Asm shape (92 bytes — from asm/ffxivgame/000640f0_sk_delete_ptr.s,
// RVA 0x000640f0..0x0006414b):
//
//   __cdecl void* sk_delete_ptr(OPENSSL_STACK* st, void* ptr);
//
//   struct OPENSSL_STACK {
//       int   num;    // +0: element count
//       int** data;   // +4: pointer to array of pointers
//   };
//
//   Pseudocode:
//     EDX = st; ESI = st->num;
//     if (ESI <= 0) return NULL;
//     EDI = st->data; EBX = ptr;
//     ECX = EDI; EAX = 0;           // ECX = running scan ptr; EAX = index
//     do {
//         if (*ECX == ptr) goto found;
//         EAX++; ECX += 4;
//     } while (EAX < ESI);
//     return NULL;
//   found:
//     if (EAX < 0 || EAX >= ESI) return NULL;  // defensive bounds check
//     EDI = st->data[EAX];           // save element
//     ESI--;                         // count - 1
//     if (EAX < ESI)                 // shift if not last element
//         for (; EAX < ESI; EAX++)
//             st->data[EAX] = st->data[EAX+1];
//     st->num--;
//     return EDI;
//
// Notes:
//   - The search loop uses ECX as a running pointer (starts at array base,
//     incremented by 4 per step); EAX is the parallel index.
//   - The `MOV EDI,EDI` at +0x3e is a 2-byte alignment NOP (8b ff) emitted
//     by MSVC 2005 at the top of the shift loop block.
//   - The shift loop reloads the array base from memory each iteration
//     (MOV ECX,[EDX+4]) — this is the pattern MSVC emitted here.
//   - No external calls → no COFF relocations → naked asm produces a
//     byte-identical obj without the compare.py reloc masking path.

extern "C" __declspec(naked) void FUN_004640f0() {
    __asm {
        mov     edx, dword ptr [esp + 4]
        push    ebx
        push    esi
        mov     esi, dword ptr [edx]
        xor     eax, eax
        test    esi, esi
        push    edi
        jle     not_found
        mov     edi, dword ptr [edx + 4]
        mov     ebx, dword ptr [esp + 0x14]
        mov     ecx, edi
    search_loop:
        cmp     dword ptr [ecx], ebx
        jz      found
        add     eax, 1
        add     ecx, 4
        cmp     eax, esi
        jl      search_loop
    not_found:
        pop     edi
        pop     esi
        xor     eax, eax
        pop     ebx
        ret
    found:
        test    eax, eax
        jl      not_found
        cmp     eax, esi
        jge     not_found
        mov     edi, dword ptr [edi + eax*4]
        add     esi, -1
        cmp     eax, esi
        jge     do_decrement
        mov     edi, edi
    shift_loop:
        mov     ecx, dword ptr [edx + 4]
        mov     ebx, dword ptr [ecx + eax*4 + 4]
        lea     ecx, [ecx + eax*4]
        add     eax, 1
        cmp     eax, esi
        mov     dword ptr [ecx], ebx
        jl      shift_loop
    do_decrement:
        add     dword ptr [edx], -1
        mov     eax, edi
        pop     edi
        pop     esi
        pop     ebx
        ret
    }
}
