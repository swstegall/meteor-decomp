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
// FUNCTION: ffxivgame 0x000579d0 — binary search on a sorted unsigned short array (36 B)
//
// Performs a lower-bound binary search on the sorted word array pointed to by ESI,
// searching for the unsigned short value in DI within the half-open index range [EAX, EDX).
// Returns the element's index in EAX, or -1 (0xFFFFFFFF) if the value is not present.
//
// Non-standard register-based calling convention (no prologue, no epilogue, no frame):
//   ESI  = base pointer to sorted unsigned short[] array
//   DI   = target value (unsigned short) to search for
//   EAX  = low index  (entry; modified by loop)
//   EDX  = high index (entry; modified by loop)
// Return: EAX = found index, or -1 if not found.
//
// Asm (36 bytes @ orig RVA 0x000579d0):
//   3b c2           CMP EAX, EDX              ; lo < hi?
//   7d 16           JGE short end_loop         ; if lo >= hi, exit
//   8d 0c 10        LEA ECX, [EAX + EDX*1]    ; ECX = lo + hi
//   d1 f9           SAR ECX, 1                 ; ECX = (lo + hi) >> 1  (mid)
//   66 39 3c 4e     CMP word [ESI + ECX*2], DI ; arr[mid] vs target
//   73 05           JNC short set_high         ; arr[mid] >= target: hi = mid
//   8d 41 01        LEA EAX, [ECX + 1]         ; arr[mid] < target: lo = mid + 1
//   eb 02           JMP short check_again
//   8b d1           MOV EDX, ECX              ; set_high: hi = mid
//   3b c2           CMP EAX, EDX              ; check_again: lo < hi?
//   7c ea           JL  short loop_body        ; continue loop
//   66 39 3c 46     CMP word [ESI + EAX*2], DI ; arr[lo] == target?
//   74 03           JZ  short found
//   83 c8 ff        OR  EAX, 0FFFFFFFFh        ; not found: return -1
//   c3              RET

extern "C" __declspec(naked) int __cdecl FUN_004579d0(void)
{
    __asm {
        cmp     eax, edx
        jge     short end_loop
    loop_body:
        lea     ecx, [eax + edx*1]
        sar     ecx, 1
        cmp     word ptr [esi + ecx*2], di
        jnc     short set_high
        lea     eax, [ecx + 1]
        jmp     short check_again
    set_high:
        mov     edx, ecx
    check_again:
        cmp     eax, edx
        jl      short loop_body
    end_loop:
        cmp     word ptr [esi + eax*2], di
        jz      short found
        or      eax, 0FFFFFFFFh
    found:
        ret
    }
}
