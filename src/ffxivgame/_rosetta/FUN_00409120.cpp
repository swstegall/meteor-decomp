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
// FUNCTION: ffxivgame 0x00409120 — MSVC 2005 STL `_Sort` body (205 B / 0xCD)
//                                  specialised on a 64-byte-element iterator
//                                  (Dinkumware introsort: median-of-three
//                                  quicksort with `_ISORT_MAX = 32` cutoff,
//                                  heap-sort fallback once the recursion
//                                  budget is exhausted).
//
// Asm (read from asm/ffxivgame/00009120_FUN_00409120.s):
//
//   __cdecl void FUN_00409120(elem* first, elem* last, ptrdiff_t budget);
//
//     sizeof(elem) = 0x40 (the SAR-by-6 element-count math).
//
//   Outline:
//     count = (last - first) / 0x40
//     if (count <= 0x20) goto small_path
//
//     for (; ; ) {                          // outer introsort loop
//         if (budget <= 0) goto heap_path
//         pair<elem*,elem*> mid;
//         FUN_00408ae0(&mid, first, last);  // _Unguarded_partition_3
//         budget = (budget / 2) + (budget / 4)   // *= 3/4
//         lhs = (mid.first  - first) & ~0x3F     // size of left   subrange
//         rhs = (last - mid.second  ) & ~0x3F    // size of right  subrange
//         if (lhs < rhs) {
//             FUN_00409120(first, mid.first, budget)   // recurse small left
//             first = mid.second
//         } else {
//             FUN_00409120(mid.second, last, budget)   // recurse small right
//             last  = mid.first
//         }
//         count = (last - first) / 0x40
//         if (count <= 0x20) goto small_path
//     }
//
//   small_path:
//     if (count > 1) FUN_00408f10(first, last, 0);     // _Insertion_sort
//     return
//
//   heap_path:
//     if (count <= 0x20) goto small_path               // safety fallthrough
//     bytes = (last - first) & ~0x3F
//     if (bytes > 0x40)
//         FUN_00408a80(first, last, 0, 0);             // _Make_heap (4-arg variant)
//     FUN_004090b0(first, last);                       // _Sort_heap
//     return
//
//   Stack frame (after the 4 saved-reg pushes + the leading SUB ESP, 8):
//     [esp+0x00]  EDI saved
//     [esp+0x04]  ESI saved
//     [esp+0x08]  EBP saved
//     [esp+0x0c]  EBX saved
//     [esp+0x10]  partition_out.first   (lhs pivot, written by 408ae0)
//     [esp+0x14]  partition_out.second  (rhs pivot, written by 408ae0)
//     [esp+0x18]  return address
//     [esp+0x1c]  arg first
//     [esp+0x20]  arg last
//     [esp+0x24]  arg budget
//
//   Reloc-bearing call sites in the orig 205 bytes:
//     +0x2b  rel32  FUN_00408ae0 (partition helper)
//     +0x60  rel32  FUN_00409120 (self-recursion, smaller-left branch)
//     +0x6b  rel32  FUN_00409120 (self-recursion, smaller-right branch)
//     +0x8c  rel32  FUN_00408f10 (_Insertion_sort)
//     +0xb3  rel32  FUN_00408a80 (_Make_heap, 4-arg variant)
//     +0xbd  rel32  FUN_004090b0 (_Sort_heap)
//
// Reconstruction strategy — symbolic naked-asm (same shape as FUN_00408230):
//
//   Every CALL is a single COFF rel32 relocation that `tools/compare.py`
//   masks during the byte diff. The remaining bytes are encoded
//   deterministically by MSVC's inline assembler (all jumps fit in short
//   form, all ESP-relative loads use disp8, all AND-masks are
//   sign-extended imm8). The resulting .obj's `.text` is byte-identical
//   to orig modulo the masked rel32 slots — i.e. GREEN.

extern "C" {
    void FUN_00408ae0();    // _Unguarded_partition_3 helper
    void FUN_00408f10();    // _Insertion_sort
    void FUN_00408a80();    // _Make_heap (4-arg form)
    void FUN_004090b0();    // _Sort_heap
    void FUN_00409120();    // self (forward declaration for the recursion)
}

extern "C" __declspec(naked) void FUN_00409120() {
    __asm {
        sub  esp, 8
        push ebx
        mov  ebx, dword ptr [esp + 0x10]
        push ebp
        push esi
        push edi
        mov  edi, dword ptr [esp + 0x20]
        mov  eax, edi
        sub  eax, ebx
        sar  eax, 6
        cmp  eax, 0x20
        jle  short small_path
        mov  esi, dword ptr [esp + 0x24]
        nop

    outer_loop:
        test esi, esi
        jle  short heap_path
        push edi
        lea  eax, [esp + 0x14]
        push ebx
        push eax
        call FUN_00408ae0
        mov  ebp, dword ptr [esp + 0x20]
        mov  eax, esi
        cdq
        sub  eax, edx
        sar  eax, 1
        mov  esi, eax
        cdq
        sub  eax, edx
        sar  eax, 1
        add  esi, eax
        mov  eax, dword ptr [esp + 0x1c]
        mov  ecx, edi
        mov  edx, eax
        sub  ecx, ebp
        sub  edx, ebx
        add  esp, 0x0C
        and  ecx, -64
        and  edx, -64
        cmp  edx, ecx
        push esi
        jge  short right_smaller
        push eax
        push ebx
        call FUN_00409120
        mov  ebx, ebp
        jmp  short loop_tail
    right_smaller:
        push edi
        push ebp
        call FUN_00409120
        mov  edi, dword ptr [esp + 0x1c]
    loop_tail:
        mov  eax, edi
        sub  eax, ebx
        sar  eax, 6
        add  esp, 0x0C
        cmp  eax, 0x20
        jg   short outer_loop

    small_path:
        cmp  eax, 1
        jle  short small_done
        push 0
        push edi
        push ebx
        call FUN_00408f10
        add  esp, 0x0C
    small_done:
        pop  edi
        pop  esi
        pop  ebp
        pop  ebx
        add  esp, 8
        ret

    heap_path:
        cmp  eax, 0x20
        jle  short small_path
        mov  eax, edi
        sub  eax, ebx
        and  eax, -64
        cmp  eax, 0x40
        jle  short skip_make_heap
        push 0
        push 0
        push edi
        push ebx
        call FUN_00408a80
        add  esp, 0x10
    skip_make_heap:
        push edi
        push ebx
        call FUN_004090b0
        add  esp, 0x08
        pop  edi
        pop  esi
        pop  ebp
        pop  ebx
        add  esp, 8
        ret
    }
}
