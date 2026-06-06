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
// FUNCTION: ffxivgame 0x0004f5e0 — __cdecl locale-aware string compare
//                                  (153 bytes / 0x99, ret 0).
//
// __cdecl int compare(const char *a /*arg0*/, const char *b /*arg1*/,
//                     unsigned int max /*arg2*/, char fold /*arg3*/):
//   EDI = a, ESI = b, EBP = b (saved start of b for the run-length test).
//   EBX selects one of two global locale/collation tables:
//     fold != 0 -> 0xf67648, fold == 0 -> 0xf67600.
//
//   Per iteration FUN_0044f4b0 (__thiscall, ECX = &out8, EAX = ptr,
//   stack arg = table) decodes one element from each string into an
//   8-byte struct {dword lo; dword hi;} and returns the byte length
//   consumed (added to the running pointer). The two decoded structs
//   are compared as 64-bit collation keys: hi compared signed, lo
//   compared unsigned. Equal keys continue the loop; otherwise return
//   -1 / +1. Returns 0 when both strings end together or the byte run
//   on b reaches `max`.
//
// CALL targets (REL32, wildcarded by tools/compare.py):
//   +0x3d, +0x4b  CALL FUN_0044f4b0  — decode one element
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//   The function reads its arguments at fixed ESP displacements after a
//   bespoke prologue (SUB ESP,0x10 / CMP arg3 / four register pushes),
//   spills two decode results into the reserved 16-byte local block, and
//   threads the table pointer through two transient PUSH/CALL/ADD ESP,8
//   windows. Source-level C++ at /O2 will not reproduce the exact
//   register allocation or the interleaved 64-bit-key compare, so — per
//   the ffxivgame convention — emit naked .text matching orig modulo the
//   two REL32 call windows.

extern "C" {
    // .text — internal direct-call target within the binary (REL32).
    int FUN_0044f4b0();   // __thiscall: decode one element, returns length
}

extern "C" __declspec(naked) void FUN_0044f5e0() {
    __asm {
        sub     esp, 0x10                       // 83 ec 10
        cmp     byte ptr [esp + 0x20], 0        // 80 7c 24 20 00
        push    ebx                             // 53
        push    ebp                             // 55
        push    esi                             // 56
        push    edi                             // 57
        mov     ebx, 0xf67648                   // bb 48 76 f6 00
        jnz     short keep_table                // 75 05
        mov     ebx, 0xf67600                   // bb 00 76 f6 00
    keep_table:
        mov     esi, dword ptr [esp + 0x28]     // 8b 74 24 28
        mov     edi, dword ptr [esp + 0x24]     // 8b 7c 24 24
        mov     ebp, esi                        // 8b ee
    loop_top:
        cmp     byte ptr [edi], 0               // 80 3f 00
        jnz     short decode                    // 75 05
        cmp     byte ptr [esi], 0               // 80 3e 00
        jz      short ret_zero                  // 74 63
    decode:
        mov     eax, esi                        // 8b c6
        sub     eax, ebp                        // 2b c5
        cmp     eax, dword ptr [esp + 0x2c]     // 3b 44 24 2c
        jnc     short ret_zero                  // 73 59
        push    ebx                             // 53
        lea     ecx, [esp + 0x14]               // 8d 4c 24 14
        mov     eax, edi                        // 8b c7
        call    FUN_0044f4b0                    // e8 8e fe ff ff
        add     edi, eax                        // 03 f8
        push    ebx                             // 53
        lea     ecx, [esp + 0x20]               // 8d 4c 24 20
        mov     eax, esi                        // 8b c6
        call    FUN_0044f4b0                    // e8 80 fe ff ff
        mov     ecx, dword ptr [esp + 0x20]     // 8b 4c 24 20
        mov     edx, dword ptr [esp + 0x24]     // 8b 54 24 24
        add     esi, eax                        // 03 f0
        mov     eax, dword ptr [esp + 0x18]     // 8b 44 24 18
        add     esp, 0x8                         // 83 c4 08
        cmp     eax, ecx                        // 3b c1
        jnz     short do_cmp                    // 75 06
        cmp     dword ptr [esp + 0x14], edx     // 39 54 24 14
        jz      short loop_top                  // 74 b7
    do_cmp:
        cmp     dword ptr [esp + 0x14], edx     // 39 54 24 14
        jg      short ret_one                   // 7f 11
        jl      short ret_neg                   // 7c 04
        cmp     eax, ecx                        // 3b c1
        jnc     short ret_one                   // 73 0b
    ret_neg:
        pop     edi                             // 5f
        pop     esi                             // 5e
        pop     ebp                             // 5d
        or      eax, 0xffffffff                 // 83 c8 ff
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
    ret_zero:
        pop     edi                             // 5f
        pop     esi                             // 5e
        pop     ebp                             // 5d
        xor     eax, eax                        // 33 c0
        pop     ebx                             // 5b
        add     esp, 0x10                        // 83 c4 10
        ret                                     // c3
    }
}
