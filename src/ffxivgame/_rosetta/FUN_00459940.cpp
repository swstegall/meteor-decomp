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
// FUNCTION: ffxivgame 0x00459940 — __cdecl wide-character memory compare
//                                  returning -1/0/1 (56 bytes).
//
// int FUN_00459940(const unsigned short *s1, const unsigned short *s2, int n)
//
// Compares n unsigned-short (wide-char) elements of s1 and s2 in order.
// Returns:
//    0  if all n elements are equal (or n == 0)
//   -1  if the first differing element satisfies s1[i] < s2[i] (unsigned)
//   +1  if the first differing element satisfies s1[i] > s2[i] (unsigned)
//
// Register layout (after PUSH ESI):
//   EDX  count n  (loaded at entry, before the PUSH, while [ESP+0xc] == arg3)
//   ECX  ptr2     (loaded after PUSH,  when [ESP+0xc] == arg2)
//   EAX  ptr1     (loaded after PUSH,  when [ESP+0x8] == arg1)
//   SI   scratch for 16-bit word loads / comparisons
//
// The PUSH ESI is intentionally hoisted between TEST EDX,EDX and the JBE:
// MSVC's scheduler inserts it there because PUSH does not clobber EFLAGS,
// giving the processor a free slot while the branch unit resolves the
// condition code.  The displacement 0x0c then addresses arg2 (ptr2) after
// the push -- the same displacement that addressed arg3 (n) before it.
//
// The not-equal arm uses SBB EAX,EAX + AND EAX,-2 + ADD EAX,1 to fold the
// signed-comparison result into ±1 without an extra branch:
//   CMP AX,[ECX]         ; CF=1 iff *s1 < *s2 (unsigned)
//   SBB EAX,EAX          ; EAX = 0 or -1
//   AND EAX,-2           ; 0→0, -1→-2
//   ADD EAX,1            ; 0→1, -2→-1
//
// Calling convention: __cdecl (three DWORD stack args; caller cleans; RET).
//
// Naked __asm preserves the instruction scheduling (PUSH between TEST and
// JBE), the 16-bit operand-size prefixes (66h), and the short-form (cb)
// conditional-branch encodings.

extern "C" __declspec(naked) void FUN_00459940() {
    __asm {
        mov     edx, dword ptr [esp + 0x0c]
        test    edx, edx
        push    esi
        jbe     equal_lbl
        mov     ecx, dword ptr [esp + 0x0c]
        mov     eax, dword ptr [esp + 0x08]
    loop_lbl:
        mov     si, word ptr [eax]
        cmp     si, word ptr [ecx]
        jnz     neq_lbl
        add     eax, 2
        add     ecx, 2
        sub     edx, 1
        jnz     loop_lbl
    equal_lbl:
        xor     eax, eax
        pop     esi
        ret
    neq_lbl:
        movzx   eax, word ptr [eax]
        cmp     ax, word ptr [ecx]
        pop     esi
        sbb     eax, eax
        and     eax, -2
        add     eax, 1
        ret
    }
}
