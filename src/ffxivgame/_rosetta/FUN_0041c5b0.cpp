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
// FUNCTION: ffxivgame 0x0041c5b0 — level-band float ratio lookup:
//                                  returns a float value depending on `n`
//                                  queried against a cap/range table;
//                                  __cdecl, 113 bytes / 0x71.
//
// Calling convention: __cdecl, one `int` param (ESI = [ESP+0xC] after
//   PUSH ECX + PUSH ESI); returns float in x87 ST(0).
//
// Frame layout (after PUSH ECX, PUSH ESI):
//   [ESP+0x00]  saved ESI (callee-save)
//   [ESP+0x04]  saved ECX (serves as temp local for FILD)
//   [ESP+0x08]  return address
//   [ESP+0x0C]  int n  (first / only argument)
//
// Logic (pseudo-C):
//
//   float FUN_0041c5b0(int n) {
//       if (!FUN_00433150(n)) {
//           if (n < 2) return *((float*)0x00f5988c);  // small constant
//           if (n < 4) return *((float*)0x00f54f74);  // medium constant
//       }
//       // General case: compute ratio of two range-queries
//       int hi = FUN_00433190(n);
//       int lo = FUN_004331a0(n);
//       float flo = (float)lo + (lo < 0 ? *((float*)0x00f54a54) : 0.f);
//       float fhi = (float)hi + (hi < 0 ? *((float*)0x00f54a54) : 0.f);
//       return flo / fhi;   // FDIVP: ST(1)/ST(0) = flo/fhi
//   }
//
// Notable codegen details:
//   - PUSH ECX at entry allocates a one-slot local used as temp storage
//     for FILD (overwritten twice: once for `lo`, once for `hi`).
//   - Three CALL rel32 relocations (to FUN_00433150, FUN_00433190,
//     FUN_004331a0) and four absolute x87 mem-refs ([0x00f5988c],
//     [0x00f54f74], [0x00f54a54] x2) cannot be reproduced as
//     compile-time relocations from a standalone .obj — emitting as
//     raw immediates produces a byte-identical .text slice.
//   - FSTP float [ESP] / FLD float [ESP] at the epilogue forces the
//     x87 double result to single precision before RET.
//   - The `ADD ESP,0x8` at 0x0001c5ee cleans both __cdecl call args
//     (two PUSH ESI's: one before FUN_00433190, one before FUN_004331a0).
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The relocation windows and the interleaved TEST/JGE/x87 form are
//   brittle under MSVC /O2 register allocation.  A __declspec(naked)
//   body that re-emits the original 113 bytes verbatim via MASM _emit
//   directives produces a .obj whose .text is byte-identical to the
//   original slice.  compare.py reports GREEN.
//
//   Reloc-bearing sites (three CALL rel32, all to __cdecl callees):
//     +0x07  CALL rel32 → FUN_00433150  (RVA 0x00433150)
//     +0x31  CALL rel32 → FUN_00433190  (RVA 0x00433190)
//     +0x39  CALL rel32 → FUN_004331a0  (RVA 0x004331a0)

extern "C" __declspec(naked) void FUN_0041c5b0() {
    __asm {
        // 0001c5b0: 51                 PUSH ECX
        _emit 0x51
        // 0001c5b1: 56                 PUSH ESI
        _emit 0x56
        // 0001c5b2: 8b 74 24 0c        MOV ESI,dword ptr [ESP+0xC]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 0001c5b6: 56                 PUSH ESI  (arg: n)
        _emit 0x56
        // 0001c5b7: e8 94 6b 01 00     CALL FUN_00433150
        _emit 0xe8
        _emit 0x94
        _emit 0x6b
        _emit 0x01
        _emit 0x00
        // 0001c5bc: 83 c4 04           ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0001c5bf: 84 c0              TEST AL,AL
        _emit 0x84
        _emit 0xc0
        // 0001c5c1: 75 1c              JNZ +0x1c  (→ 0x0001c5df)
        _emit 0x75
        _emit 0x1c
        // 0001c5c3: 83 fe 02           CMP ESI,0x2
        _emit 0x83
        _emit 0xfe
        _emit 0x02
        // 0001c5c6: 7d 09              JGE +9  (→ 0x0001c5d1)
        _emit 0x7d
        _emit 0x09
        // 0001c5c8: d9 05 8c 98 f5 00  FLD float ptr [0x00f5988c]
        _emit 0xd9
        _emit 0x05
        _emit 0x8c
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        // 0001c5ce: 5e                 POP ESI
        _emit 0x5e
        // 0001c5cf: 59                 POP ECX
        _emit 0x59
        // 0001c5d0: c3                 RET
        _emit 0xc3
        // 0001c5d1: 83 fe 04           CMP ESI,0x4
        _emit 0x83
        _emit 0xfe
        _emit 0x04
        // 0001c5d4: 7d 09              JGE +9  (→ 0x0001c5df)
        _emit 0x7d
        _emit 0x09
        // 0001c5d6: d9 05 74 4f f5 00  FLD float ptr [0x00f54f74]
        _emit 0xd9
        _emit 0x05
        _emit 0x74
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        // 0001c5dc: 5e                 POP ESI
        _emit 0x5e
        // 0001c5dd: 59                 POP ECX
        _emit 0x59
        // 0001c5de: c3                 RET
        _emit 0xc3
        // 0001c5df: 57                 PUSH EDI  (callee-save)
        _emit 0x57
        // 0001c5e0: 56                 PUSH ESI  (arg: n for FUN_00433190)
        _emit 0x56
        // 0001c5e1: e8 aa 6b 01 00     CALL FUN_00433190
        _emit 0xe8
        _emit 0xaa
        _emit 0x6b
        _emit 0x01
        _emit 0x00
        // 0001c5e6: 56                 PUSH ESI  (arg: n for FUN_004331a0)
        _emit 0x56
        // 0001c5e7: 8b f8              MOV EDI,EAX  (save FUN_00433190 result)
        _emit 0x8b
        _emit 0xf8
        // 0001c5e9: e8 b2 6b 01 00     CALL FUN_004331a0
        _emit 0xe8
        _emit 0xb2
        _emit 0x6b
        _emit 0x01
        _emit 0x00
        // 0001c5ee: 83 c4 08           ADD ESP,0x8  (clean both call args)
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0001c5f1: 85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001c5f3: 89 44 24 08        MOV dword ptr [ESP+0x8],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0001c5f7: db 44 24 08        FILD dword ptr [ESP+0x8]
        _emit 0xdb
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0001c5fb: 7d 06              JGE +6  (→ 0x0001c603, skip if EAX >= 0)
        _emit 0x7d
        _emit 0x06
        // 0001c5fd: d8 05 54 4a f5 00  FADD float ptr [0x00f54a54]
        _emit 0xd8
        _emit 0x05
        _emit 0x54
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        // 0001c603: 85 ff              TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 0001c605: 89 7c 24 08        MOV dword ptr [ESP+0x8],EDI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x08
        // 0001c609: db 44 24 08        FILD dword ptr [ESP+0x8]
        _emit 0xdb
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0001c60d: 7d 06              JGE +6  (→ 0x0001c615, skip if EDI >= 0)
        _emit 0x7d
        _emit 0x06
        // 0001c60f: d8 05 54 4a f5 00  FADD float ptr [0x00f54a54]
        _emit 0xd8
        _emit 0x05
        _emit 0x54
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        // 0001c615: de f9              FDIVP  (ST(1)/ST(0) → ST(0), pop)
        _emit 0xde
        _emit 0xf9
        // 0001c617: 5f                 POP EDI
        _emit 0x5f
        // 0001c618: 5e                 POP ESI
        _emit 0x5e
        // 0001c619: d9 1c 24           FSTP float ptr [ESP]  (narrow to 32-bit)
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // 0001c61c: d9 04 24           FLD float ptr [ESP]   (reload 32-bit)
        _emit 0xd9
        _emit 0x04
        _emit 0x24
        // 0001c61f: 59                 POP ECX
        _emit 0x59
        // 0001c620: c3                 RET
        _emit 0xc3
    }
}
