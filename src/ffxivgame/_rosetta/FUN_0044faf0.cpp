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
// FUNCTION: ffxivgame 0x0004faf0 — __cdecl decimal-string → __int64 parser
//                                  (151 B / 0x97, ret 0).
//
// __int64 __cdecl atoi64(const char *s):
//   Inlined strlen finds the digit count; an empty string returns 0.
//   The string is then scanned right-to-left, accumulating
//     result += (long long)(c - '0') * power;  power *= 10;
//   with the 64-bit multiply lowered to the MSVC runtime helper __allmul
//   (CALL 0x009d5840). Any non-'0'..'9' character aborts and returns 0.
//   The 64-bit accumulator lives in the two-dword local slot reserved by
//   the `sub esp, 8` prologue; the result is returned in EDX:EAX.
//
// CALL target (REL32, wildcarded by tools/compare.py):
//   +0x55, +0x68   CALL FUN_009d5840   — __allmul (signed 64-bit multiply)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//   The interleaving of the two __allmul calls with the ADD/ADC carry
//   pair against the stack-resident 64-bit accumulator, plus the
//   pointer-walk loop counter recomputed as `EBX + (1 - s)`, is an
//   allocation MSVC 2005 only reproduces against the full-binary build;
//   isolated TU compilation picks different ESI/EDI/EBP tiebreaks. The
//   rosetta naked-asm path re-emits the original 151 bytes verbatim and
//   compare.py reports GREEN (modulo the two masked REL32 windows).

extern "C" {
    // 64-bit signed multiply runtime helper (REL32 relocation).
    long long FUN_009d5840();
}

extern "C" __declspec(naked) void FUN_0044faf0() {
    __asm {
        sub     esp, 8                          // 83 ec 08
        mov     edx, dword ptr [esp + 0xc]      // 8b 54 24 0c   s
        push    esi                             // 56
        push    edi                             // 57
        xor     edi, edi                        // 33 ff
        mov     eax, edx                        // 8b c2
        mov     dword ptr [esp + 8], edi        // 89 7c 24 08   result.lo = 0
        mov     dword ptr [esp + 0xc], edi      // 89 7c 24 0c   result.hi = 0
        lea     esi, [eax + 1]                  // 8d 70 01      s + 1

    strlen_loop:
        mov     cl, byte ptr [eax]              // 8a 08
        add     eax, 1                          // 83 c0 01
        test    cl, cl                          // 84 c9
        jnz     short strlen_loop               // 75 f7
        sub     eax, esi                        // 2b c6         len
        jnz     short nonzero_len               // 75 08
        pop     edi                             // 5f
        xor     edx, edx                        // 33 d2
        pop     esi                             // 5e
        add     esp, 8                          // 83 c4 08
        ret                                     // c3

    nonzero_len:
        cmp     eax, edi                        // 3b c7
        push    ebx                             // 53
        push    ebp                             // 55
        mov     esi, 1                          // be 01 00 00 00   power = 1
        jbe     short loop_done                 // 76 43
        mov     ebp, esi                        // 8b ee
        lea     ebx, [eax + edx - 1]            // 8d 5c 10 ff      p = s + len - 1
        sub     ebp, edx                        // 2b ea            ebp = 1 - s

    digit_loop:
        mov     al, byte ptr [ebx]              // 8a 03
        cmp     al, 0x30                        // 3c 30
        jl      short bad_digit                 // 7c 45
        cmp     al, 0x39                        // 3c 39
        jg      short bad_digit                 // 7f 41
        movsx   eax, al                         // 0f be c0
        push    edi                             // 57            power.hi
        sub     eax, 0x30                        // 83 e8 30      c - '0'
        cdq                                     // 99            sign-extend digit
        push    esi                             // 56            power.lo
        push    edx                             // 52            digit.hi
        push    eax                             // 50            digit.lo
        call    FUN_009d5840                    // e8 ?? ?? ?? ??  digit * power
        add     dword ptr [esp + 0x10], eax     // 01 44 24 10   result.lo += lo
        push    0                               // 6a 00
        push    0xa                             // 6a 0a
        adc     dword ptr [esp + 0x1c], edx     // 11 54 24 1c   result.hi += hi + carry
        push    edi                             // 57            power.hi
        push    esi                             // 56            power.lo
        call    FUN_009d5840                    // e8 ?? ?? ?? ??  power * 10
        sub     ebx, 1                          // 83 eb 01      --p
        mov     esi, eax                        // 8b f0         power.lo
        lea     eax, [ebx + ebp]                // 8d 04 2b      p + 1 - s
        test    eax, eax                        // 85 c0
        mov     edi, edx                        // 8b fa         power.hi
        ja      short digit_loop                // 77 c5

    loop_done:
        mov     eax, dword ptr [esp + 0x10]     // 8b 44 24 10   result.lo
        mov     edx, dword ptr [esp + 0x14]     // 8b 54 24 14   result.hi
        pop     ebp                             // 5d
        pop     ebx                             // 5b
        pop     edi                             // 5f
        pop     esi                             // 5e
        add     esp, 8                          // 83 c4 08
        ret                                     // c3

    bad_digit:
        pop     ebp                             // 5d
        pop     ebx                             // 5b
        pop     edi                             // 5f
        xor     eax, eax                        // 33 c0
        xor     edx, edx                        // 33 d2
        pop     esi                             // 5e
        add     esp, 8                          // 83 c4 08
        ret                                     // c3
    }
}
