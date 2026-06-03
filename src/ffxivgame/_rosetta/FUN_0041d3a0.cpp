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
// FUNCTION: ffxivgame 0x0001d3a0 — FUN_0041d3a0 (__cdecl, 180 B / 0xb4)
//
// Log/format helper: allocates a 2 KB stack buffer, formats a message into it
// via _snprintf_s (one of two format strings depending on whether the fifth
// argument is non-null), then passes the result to a logging sink via an IAT
// indirect call with tag 6.  /GS cookie is materialized at [ESP+0x800].
//
// Inferred signature:
//   void FUN_0041d3a0(int p1, int p2, int p3, int p4, int p5)
//
// Branch shape:
//   if (p5 != 0):
//     _snprintf_s(buf,0x800,0x7ff, fmt1, p3, p4, p5, p1, p2)
//   else:
//     _snprintf_s(buf,0x800,0x7ff, fmt2, p3, p4)
//   (*g_iat_log_2651b4)(buf, 6)
//   g_global_1d3a0 = 0          // scheduled between /GS XOR and check
//
// Reloc-bearing sites (compare.py wildcards these 4-byte windows):
//   +0x07  __security_cookie load  (.data 0x012ea8b0)
//   +0x41  fmt1 push               (.rdata 0x00f59800)
//   +0x59  CALL rel32 _snprintf_s  (0x009d4f9f)
//   +0x63  fmt2 push               (.rdata 0x00f59820)
//   +0x78  CALL rel32 _snprintf_s  (0x009d4f9f)
//   +0x89  CALL [IAT] log sink     ([0x012651b4])
//   +0x9b  g_global_1d3a0 addr     (.data, zeros in binary)
//   +0xa7  CALL rel32 _sec_check   (0x009d20f4)

extern "C" {
    // .data — /GS security cookie
    extern unsigned __security_cookie;

    // .rdata — format strings (two variants, 0x20 bytes apart)
    extern char g_fmt_1d3a0_p5[];       // 0x00f59800 — p5-present branch
    extern char g_fmt_1d3a0_nop5[];     // 0x00f59820 — p5-absent  branch

    // .idata — IAT slot for the logging sink
    extern int g_iat_log_2651b4;        // 0x012651b4

    // .data — global written to 0 in the epilog
    extern int g_global_1d3a0;

    // CRT — _snprintf_s (rel32 CALL target)
    int g_snprintf_s_9d4f9f();
    // CRT — __security_check_cookie (rel32 CALL target)
    void g_sec_check_9d20f4();
}

extern "C" __declspec(naked) void FUN_0041d3a0() {
    __asm {
        // ---- prologue: allocate frame + /GS cookie ----------------------
        sub     esp, 0x804
        mov     eax, __security_cookie          // a1 <reloc>
        xor     eax, esp
        mov     dword ptr [esp + 0x800], eax    // store cookie

        // ---- read params, test p5 (fifth arg) ---------------------------
        mov     edx, dword ptr [esp + 0x80c]    // p2
        mov     eax, dword ptr [esp + 0x818]    // p5
        test    eax, eax
        mov     ecx, dword ptr [esp + 0x808]    // p1
        push    esi
        mov     esi, dword ptr [esp + 0x814]    // p3  (after PUSH ESI → +0x814)
        push    edx                             // p2 onto stack
        push    ecx                             // p1 onto stack
        jz      branch_nop5

        // ---- branch: p5 != 0 (7-arg snprintf) ---------------------------
        push    eax                             // p5
        mov     eax, dword ptr [esp + 0x824]    // p4  (after 4 pushes)
        push    eax                             // p4
        push    esi                             // p3
        push    offset g_fmt_1d3a0_p5           // fmt1  68 <reloc>
        push    0x7ff
        lea     ecx, [esp + 0x20]               // buf (bottom of frame)
        push    0x800
        push    ecx
        call    g_snprintf_s_9d4f9f
        add     esp, 0x24
        jmp     tail

        // ---- branch: p5 == 0 (4-arg snprintf) ---------------------------
    branch_nop5:
        mov     edx, dword ptr [esp + 0x820]    // p4  (after 3 pushes)
        push    edx                             // p4
        push    esi                             // p3
        push    offset g_fmt_1d3a0_nop5         // fmt2  68 <reloc>
        push    0x7ff
        lea     eax, [esp + 0x1c]               // buf
        push    0x800
        push    eax
        call    g_snprintf_s_9d4f9f
        add     esp, 0x20

        // ---- tail: pass formatted buf to log sink -----------------------
    tail:
        lea     ecx, [esp + 0x4]                // buf (only ESI push remains)
        push    0x6
        push    ecx
        call    dword ptr [g_iat_log_2651b4]    // ff 15 <reloc>
        mov     ecx, dword ptr [esp + 0x80c]    // reload cookie (after 2 pushes)
        add     esp, 0x8
        pop     esi
        xor     ecx, esp
        mov     dword ptr [g_global_1d3a0], 0   // c7 05 <reloc> 00000000
        call    g_sec_check_9d20f4
        add     esp, 0x804
        ret
    }
}
