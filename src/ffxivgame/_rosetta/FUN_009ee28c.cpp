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
// FUNCTION: ffxivgame 0x009ee28c — x87 extended-precision fmod dispatcher
//                                  (non-standard: args via x87 FP stack, 178 B / 0xb2)
//
// Wraps the real fmod implementation at FUN_009ee086, handling zero and
// denormal cases for 80-bit extended-precision arguments passed on the x87
// FPU stack.  EDX is used as a flag register to signal to the callee which
// normalisation was applied (bit 0 = small-exponent scale, bit 1 = denormal
// path); the caller preserves EDX across the call.
//
// Entry state: ST(0) = X (dividend), ST(1) = Y (divisor), both 80-bit
// extended.  The function stores them to [ESP] (Y) and [ESP+0x18] (X) and
// then branches on Y's exponent:
//
//   Case 1 — Y is normal / infinity / NaN (exponent 0x0001..0x7fff):
//     Call FUN_009ee086 directly with Y at [ESP] and X at [ESP+0x18].
//
//   Case 2 — Y is exactly zero (exponent == 0 AND significand == 0):
//     Execute FPREM (ST(0)=X mod ST(1)=Y) and return the raw x87 result.
//
//   Case 3 — Y is denormal (exponent == 0, significand != 0):
//     Set EDX |= 0x2 (denormal flag).
//     Modify FPU CW to mask all exceptions with extended precision.
//     Inspect X's 15-bit biased exponent (from [ESP+0x20], the exponent
//     word of the 80-bit X stored at [ESP+0x18]):
//
//     Case 3a — X exponent <= 0x7fbe (small):
//       OR EDX, 0x1 (needs post-scale correction).
//       Scale both X and Y by the constant at [0x012ebd34] (2^64),
//       writing scaled X to [ESP+0x18] and scaled Y to [ESP+0x00].
//       Restore FPU CW and call FUN_009ee086.
//
//     Case 3b — X exponent > 0x7fbe (large):
//       Save/restore FPU CW around a fresh FLDCW with extended-precision
//       mask (0x300).  Discard ST(0) (X), scale Y by [0x012ebd34], store
//       to [ESP+0x00], restore original FPU CW, call FUN_009ee086.
//
// Relocatable sites (4-byte windows wildcarded by compare.py):
//   +0x18   CALL rel32 → FUN_009ee086
//   +0x68   FMUL mem32 → [0x012ebd34]   (first multiply, case 3a, X)
//   +0x72   FMUL mem32 → [0x012ebd34]   (second multiply, case 3a, Y)
//   +0x94   FMUL mem32 → [0x012ebd34]   (multiply, case 3b, Y)
//   +0xa1   CALL rel32 → FUN_009ee086

extern "C" __declspec(naked) void FUN_009ee28c() {
    __asm {
        _emit 0x52              // PUSH EDX
        _emit 0x83              // SUB ESP, 0x30
        _emit 0xec
        _emit 0x30
        _emit 0xdb              // FSTP TBYTE PTR [ESP+0x18]
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0xdb              // FSTP TBYTE PTR [ESP]
        _emit 0x3c
        _emit 0x24
        _emit 0x33              // XOR EDX, EDX
        _emit 0xd2
        _emit 0x8b              // MOV EAX, DWORD PTR [ESP+0x6]
        _emit 0x44
        _emit 0x24
        _emit 0x06
        _emit 0xa9              // TEST EAX, 0x7fff0000
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0x7f
        _emit 0x74              // JZ +0x0a  (to case 2/3 handler)
        _emit 0x0a
        _emit 0xe8              // CALL rel32 → FUN_009ee086  (+0x18 reloc)
        _emit 0xdd
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x30
        _emit 0xc4
        _emit 0x30
        _emit 0x5a              // POP EDX
        _emit 0xc3              // RET
        _emit 0xdb              // FLD TBYTE PTR [ESP]  (case 2/3: reload Y)
        _emit 0x2c
        _emit 0x24
        _emit 0xdb              // FLD TBYTE PTR [ESP+0x18]  (reload X)
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV EAX, DWORD PTR [ESP]
        _emit 0x04
        _emit 0x24
        _emit 0x0b              // OR EAX, DWORD PTR [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x74              // JZ +0x79  (to FPREM path — case 2)
        _emit 0x79
        _emit 0xd9              // FXCH  (case 3 denormal: Y↔X on FP stack)
        _emit 0xc9
        _emit 0xdb              // FSTP TBYTE PTR [ESP+0xc]  (save ST(0)=Y)
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0xdb              // FLD TBYTE PTR [ESP]  (reload Y from [ESP])
        _emit 0x2c
        _emit 0x24
        _emit 0xd9              // FXCH  (restore order: ST(0)=X, ST(1)=Y)
        _emit 0xc9
        _emit 0x83              // OR EDX, 0x2  (set denormal flag)
        _emit 0xca
        _emit 0x02
        _emit 0xd9              // FNSTCW WORD PTR [ESP+0x24]
        _emit 0x7c
        _emit 0x24
        _emit 0x24
        _emit 0x8b              // MOV EAX, DWORD PTR [ESP+0x24]
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x0d              // OR EAX, 0x33f  (mask all exceptions, extended precision)
        _emit 0x3f
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV DWORD PTR [ESP+0x28], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0xd9              // FLDCW WORD PTR [ESP+0x28]
        _emit 0x6c
        _emit 0x24
        _emit 0x28
        _emit 0x8b              // MOV EAX, DWORD PTR [ESP+0x20]  (X exponent word)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x25              // AND EAX, 0x7fff  (strip sign, keep 15-bit exp)
        _emit 0xff
        _emit 0x7f
        _emit 0x00
        _emit 0x00
        _emit 0x3d              // CMP EAX, 0x7fbe
        _emit 0xbe
        _emit 0x7f
        _emit 0x00
        _emit 0x00
        _emit 0x77              // JA +0x18  (large exponent → case 3b)
        _emit 0x18
        _emit 0x83              // OR EDX, 0x1  (case 3a: small-exp, needs post-scale)
        _emit 0xca
        _emit 0x01
        _emit 0xdc              // FMUL QWORD PTR [0x012ebd34]  (+0x68 reloc, scale X)
        _emit 0x0d
        _emit 0x34
        _emit 0xbd
        _emit 0x2e
        _emit 0x01
        _emit 0xdb              // FSTP TBYTE PTR [ESP+0x18]  (store scaled X)
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0xdc              // FMUL QWORD PTR [0x012ebd34]  (+0x72 reloc, scale Y)
        _emit 0x0d
        _emit 0x34
        _emit 0xbd
        _emit 0x2e
        _emit 0x01
        _emit 0xdb              // FSTP TBYTE PTR [ESP]  (store scaled Y)
        _emit 0x3c
        _emit 0x24
        _emit 0xeb              // JMP +0x20  (to FLDCW restore + call)
        _emit 0x20
        _emit 0xd9              // FNSTCW WORD PTR [ESP+0x24]  (case 3b: save CW again)
        _emit 0x7c
        _emit 0x24
        _emit 0x24
        _emit 0x8b              // MOV EAX, DWORD PTR [ESP+0x24]
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x0d              // OR EAX, 0x300  (extended precision only)
        _emit 0x00
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV DWORD PTR [ESP+0x28], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0xd9              // FLDCW WORD PTR [ESP+0x28]
        _emit 0x6c
        _emit 0x24
        _emit 0x28
        _emit 0xdd              // FSTP ST(0)  (discard X — too large)
        _emit 0xd8
        _emit 0xdc              // FMUL QWORD PTR [0x012ebd34]  (+0x94 reloc, scale Y)
        _emit 0x0d
        _emit 0x34
        _emit 0xbd
        _emit 0x2e
        _emit 0x01
        _emit 0xdb              // FSTP TBYTE PTR [ESP]  (store scaled Y)
        _emit 0x3c
        _emit 0x24
        _emit 0xd9              // FLDCW WORD PTR [ESP+0x24]  (restore original CW)
        _emit 0x6c
        _emit 0x24
        _emit 0x24
        _emit 0xe8              // CALL rel32 → FUN_009ee086  (+0xa1 reloc)
        _emit 0x54
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x30
        _emit 0xc4
        _emit 0x30
        _emit 0x5a              // POP EDX
        _emit 0xc3              // RET
        _emit 0xd9              // FPREM  (case 2: Y is exactly zero)
        _emit 0xf8
        _emit 0x83              // ADD ESP, 0x30
        _emit 0xc4
        _emit 0x30
        _emit 0x5a              // POP EDX
        _emit 0xc3              // RET
    }
}
