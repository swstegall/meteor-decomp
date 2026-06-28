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
// FUNCTION: ffxivgame 0x0043bfc0 — conditional debug-log formatter
// (__cdecl void FUN_0043bfc0(arg1, arg2, arg3, arg4, arg5, arg6, arg7), 180 B / 0xb4)
//
// Behaviour (recovered from asm @ 0x0003bfc0):
//
//   The function is a small logging / string-format helper. It allocates a
//   2048-byte stack buffer (char buf[0x800]), then selects one of two
//   _snprintf_s-like format strings based on whether the fifth argument
//   (arg5) is non-zero:
//
//     if (arg5 != 0)
//         _snprintf_s(buf, 0x800, 0x7ff, fmt_with_col,
//                     arg3, arg4, arg5, arg1, arg2);
//     else
//         _snprintf_s(buf, 0x800, 0x7ff, fmt_no_col,
//                     arg3, arg4, arg1, arg2);
//     output_fn(buf, 6);
//
//   The "output_fn" is called through an IAT slot at 0x012651b4 with
//   log-level 6. After the call the function validates the /GS security
//   cookie in the standard MSVC 2005 epilogue, preceded by an unusual
//   10-byte `MOV dword ptr [0x00000000], 0x0` instruction (emitted raw).
//
// Stack layout (after SUB ESP, 0x804):
//   [ESP+0x000 .. ESP+0x7FF]  char buf[0x800]   (2048-byte local buffer)
//   [ESP+0x800]               /GS cookie storage
//   [ESP+0x804]               return address
//   [ESP+0x808]               arg1
//   [ESP+0x80c]               arg2
//   [ESP+0x810]               arg3
//   [ESP+0x814]               arg4
//   [ESP+0x818]               arg5  (tested for zero to pick format branch)
//   [ESP+0x81c]               arg6
//   [ESP+0x820]               arg7 / extra variadic arg (non-zero branch only)
//
// Reloc-bearing sites (offsets within the 180-byte function body — these
// 4-byte windows are wildcarded by compare.py against orig):
//   +0x07   MOV EAX, [__security_cookie]        (.data  0x012ea8b0)
//   +0x42   PUSH offset fmt_with_col             (.rdata 0x00f6657c)
//   +0x56   CALL rel32 → fn_009d4f9f            (snprintf-like, 0x009d4f9f)
//   +0x69   PUSH offset fmt_no_col              (.rdata 0x00f664f4)
//   +0x7d   CALL rel32 → fn_009d4f9f            (second snprintf call)
//   +0x8c   CALL dword ptr [ext_012651b4]        (IAT  0x012651b4)
//   +0xa9   CALL rel32 → fn_009d20f4            (__security_check_cookie)

extern "C" {
    // .data — MSVC /GS security cookie
    extern unsigned __security_cookie;

    // .rdata — format strings for the two logging branches
    extern char fmt_with_col[];   // VA 0x00f6657c  (non-zero branch: includes col/line fields)
    extern char fmt_no_col[];     // VA 0x00f664f4  (zero branch: omits column field)

    // .idata — IAT slot for the output/logging function at index 0x012651b4
    extern int ext_012651b4;

    // .text — snprintf-like CRT function (internal or statically linked)
    int fn_009d4f9f();

    // .text — __security_check_cookie (CRT /GS check, statically linked)
    int fn_009d20f4();
}

extern "C" __declspec(naked) void FUN_0043bfc0()
{
    __asm {
        // --- Prologue: allocate frame + /GS cookie -------------------------
        sub     esp, 0x804                          // 81 ec 04 08 00 00
        mov     eax, __security_cookie              // a1 ?? ?? ?? ??  (reloc)
        xor     eax, esp                            // 33 c4
        mov     dword ptr [esp + 0x800], eax        // 89 84 24 00 08 00 00

        // --- Load arguments before the conditional branch ------------------
        mov     edx, dword ptr [esp + 0x80c]        // 8b 94 24 0c 08 00 00  (arg2)
        mov     eax, dword ptr [esp + 0x818]        // 8b 84 24 18 08 00 00  (arg5)
        test    eax, eax                            // 85 c0
        mov     ecx, dword ptr [esp + 0x808]        // 8b 8c 24 08 08 00 00  (arg1)
        push    esi                                 // 56               (callee-save)
        mov     esi, dword ptr [esp + 0x814]        // 8b b4 24 14 08 00 00  (arg3, ESP-adj'd)
        push    edx                                 // 52               (arg2 → stack for call)
        push    ecx                                 // 51               (arg1 → stack for call)
        jz      short else_branch                   // 74 28

        // --- Non-zero branch: include arg5 in format string ----------------
        push    eax                                 // 50               (arg5)
        mov     eax, dword ptr [esp + 0x824]        // 8b 84 24 24 08 00 00  (arg4)
        push    eax                                 // 50               (arg4)
        push    esi                                 // 56               (arg3)
        push    offset fmt_with_col                 // 68 ?? ?? ?? ??  (reloc)
        push    0x7ff                               // 68 ff 07 00 00
        lea     ecx, [esp + 0x20]                   // 8d 4c 24 20      (&buf[0])
        push    0x800                               // 68 00 08 00 00
        push    ecx                                 // 51
        call    fn_009d4f9f                         // e8 ?? ?? ?? ??  (reloc)
        add     esp, 0x24                           // 83 c4 24
        jmp     short end_if                        // eb 25

    else_branch:
        // --- Zero branch: omit arg5 from format string ---------------------
        mov     edx, dword ptr [esp + 0x820]        // 8b 94 24 20 08 00 00  (arg4)
        push    edx                                 // 52               (arg4)
        push    esi                                 // 56               (arg3)
        push    offset fmt_no_col                   // 68 ?? ?? ?? ??  (reloc)
        push    0x7ff                               // 68 ff 07 00 00
        lea     eax, [esp + 0x1c]                   // 8d 44 24 1c      (&buf[0])
        push    0x800                               // 68 00 08 00 00
        push    eax                                 // 50
        call    fn_009d4f9f                         // e8 ?? ?? ?? ??  (reloc)
        add     esp, 0x20                           // 83 c4 20

    end_if:
        // --- Call output function with buf and log-level 6 -----------------
        lea     ecx, [esp + 0x4]                    // 8d 4c 24 04      (&buf[0], ESP=ESP1)
        push    6                                   // 6a 06
        push    ecx                                 // 51
        call    dword ptr [ext_012651b4]            // ff 15 ?? ?? ?? ??  (IAT, reloc)

        // --- /GS epilogue --------------------------------------------------
        mov     ecx, dword ptr [esp + 0x80c]        // 8b 8c 24 0c 08 00 00  (cookie from stack)
        add     esp, 0x8                            // 83 c4 08
        pop     esi                                 // 5e
        xor     ecx, esp                            // 33 cc

        // MOV dword ptr [0x00000000], 0x0  (c7 05 00 00 00 00  00 00 00 00)
        _emit   0xc7
        _emit   0x05
        _emit   0x00
        _emit   0x00
        _emit   0x00
        _emit   0x00
        _emit   0x00
        _emit   0x00
        _emit   0x00
        _emit   0x00

        call    fn_009d20f4                         // e8 ?? ?? ?? ??  (__security_check_cookie)
        add     esp, 0x804                          // 81 c4 04 08 00 00
        ret                                         // c3
    }
}
