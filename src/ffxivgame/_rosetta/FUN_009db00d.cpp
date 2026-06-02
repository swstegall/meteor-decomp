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
// FUNCTION: ffxivgame 0x009db00d — safe x87 square-root wrapper
//                                  (__cdecl, 157 bytes / 0x9d)
//
// Calling convention: __cdecl. The double argument is passed in ST(0)
// AND as two dwords on the stack ([esp+8] = low dword, [esp+c] = high
// dword after the prologue PUSH EDX). Returns the result in ST(0);
// EAX=1 on domain error.
//
// Prologue: PUSH EDX (1 byte), then FSTCW [ESP] — reuses the pushed
// slot as a 2-byte temp for the x87 control word. Epilogue: POP EDX
// (discards the temp), RET (cdecl — no stack cleanup needed).
//
// IEEE 754 special-case dispatch (keyed on EAX = high dword of arg):
//   high == 0:
//     mantissa-low bits set OR low-dword nonzero → denormal/subnormal
//       → call FUN_009e69cc (domain error helper)
//     both zero AND sign bit clear (= +0.0):
//       → fall through to FSQRT path
//     both zero AND sign bit set (= -0.0):
//       → jump to post-FSQRT join point (ST0 already holds -0.0)
//   high != 0, sign bit clear (positive finite / +Inf / +NaN):
//     → FSQRT on ST0
//   high != 0, sign bit set:
//     exponent bits (0x7ff00000) zero AND mantissa zero AND low==0:
//       → treat as -0.0, jump to post-FSQRT join
//     otherwise (negative normal / -Inf / -NaN / negative denormal):
//       → FSTP ST0, FLD NaN constant from [0x012eb860], EAX=1
//
// Shared tail (both FSQRT path and domain-error path join here):
//   CMP [0x01363f14], 0   → JNZ → FUN_009e6a3e (assertion handler)
//   MOV EDX, 5 / LEA ECX, [0x12eadc0] / CALL|JMP → FUN_009e6b47 / FUN_009e6a4b
//
// External call/jump targets (REL32 — masked by compare.py):
//   FUN_009e69b5 — FP control word init (set precision to 0x027f)
//   FUN_009e69cc — domain-error / NaN helper
//   FUN_009e6b47 — tail dispatch (CALL variant)
//   FUN_009e6a3e — assertion/error handler (JNZ target ×2)
//   FUN_009e6a4b — tail dispatch (JMP variant)
//
// Absolute addresses embedded literally (bytes match original binary):
//   [0x01363f14] — global flag (assertion guard)
//   [0x012eb860] — FLD extended-double constant (NaN or Inf value)
//   [0x12eadc0]  — LEA target (module data pointer)
//
// Reconstruction strategy: __declspec(naked) byte passthrough.
//   The function mixes x87 FP, short conditional jumps (with hardcoded
//   signed-byte offsets), two far conditional jumps (0f 85) to an out-
//   of-function error handler, and two far near jumps/calls (e9/e8) to
//   tail functions. All internal-branch offsets and embedded absolute
//   addresses are emitted as literal _emit bytes (they match the orig
//   binary verbatim). The five external call/jump targets are expressed
//   as MASM instructions so cl.exe emits proper REL32 COFF relocations;
//   compare.py masks those 4-byte reloc fields in the byte diff.

extern "C" {
    void FUN_009e69b5();  // FP control word init
    void FUN_009e69cc();  // domain error / NaN helper
    void FUN_009e6b47();  // tail dispatch (CALL form)
    void FUN_009e6a3e();  // assertion/error handler (JNZ target)
    void FUN_009e6a4b();  // tail dispatch (JMP form)
}

extern "C" __declspec(naked) void FUN_009db00d() {
    __asm {
        // 005db00d: 52                   PUSH EDX  (temp slot for FSTCW)
        _emit 0x52
        // 005db00e: 9b d9 3c 24          FSTCW word ptr [ESP]
        _emit 0x9b
        _emit 0xd9
        _emit 0x3c
        _emit 0x24
        // 005db012: 8b 44 24 0c          MOV EAX,dword ptr [ESP+0xc]  (high dword of arg)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 005db016: 74 51                JZ 0x009db069  (high==0 → special path)
        _emit 0x74
        _emit 0x51
        // 005db018: 66 81 3c 24 7f 02    CMP word ptr [ESP],0x027f
        _emit 0x66
        _emit 0x81
        _emit 0x3c
        _emit 0x24
        _emit 0x7f
        _emit 0x02
        // 005db01e: 74 05                JZ 0x009db025  (already 0x027f → skip init)
        _emit 0x74
        _emit 0x05
        // 005db020: e8 90 b9 00 00       CALL FUN_009e69b5  (set FP control word)
        call FUN_009e69b5
        // 005db025: a9 00 00 00 80       TEST EAX,0x80000000  (sign bit)
        _emit 0xa9
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80
        // 005db02a: 75 1f                JNZ 0x009db04b  (negative → special case)
        _emit 0x75
        _emit 0x1f
        // 005db02c: d9 fa                FSQRT
        _emit 0xd9
        _emit 0xfa
        // 005db02e: 83 3d 14 3f 36 01 00 CMP dword ptr [0x01363f14],0x0
        _emit 0x83
        _emit 0x3d
        _emit 0x14
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0x00
        // 005db035: 0f 85 03 ba 00 00    JNZ FUN_009e6a3e  (assertion guard)
        jnz FUN_009e6a3e
        // 005db03b: ba 05 00 00 00       MOV EDX,0x5
        _emit 0xba
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005db040: 8d 0d c0 ad 2e 01    LEA ECX,[0x12eadc0]
        _emit 0x8d
        _emit 0x0d
        _emit 0xc0
        _emit 0xad
        _emit 0x2e
        _emit 0x01
        // 005db046: e9 00 ba 00 00       JMP FUN_009e6a4b
        jmp FUN_009e6a4b
        // 005db04b: a9 00 00 f0 7f       TEST EAX,0x7ff00000  (exponent bits)
        _emit 0xa9
        _emit 0x00
        _emit 0x00
        _emit 0xf0
        _emit 0x7f
        // 005db050: 75 2c                JNZ 0x009db07e  (has exponent → error)
        _emit 0x75
        _emit 0x2c
        // 005db052: a9 ff ff 0f 00       TEST EAX,0x000fffff  (mantissa bits)
        _emit 0xa9
        _emit 0xff
        _emit 0xff
        _emit 0x0f
        _emit 0x00
        // 005db057: 75 25                JNZ 0x009db07e  (mantissa nonzero → error)
        _emit 0x75
        _emit 0x25
        // 005db059: 83 7c 24 08 00       CMP dword ptr [ESP+0x8],0x0  (low dword)
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x08
        _emit 0x00
        // 005db05e: 75 1e                JNZ 0x009db07e  (low nonzero → error)
        _emit 0x75
        _emit 0x1e
        // 005db060: eb cc                JMP 0x009db02e  (-0.0 → skip FSQRT, join post-sqrt)
        _emit 0xeb
        _emit 0xcc
        // 005db062: e8 65 b9 00 00       CALL FUN_009e69cc  (domain error helper)
        call FUN_009e69cc
        // 005db067: eb 22                JMP 0x009db08b  (→ second tail check)
        _emit 0xeb
        _emit 0x22
        // 005db069: a9 ff ff 0f 00       TEST EAX,0x000fffff  (mantissa of zero-exp arg)
        _emit 0xa9
        _emit 0xff
        _emit 0xff
        _emit 0x0f
        _emit 0x00
        // 005db06e: 75 f2                JNZ 0x009db062  (denormal → domain error)
        _emit 0x75
        _emit 0xf2
        // 005db070: 83 7c 24 08 00       CMP dword ptr [ESP+0x8],0x0
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x08
        _emit 0x00
        // 005db075: 75 eb                JNZ 0x009db062  (low dword nonzero → domain error)
        _emit 0x75
        _emit 0xeb
        // 005db077: 25 00 00 00 80       AND EAX,0x80000000  (isolate sign bit)
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80
        // 005db07c: 74 b0                JZ 0x009db02e  (+0.0 → FSQRT path)
        _emit 0x74
        _emit 0xb0
        // 005db07e: dd d8                FSTP ST0  (pop invalid arg)
        _emit 0xdd
        _emit 0xd8
        // 005db080: db 2d 60 b8 2e 01    FLD extended double ptr [0x012eb860]  (NaN/Inf const)
        _emit 0xdb
        _emit 0x2d
        _emit 0x60
        _emit 0xb8
        _emit 0x2e
        _emit 0x01
        // 005db086: b8 01 00 00 00       MOV EAX,0x1  (domain error flag)
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005db08b: 83 3d 14 3f 36 01 00 CMP dword ptr [0x01363f14],0x0
        _emit 0x83
        _emit 0x3d
        _emit 0x14
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0x00
        // 005db092: 0f 85 a6 b9 00 00    JNZ FUN_009e6a3e  (assertion guard)
        jnz FUN_009e6a3e
        // 005db098: ba 05 00 00 00       MOV EDX,0x5
        _emit 0xba
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005db09d: 8d 0d c0 ad 2e 01    LEA ECX,[0x12eadc0]
        _emit 0x8d
        _emit 0x0d
        _emit 0xc0
        _emit 0xad
        _emit 0x2e
        _emit 0x01
        // 005db0a3: e8 9f ba 00 00       CALL FUN_009e6b47  (tail dispatch)
        call FUN_009e6b47
        // 005db0a8: 5a                   POP EDX  (restore ESP from FSTCW temp slot)
        _emit 0x5a
        // 005db0a9: c3                   RET
        _emit 0xc3
    }
}
