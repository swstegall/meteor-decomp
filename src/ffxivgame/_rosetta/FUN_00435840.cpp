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
// FUNCTION: ffxivgame 0x00435840 — __thiscall virtual-dispatch + lazy-init
// log callback (103 B).
//
// Calling convention: __thiscall (ECX = this, 1 dword stack arg cleaned by
// callee → RET 4).
//
// Description (recovered from asm at RVA 0x00035840):
//
//   arg = [ESP+4]           — caller's first (only) stack argument
//   vtable = *arg           — vtable pointer of *arg
//   method = vtable[0x190/4]— virtual method at vtable offset 0x190
//
//   Push 5 args for the virtual call (right-to-left):
//     [ECX+0xc], [ECX+0x8], [ECX+0x10], [ECX+0x4], arg
//   Set ECX = [ECX+0x4]  (thiscall "this" for the callee)
//   CALL method
//
//   If return value == 0: return immediately (RET 4).
//
//   Otherwise: lazy-init the log-callback slot at .data[0x0132390c]
//   (bit 0 of flag at .data[0x01323910]; default fn FUN_00433720),
//   then call it with args:
//     s_00435840_str1, s_00435840_str2, s_00435840_str3, 0x11a, s_00435840_str4
//   (caller-cleans: ADD ESP,0x14 after the call)
//
// Register-allocation note: ESI is the only callee-save used (saved/restored
// around the virtual call). It holds successive ECX-field values as temps
// before they are pushed. MSVC overwrites ECX with [ECX+0x4] *after* loading
// [ECX+0x10] into ESI, preserving all four field reads from the original ECX.
//
// Why naked asm: the virtual-call setup sequence (ESI temp, ECX overwritten
// mid-push-sequence) and the lazy-init instruction encodings (TEST byte ptr
// [mem],AL; OR [mem],EAX; MOV [mem],OFFSET) cannot be reliably reproduced
// from plain C++ source without risking register-reorder divergence.
//
// Reloc-bearing sites (compare.py wildcard-masks each 4-byte window):
//   +0x2c   TEST byte ptr [g_log_flag],    AL      — .data 0x01323910
//   +0x34   OR   [g_log_flag],             EAX     — .data 0x01323910
//   +0x3a   MOV  [g_log_fptr],             OFFSET  — .data 0x0132390c
//   +0x3e   MOV  [...],  OFFSET FUN_00433720        — .text 0x00433720
//   +0x43   PUSH OFFSET s_00435840_str4             — .rdata 0xf64dd0
//   +0x4d   PUSH OFFSET s_00435840_str3             — .rdata 0xf64c18
//   +0x52   PUSH OFFSET s_00435840_str2             — .rdata 0xf64db4
//   +0x57   PUSH OFFSET s_00435840_str1             — .rdata 0xf64be8
//   +0x5d   CALL dword ptr [g_log_fptr]             — .data 0x0132390c

extern "C" {
    // .data 0x01323910 — lazy-init flag; bit 0 signals "already initialized"
    extern int  g_log_flag_00435840;

    // .data 0x0132390c — log-callback function pointer (set lazily)
    extern void *g_log_fptr_00435840;

    // .text 0x00433720 — default log implementation stored into g_log_fptr
    void FUN_00433720();

    // .rdata string literals passed to the log callback
    extern char s_00435840_str1[];  // 0xf64be8
    extern char s_00435840_str2[];  // 0xf64db4
    extern char s_00435840_str3[];  // 0xf64c18
    extern char s_00435840_str4[];  // 0xf64dd0
}

extern "C" __declspec(naked) void FUN_00435840() {
    __asm {
        // --- load arg and fetch virtual method from its vtable --------
        mov     eax, [esp + 4]                           // 8b 44 24 04
        mov     edx, [eax]                               // 8b 10
        mov     edx, [edx + 0x190]                      // 8b 92 90 01 00 00

        // --- save ESI (callee-save), then build call args from ECX ---
        push    esi                                      // 56
        mov     esi, [ecx + 0xc]                        // 8b 71 0c
        push    esi                                      // 56  — arg5
        mov     esi, [ecx + 0x8]                        // 8b 71 08
        push    esi                                      // 56  — arg4
        mov     esi, [ecx + 0x10]                       // 8b 71 10
        mov     ecx, [ecx + 0x4]                        // 8b 49 04  (overwrites this; ECX now = arg2/thiscall-this)
        push    esi                                      // 56  — arg3
        push    ecx                                      // 51  — arg2
        push    eax                                      // 50  — arg1

        // --- dispatch through the virtual method ----------------------
        call    edx                                      // ff d2
        test    eax, eax                                 // 85 c0
        pop     esi                                      // 5e
        jz      done                                     // 74 3f

        // --- lazy-init log callback at g_log_fptr --------------------
        mov     eax, 1                                   // b8 01 00 00 00
        test    byte ptr [g_log_flag_00435840], al       // 84 05 [reloc]
        jnz     log_call                                 // 75 10
        or      dword ptr [g_log_flag_00435840], eax    // 09 05 [reloc]
        mov     dword ptr [g_log_fptr_00435840], offset FUN_00433720  // c7 05 [reloc] [reloc]

    log_call:
        // --- call log callback with 5 args (caller cleans) -----------
        push    offset s_00435840_str4                   // 68 [reloc]
        push    0x11a                                    // 68 1a 01 00 00
        push    offset s_00435840_str3                   // 68 [reloc]
        push    offset s_00435840_str2                   // 68 [reloc]
        push    offset s_00435840_str1                   // 68 [reloc]
        call    dword ptr [g_log_fptr_00435840]          // ff 15 [reloc]
        add     esp, 0x14                                // 83 c4 14

    done:
        ret     4                                        // c2 04 00
    }
}
