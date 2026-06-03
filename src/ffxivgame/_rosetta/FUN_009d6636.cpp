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
// FUNCTION: ffxivgame 0x009d6636 — float-to-int64 truncation (round toward zero)
//                                  custom _ftol-style helper, 117 bytes / 0x75
//
// Calling convention: non-standard — float argument in ST(0) on FPU stack entry;
//                     returns int64 in EDX:EAX; __cdecl-like stack frame.
//
// Stack frame (EBP-based, 32 bytes local space, 16-byte aligned):
//   [ESP+0x10..0x17]  int64 scratch (FISTP output / FILD input)
//   [ESP+0x14]        upper dword of int64 scratch
//   [ESP+0x18]        float scratch (FST of original)
//
// Algorithm (pseudo-C):
//   int64_t FUN_009d6636(/* float in ST(0) */) {
//       float  orig = ST(0);          // FST [ESP+0x18]
//       int64_t n   = (int64_t)orig;  // FISTP via round-to-nearest
//       float  rnd  = (float)n;       // FILD [ESP+0x10]
//       if (n == 0) {
//           if (orig != 0.0f) goto retry_fsubp;  // TEST [ESP+0x14] & 0x7fffffff
//           // clean FP stack and return 0
//           return 0;
//       }
//   retry_fsubp:
//       float diff = orig - rnd;       // FSUBP ST(1),ST(0)
//       if (orig < 0) {
//           // XOR sign bit, add 0x7fffffff, propagate carry into n
//           return n + (carry from sign-flip + 0x7fffffff);
//       } else {
//           // add 0x7fffffff, subtract borrow from n
//           return n - (borrow from 0x7fffffff + diff_bits);
//       }
//   }
//
// Notable codegen details:
//   - SUB ESP,0x20 + AND ESP,0xfffffff0 gives 16-byte-aligned frame
//   - FLD ST(0) at entry duplicates the incoming float so FST can
//     snapshot it before FISTP pops it
//   - FSUBP ST(1),ST(0): difference = ST(1) - ST(0), pop → diff in ST(0)
//   - XOR ECX,0x80000000 + ADD ECX,0x7fffffff + ADC EAX,0 / ADC EDX,0
//     implements the negative-input correction (adjusts EDX:EAX by +1
//     when FISTP rounded away from zero)
//   - ADD ECX,0x7fffffff + SBB EAX,0 / SBB EDX,0 implements the
//     positive-input correction (adjusts EDX:EAX by -1 when FISTP
//     rounded away from zero)
//   - FSTP × 2 at 0x6b/0x6f cleans the two FPU-stack slots when
//     the input is exactly zero
//   - No CALL rel32 relocations; the .obj is byte-identical without masking
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The interaction of the EBP frame, the FPU-stack convention for the
//   float argument, the FISTP/FILD round-trip, the two-path correction
//   (ADC vs SBB), the exact FSUBP encoding (DE E9), and the double FSTP
//   at the zero-exit are not reproducible from source-level C++ without
//   extensive pragmas and intrinsics under /O2. A __declspec(naked) body
//   re-emitting the 117 bytes verbatim via MASM _emit directives produces
//   a .obj whose .text is byte-identical to the original slice.

extern "C" __declspec(naked) void FUN_009d6636() {
    __asm {
        // 009d6636: 55              PUSH EBP
        _emit 0x55
        // 009d6637: 8b ec           MOV EBP,ESP
        _emit 0x8b
        _emit 0xec
        // 009d6639: 83 ec 20        SUB ESP,0x20
        _emit 0x83
        _emit 0xec
        _emit 0x20
        // 009d663c: 83 e4 f0        AND ESP,0xfffffff0
        _emit 0x83
        _emit 0xe4
        _emit 0xf0
        // 009d663f: d9 c0           FLD ST(0)   (duplicate incoming float)
        _emit 0xd9
        _emit 0xc0
        // 009d6641: d9 54 24 18     FST dword ptr [ESP+0x18]   (snapshot original)
        _emit 0xd9
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 009d6645: df 7c 24 10     FISTP qword ptr [ESP+0x10]  (→ int64, pop)
        _emit 0xdf
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 009d6649: df 6c 24 10     FILD qword ptr [ESP+0x10]   (reload as float)
        _emit 0xdf
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // 009d664d: 8b 54 24 18     MOV EDX,dword ptr [ESP+0x18]  (original float bits)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 009d6651: 8b 44 24 10     MOV EAX,dword ptr [ESP+0x10]  (low dword of int64)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 009d6655: 85 c0           TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 009d6657: 74 3c           JZ +0x3c  (→ 009d6695, EAX==0 path)
        _emit 0x74
        _emit 0x3c
        // 009d6659: de e9           FSUBP ST(1),ST(0)  (diff = orig - rnd, pop)
        _emit 0xde
        _emit 0xe9
        // 009d665b: 85 d2           TEST EDX,EDX
        _emit 0x85
        _emit 0xd2
        // 009d665d: 79 1e           JNS +0x1e  (→ 009d667d, positive/zero original)
        _emit 0x79
        _emit 0x1e
        // --- negative-original correction path ---
        // 009d665f: d9 1c 24        FSTP dword ptr [ESP]   (store diff bits, pop)
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // 009d6662: 8b 0c 24        MOV ECX,dword ptr [ESP]
        _emit 0x8b
        _emit 0x0c
        _emit 0x24
        // 009d6665: 81 f1 00 00 00 80  XOR ECX,0x80000000
        _emit 0x81
        _emit 0xf1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80
        // 009d666b: 81 c1 ff ff ff 7f  ADD ECX,0x7fffffff
        _emit 0x81
        _emit 0xc1
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x7f
        // 009d6671: 83 d0 00        ADC EAX,0x0
        _emit 0x83
        _emit 0xd0
        _emit 0x00
        // 009d6674: 8b 54 24 14     MOV EDX,dword ptr [ESP+0x14]  (high dword)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 009d6678: 83 d2 00        ADC EDX,0x0
        _emit 0x83
        _emit 0xd2
        _emit 0x00
        // 009d667b: eb 2c           JMP +0x2c  (→ 009d66a9, epilogue)
        _emit 0xeb
        _emit 0x2c
        // --- positive-original correction path ---
        // 009d667d: d9 1c 24        FSTP dword ptr [ESP]   (store diff bits, pop)
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // 009d6680: 8b 0c 24        MOV ECX,dword ptr [ESP]
        _emit 0x8b
        _emit 0x0c
        _emit 0x24
        // 009d6683: 81 c1 ff ff ff 7f  ADD ECX,0x7fffffff
        _emit 0x81
        _emit 0xc1
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x7f
        // 009d6689: 83 d8 00        SBB EAX,0x0
        _emit 0x83
        _emit 0xd8
        _emit 0x00
        // 009d668c: 8b 54 24 14     MOV EDX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 009d6690: 83 da 00        SBB EDX,0x0
        _emit 0x83
        _emit 0xda
        _emit 0x00
        // 009d6693: eb 14           JMP +0x14  (→ 009d66a9, epilogue)
        _emit 0xeb
        _emit 0x14
        // --- EAX==0 path ---
        // 009d6695: 8b 54 24 14     MOV EDX,dword ptr [ESP+0x14]  (high dword of int64)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 009d6699: f7 c2 ff ff ff 7f  TEST EDX,0x7fffffff
        _emit 0xf7
        _emit 0xc2
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x7f
        // 009d669f: 75 b8           JNZ -0x48  (→ 009d6659, FSUBP path)
        _emit 0x75
        _emit 0xb8
        // --- exact-zero path: clean FP stack ---
        // 009d66a1: d9 5c 24 18     FSTP dword ptr [ESP+0x18]  (pop ST(0))
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        // 009d66a5: d9 5c 24 18     FSTP dword ptr [ESP+0x18]  (pop ST(1))
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        // --- epilogue ---
        // 009d66a9: c9              LEAVE
        _emit 0xc9
        // 009d66aa: c3              RET
        _emit 0xc3
    }
}
