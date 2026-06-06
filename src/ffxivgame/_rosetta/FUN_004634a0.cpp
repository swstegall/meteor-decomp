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
// FUNCTION: ffxivgame 0x004634a0 — setup_crldp (OpenSSL X.509 CRL DP setup)
//                                  (72 B / 0x48)
//
// Calling convention: non-standard — EDI is used directly as the X509*
// parameter (static intra-module register-passing optimisation by MSVC).
// No stack argument is set up. EBX is callee-saved (loop counter i);
// ESI is saved/restored only inside the count > 0 arm. plain RET (no
// stack cleanup) confirms __cdecl-style (caller-cleans), but the
// argument is in EDI rather than on the stack.
//
// Body:
//
//   x->crldp = X509_get_ext_d2i(x, NID_crl_distribution_points, NULL, NULL);
//   // NID_crl_distribution_points = 0x67
//   count = sk_num(x->crldp);
//   if (count > 0) {
//       for (i = 0; i < count; i++) {
//           DIST_POINT *dp = sk_value(x->crldp, i);
//           // dp goes in ESI; setup_dp receives x on stack, dp via ESI
//           setup_dp(x, dp);
//           count = sk_num(x->crldp);   // re-check count each iteration
//       }
//   }
//
// Notes:
//   X509* field offset 0x44 = crldp (CRL distribution points extension)
//   The initial sk_num call reuses the EAX push that was pushed for the
//   X509_get_ext_d2i call — the ADD ESP,0x14 cleans all 5 args at once
//   (4 for X509_get_ext_d2i + 1 for sk_num).
//
// Reloc-bearing positions (masked by tools/compare.py):
//   off 0x09  CALL rel32 → _X509_get_ext_d2i
//   off 0x14  CALL rel32 → sk_num
//   off 0x26  CALL rel32 → sk_value
//   off 0x2e  CALL rel32 → setup_dp
//   off 0x3a  CALL rel32 → sk_num

extern "C" void _X509_get_ext_d2i();
extern "C" void sk_num();
extern "C" void sk_value();
extern "C" void setup_dp();

extern "C" __declspec(naked) void FUN_004634a0() {
    __asm {
        push    ebx
        push    0
        push    0
        push    0x67
        push    edi
        call    _X509_get_ext_d2i
        push    eax
        mov     dword ptr [edi + 0x44], eax
        xor     ebx, ebx
        call    sk_num
        add     esp, 0x14
        test    eax, eax
        jle     done
        push    esi
    loop_top:
        mov     eax, dword ptr [edi + 0x44]
        push    ebx
        push    eax
        call    sk_value
        push    edi
        mov     esi, eax
        call    setup_dp
        mov     ecx, dword ptr [edi + 0x44]
        push    ecx
        add     ebx, 1
        call    sk_num
        add     esp, 0x10
        cmp     ebx, eax
        jl      loop_top
        pop     esi
    done:
        pop     ebx
        ret
    }
}
