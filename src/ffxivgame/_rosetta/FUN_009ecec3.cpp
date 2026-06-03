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
// FUNCTION: ffxivgame 0x005ecec3 — frexp: decompose double into mantissa and exponent
// (__cdecl double FUN_009ecec3(double x, int *pexp), 175 B = 0xaf).
//
// Decomposes x into a normalized fraction and an integer exponent such that
// x = fraction * 2^exponent, where 0.5 <= |fraction| < 1.0.
//
// Stores the exponent in *pexp. Returns the mantissa fraction (via ST0).
//
// Special cases:
//   x == 0.0  : *pexp = 0, returns 0.0
//   normal    : *pexp = biased_exponent - 1022 (= true_exponent + 1)
//   denormal  : left-shifts mantissa until bit 52 (implicit 1) is set,
//               counting the shifts into EDX; clears bit 52 and sign-extends
//               before calling the mantissa helper.
//
// FUN_009ece3e is the mantissa helper that adjusts the biased exponent field
// to 0x3fe so the result falls in [0.5, 1.0).  Its return value (ST0) is
// passed directly back to our caller.
//
// Why naked asm: the body intermixes x87 FPU comparisons (FCOM/FCOMP/
// FNSTSW/TEST AH) with integer bit manipulation of the argument's IEEE 754
// representation through [EBP+8]/[EBP+0xc]/[EBP+0xe].  Reproducing the
// exact encoding choices (near JMP at +0x11, 66 81 for AND word with
// 0xffef rather than the shorter 66 83 form, specific TEST AH forms) is
// impractical from plain C++ under /O2.  Naked asm ensures byte-identical
// output modulo the two rel32 CALL relocations to FUN_009ece3e.
//
// Reloc-bearing sites (4-byte windows wildcarded by compare.py):
//   +0x7a  CALL rel32 → FUN_009ece3e  (denorm path)
//   +0x8f  CALL rel32 → FUN_009ece3e  (normal path)

extern "C" {
    int FUN_009ece3e();  // mantissa-extraction helper (adjusts exp → 0x3fe)
}

extern "C" __declspec(naked) double FUN_009ecec3() {
    __asm {
        // --- prologue -------------------------------------------------------
        push    ebp                                    // 55
        mov     ebp, esp                               // 8b ec

        // --- zero test via FPU compare ------------------------------------
        fldz                                           // d9 ee
        fcom    qword ptr [ebp + 0x8]                  // dc 55 08
        fnstsw  ax                                     // df e0
        test    ah, 0x44                               // f6 c4 44
        jp      non_zero                               // 7a 07  (short, +7)
        xor     edx, edx                               // 33 d2
        jmp     epilogue                               // e9 92 00 00 00  (near, +0x92)

    non_zero:
        // --- check exponent field for zero (denormal or exact zero) --------
        xor     ecx, ecx                               // 33 c9
        test    word ptr [ebp + 0xe], 0x7ff0           // 66 f7 45 0e f0 7f
        jnz     normal_path                            // 75 63
        test    dword ptr [ebp + 0xc], 0x000fffff      // f7 45 0c ff ff 0f 00
        jnz     denorm_path                            // 75 05
        cmp     dword ptr [ebp + 0x8], ecx             // 39 4d 08
        jz      normal_path                            // 74 55

    denorm_path:
        // --- determine sign of denormalized number -------------------------
        fcomp   qword ptr [ebp + 0x8]                  // dc 5d 08
        mov     edx, 0xfffffc03                        // ba 03 fc ff ff
        fnstsw  ax                                     // df e0
        test    ah, 0x41                               // f6 c4 41
        jnz     pos_denorm                             // 75 05
        xor     eax, eax                               // 33 c0
        inc     eax                                    // 40
        jmp     shift_test                             // eb 18

    pos_denorm:
        xor     eax, eax                               // 33 c0
        jmp     shift_test                             // eb 14

    shift_loop:
        // --- left-shift mantissa bits (64-bit double through stack) --------
        shl     dword ptr [ebp + 0xc], 1               // d1 65 0c
        test    dword ptr [ebp + 0x8], 0x80000000      // f7 45 08 00 00 00 80
        jz      no_carry                               // 74 04
        or      dword ptr [ebp + 0xc], 1               // 83 4d 0c 01

    no_carry:
        shl     dword ptr [ebp + 0x8], 1               // d1 65 08
        dec     edx                                    // 4a

    shift_test:
        // --- loop until bit 52 (implicit 1) is set -------------------------
        test    byte ptr [ebp + 0xe], 0x10             // f6 45 0e 10
        jz      shift_loop                             // 74 e6

        // --- clear the implicit-1 bit and restore sign --------------------
        // AND word ptr [ebp+0xe], 0xffef  →  66 81 65 0e ef ff
        // Must use imm16 form (81), not sign-extended imm8 (83).
        _emit 0x66                                     // operand-size prefix
        _emit 0x81                                     // AND r/m16, imm16
        _emit 0x65                                     // ModRM: [EBP+disp8], /4
        _emit 0x0e                                     // disp8 = 14
        _emit 0xef                                     // imm16 lo = 0xef
        _emit 0xff                                     // imm16 hi = 0xff  → 0xffef
        cmp     eax, ecx                               // 3b c1
        jz      call_denorm                            // 74 06
        or      word ptr [ebp + 0xe], 0x8000           // 66 81 4d 0e 00 80

    call_denorm:
        // --- call mantissa helper with modified double --------------------
        fld     qword ptr [ebp + 0x8]                  // dd 45 08
        push    ecx                                    // 51
        push    ecx                                    // 51
        push    ecx                                    // 51
        fstp    qword ptr [esp]                        // dd 1c 24
        call    FUN_009ece3e                           // e8 fd fe ff ff
        add     esp, 0xc                               // 83 c4 0c
        jmp     epilogue                               // eb 25  (short, +0x25)

    normal_path:
        // --- normal/inf/NaN: discard FPU zero, call mantissa helper --------
        push    ecx                                    // 51
        fstp    st(0)                                  // dd d8
        fld     qword ptr [ebp + 0x8]                  // dd 45 08
        push    ecx                                    // 51
        push    ecx                                    // 51
        fstp    qword ptr [esp]                        // dd 1c 24
        call    FUN_009ece3e                           // e8 e8 fe ff ff
        mov     edx, dword ptr [ebp + 0xe]             // 8b 55 0e
        shr     edx, 0x4                               // c1 ea 04
        and     edx, 0x7ff                             // 81 e2 ff 07 00 00
        add     esp, 0xc                               // 83 c4 0c
        sub     edx, 0x3fe                             // 81 ea fe 03 00 00

    epilogue:
        // --- store exponent, return (mantissa already in ST0) --------------
        mov     eax, dword ptr [ebp + 0x10]            // 8b 45 10
        mov     dword ptr [eax], edx                   // 89 10
        pop     ebp                                    // 5d
        ret                                            // c3
    }
}
