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
// FUNCTION: ffxivgame 0x0004f680 — __cdecl locale-aware string compare,
//                                  3-argument variant (141 bytes / 0x8d,
//                                  ret -1/0/+1).
//
// __cdecl int FUN_0044f680(const char *str1, const char *str2, char fold):
//   ESI = str1, EDI = str2.
//   EBX selects one of two global locale/collation tables:
//     fold != 0 -> 0xf67648, fold == 0 -> 0xf67600.
//
//   Structurally identical to the 4-argument sibling FUN_0044f5e0 except:
//     1. Only 3 parameters (no `max` run-length bound).
//     2. `fold` (param3) is at [ESP+0x1c] before callee-saves vs
//        [ESP+0x20] in the 4-arg form.
//     3. loop_top checks [ESI] (str1) first, then [EDI] (str2), and the
//        two FUN_0044f4b0 decode calls are ordered str1 then str2 (the
//        4-arg form does the reverse: b first, a second).
//     4. The 64-bit hi words (EDX = str1.hi, EBP = str2.hi) are loaded
//        into registers BEFORE `ADD ESP, 8`, so both equality and
//        ordering comparisons use register-to-register CMP EDX,EBP
//        rather than the stack-spill form [ESP+0x14] used by FUN_0044f5e0.
//     5. Return epilogues appear in the order: -1 (0x4f6eb),
//        0 (0x4f6f6), +1 (0x4f700).
//
//   Per iteration FUN_0044f4b0 (custom register cc: ECX = &out8,
//   EAX = ptr, stack arg = table) decodes one element from each string
//   into an 8-byte struct {dword lo; dword hi;} and returns byte count
//   consumed (added to the running pointer). The two decoded structs are
//   compared as 64-bit collation keys: hi compared signed (EDX vs EBP),
//   lo compared unsigned (EAX vs ECX). Equal keys loop; otherwise -1/+1.
//   Returns 0 when both strings end together.
//
// CALL targets (REL32, wildcarded by tools/compare.py):
//   +0x31, +0x3f  CALL FUN_0044f4b0  — decode one element
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//   The bespoke prologue (SUB ESP,0x10 / CMP [ESP+0x1c] / four register
//   pushes), the interleaved PUSH/LEA/MOV/CALL/ADD-ESP windows, and the
//   register-based 64-bit comparison cannot be reproduced exactly from
//   source-level C++ at /O2. Naked asm emitting the original byte
//   sequence (modulo the two REL32 call windows) is the pragmatic match.

extern "C" {
    int FUN_0044f4b0();   // custom cc: ECX=&out8, EAX=ptr, stack arg=table
}

extern "C" __declspec(naked) void FUN_0044f680() {
    __asm {
        sub     esp, 0x10                       // 83 ec 10
        cmp     byte ptr [esp + 0x1c], 0        // 80 7c 24 1c 00
        push    ebx                             // 53
        push    ebp                             // 55
        push    esi                             // 56
        push    edi                             // 57
        mov     ebx, 0xf67648                   // bb 48 76 f6 00
        jnz     short keep_table                // 75 05
        mov     ebx, 0xf67600                   // bb 00 76 f6 00
    keep_table:
        mov     edi, dword ptr [esp + 0x28]     // 8b 7c 24 28
        mov     esi, dword ptr [esp + 0x24]     // 8b 74 24 24
    loop_top:
        cmp     byte ptr [esi], 0               // 80 3e 00
        jnz     short decode                    // 75 05
        cmp     byte ptr [edi], 0               // 80 3f 00
        jz      short ret_zero                  // 74 4c
    decode:
        push    ebx                             // 53
        lea     ecx, [esp + 0x14]               // 8d 4c 24 14
        mov     eax, esi                        // 8b c6
        call    FUN_0044f4b0                    // e8 fa fd ff ff
        add     esi, eax                        // 03 f0
        push    ebx                             // 53
        lea     ecx, [esp + 0x20]               // 8d 4c 24 20
        mov     eax, edi                        // 8b c7
        call    FUN_0044f4b0                    // e8 ec fd ff ff
        mov     ecx, dword ptr [esp + 0x20]     // 8b 4c 24 20
        mov     edx, dword ptr [esp + 0x1c]     // 8b 54 24 1c
        mov     ebp, dword ptr [esp + 0x24]     // 8b 6c 24 24
        add     edi, eax                        // 03 f8
        mov     eax, dword ptr [esp + 0x18]     // 8b 44 24 18
        add     esp, 0x8                         // 83 c4 08
        cmp     eax, ecx                        // 3b c1
        jnz     short ne_case                   // 75 04
        cmp     edx, ebp                        // 3b d5
        jz      short loop_top                  // 74 bf
    ne_case:
        cmp     edx, ebp                        // 3b d5
        jg      short ret_one                   // 7f 1b
        jl      short ret_neg                   // 7c 04
        cmp     eax, ecx                        // 3b c1
        jnc     short ret_one                   // 73 15
    ret_neg:
        pop     edi                             // 5f
        pop     esi                             // 5e
        pop     ebp                             // 5d
        or      eax, 0xffffffff                 // 83 c8 ff
        pop     ebx                             // 5b
        add     esp, 0x10                        // 83 c4 10
        ret                                     // c3
    ret_zero:
        pop     edi                             // 5f
        pop     esi                             // 5e
        pop     ebp                             // 5d
        xor     eax, eax                        // 33 c0
        pop     ebx                             // 5b
        add     esp, 0x10                        // 83 c4 10
        ret                                     // c3
    ret_one:
        pop     edi                             // 5f
        pop     esi                             // 5e
        pop     ebp                             // 5d
        mov     eax, 1                          // b8 01 00 00 00
        pop     ebx                             // 5b
        add     esp, 0x10                        // 83 c4 10
        ret                                     // c3
    }
}
