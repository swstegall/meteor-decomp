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
// FUNCTION: ffxivgame 0x9e6bae (RVA 0x5e6bae)
//
// CRT SSE2 trig kernel (sin/cos-style). Extracts the high word of the
// double argument, range-reduces against a 64-entry table at 0x10887c0,
// evaluates the polynomial, and returns the result in st(0) (__cdecl
// double return). The `ja` overflow path mirrors the special-case
// handling and tail-jumps to the slow reducer at 0x9d63ff for large
// magnitudes. Transcribed naked because this is vendor hand-tuned x87/SSE2
// assembly with no C-level source; the absolute .rdata constant addresses
// are baked literally (they match the orig image VAs byte-for-byte) and the
// tail jump relocates (masked by the byte-diff grader).

extern "C" void FUN_009d63ff();

// .rdata constant pool referenced by absolute VA. Declared as externs so the
// inline-asm memory operands emit DIR32 relocations (masked by the byte-diff
// grader) at the same offsets as the orig image's baked absolute addresses.
extern "C" double g_c1089000;
extern "C" double g_c1089008;
extern "C" double g_c1089010;
extern "C" double g_c1089020;
extern "C" double g_c1089028;
extern "C" double g_c1089030;
extern "C" double g_c1089038;
extern "C" double g_c1089040;
extern "C" double g_c1088fc0;
extern "C" double g_c1088fd0;
extern "C" double g_c1088fe0;
extern "C" double g_c1088ff0;
extern "C" double g_c10887c0;

extern "C" __declspec(naked) double FUN_009e6bae()
{
    __asm {
        pextrw  eax, xmm0, 3
        and     ax, 7fffh
        sub     ax, 3030h
        cmp     ax, 10c5h
        ja      L_d07
        movlpd  xmm1, qword ptr [g_c1089000]
        mulsd   xmm1, xmm0
        movlpd  xmm2, qword ptr [g_c1089008]
        cvtsd2si edx, xmm1
        addsd   xmm1, xmm2
        movlpd  xmm3, qword ptr [g_c1089020]
        subsd   xmm1, xmm2
        movapd  xmm2, xmmword ptr [g_c1089010]
        mulsd   xmm3, xmm1
        unpcklpd xmm1, xmm1
        add     edx, 1c7600h
        movsd   xmm4, xmm0
        and     edx, 3fh
        movapd  xmm5, xmmword ptr [g_c1088ff0]
        lea     eax, [g_c10887c0]
        shl     edx, 5
        add     eax, edx
        mulpd   xmm2, xmm1
        subsd   xmm0, xmm3
        mulsd   xmm1, qword ptr [g_c1089028]
        subsd   xmm4, xmm3
        movlpd  xmm7, qword ptr [eax+8]
        unpcklpd xmm0, xmm0
        movsd   xmm3, xmm4
        subsd   xmm4, xmm2
        mulpd   xmm5, xmm0
        subpd   xmm0, xmm2
        movapd  xmm6, xmmword ptr [g_c1088fd0]
        mulsd   xmm7, xmm4
        subsd   xmm3, xmm4
        mulpd   xmm5, xmm0
        mulpd   xmm0, xmm0
        subsd   xmm3, xmm2
        movapd  xmm2, xmmword ptr [eax]
        subsd   xmm1, xmm3
        movlpd  xmm3, qword ptr [eax+18h]
        addsd   xmm2, xmm3
        subsd   xmm7, xmm2
        mulsd   xmm2, xmm4
        mulpd   xmm6, xmm0
        mulsd   xmm3, xmm4
        mulpd   xmm2, xmm0
        mulpd   xmm0, xmm0
        addpd   xmm5, xmmword ptr [g_c1088fe0]
        mulsd   xmm4, qword ptr [eax]
        addpd   xmm6, xmmword ptr [g_c1088fc0]
        mulpd   xmm5, xmm0
        movsd   xmm0, xmm3
        addsd   xmm3, qword ptr [eax+8]
        mulpd   xmm1, xmm7
        movsd   xmm7, xmm4
        addsd   xmm4, xmm3
        addpd   xmm6, xmm5
        movlpd  xmm5, qword ptr [eax+8]
        subsd   xmm5, xmm3
        subsd   xmm3, xmm4
        addsd   xmm1, qword ptr [eax+10h]
        mulpd   xmm6, xmm2
        addsd   xmm5, xmm0
        addsd   xmm3, xmm7
        addsd   xmm1, xmm5
        addsd   xmm1, xmm3
        addsd   xmm1, xmm6
        unpckhpd xmm6, xmm6
        addsd   xmm1, xmm6
        sub     esp, 10h
        addsd   xmm4, xmm1
        movlpd  qword ptr [esp+4], xmm4
        fld     qword ptr [esp+4]
        add     esp, 10h
        ret
    L_d07:
        jg      L_d52
        sub     esp, 10h
        shr     ax, 4
        cmp     ax, 0cfdh
        jne     L_d2c
        mulsd   xmm0, qword ptr [g_c1089040]
        movlpd  qword ptr [esp+4], xmm0
        fld     qword ptr [esp+4]
        add     esp, 10h
        ret
    L_d2c:
        movlpd  xmm3, qword ptr [g_c1089030]
        mulsd   xmm3, xmm0
        subsd   xmm3, xmm0
        mulsd   xmm3, qword ptr [g_c1089038]
        movlpd  qword ptr [esp+4], xmm0
        fld     qword ptr [esp+4]
        add     esp, 10h
        ret
    L_d52:
        jmp     FUN_009d63ff
    }
}
