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
// FUNCTION: ffxivgame 0x00020010 — CRC32 lookup-table initialiser (144 B / 0x90)
//
// __cdecl void FUN_00420010(void):
//   No arguments, no stack frame, no callee-save pushes.  Fills the 256-entry
//   CRC32 table at global DAT_01329430 using the standard reflected polynomial
//   0xEDB88320 (IEEE 802.3 / PKZIP / zlib CRC32).
//
//   Equivalent C:
//       unsigned int *table = (unsigned int *)0x1329430;
//       for (unsigned int i = 0; i < 256; ++i) {
//           unsigned int crc = i;
//           for (int j = 0; j < 8; ++j) {
//               if (crc & 1) crc = (crc >> 1) ^ 0xedb88320u;
//               else         crc >>= 1;
//           }
//           table[i] = crc;
//       }
//
// Codegen notes:
//   ECX is the outer-loop counter (0..255); EAX accumulates the CRC bits.
//   The inner 8-iteration CRC loop is fully unrolled.  The first iteration
//   pre-shifts EAX (= ECX) right by 1 and then tests CL (the original loop
//   counter's LSB) via TEST CL,1 to decide whether to XOR with the polynomial.
//   The remaining 7 iterations each use TEST AL,1 on the already-updated
//   accumulator, followed by a branch-over-XOR pattern:
//       TEST AL,1 ; JZ +9 ; SHR EAX,1 ; XOR EAX,poly ; JMP +2 ; SHR EAX,1
//   Table write uses the SIB+disp32 form: MOV [ECX*4 + 0x1329430], EAX.
//   Outer loop test is CMP ECX,0x100 / JC (unsigned below).
//
// Reconstruction: __declspec(naked) + _emit passthrough.  All branches are
//   intra-function with fixed short displacements; no external call targets.
//   The table address 0x01329430 is emitted as a raw 32-bit immediate (no
//   COFF relocation in this obj); the raw bytes match the linked binary value.

extern "C" __declspec(naked) void FUN_00420010() {
    __asm {
        // 00020010: 33 c9  XOR ECX,ECX  (i = 0)
        _emit 0x33
        _emit 0xc9
    // loop_top (00020012):
        // 00020012: 8b c1  MOV EAX,ECX  (crc = i)
        _emit 0x8b
        _emit 0xc1
        // 00020014: d1 e8  SHR EAX,0x1
        _emit 0xd1
        _emit 0xe8
        // 00020016: f6 c1 01  TEST CL,0x1  (check original LSB of i)
        _emit 0xf6
        _emit 0xc1
        _emit 0x01
        // 00020019: 74 05  JZ +5  (skip XOR if LSB was 0)
        _emit 0x74
        _emit 0x05
        // 0002001b: 35 20 83 b8 ed  XOR EAX,0xedb88320
        _emit 0x35
        _emit 0x20
        _emit 0x83
        _emit 0xb8
        _emit 0xed
        // --- inner iteration 2 (00020020) ---
        // 00020020: a8 01  TEST AL,0x1
        _emit 0xa8
        _emit 0x01
        // 00020022: 74 09  JZ +9
        _emit 0x74
        _emit 0x09
        // 00020024: d1 e8  SHR EAX,0x1
        _emit 0xd1
        _emit 0xe8
        // 00020026: 35 20 83 b8 ed  XOR EAX,0xedb88320
        _emit 0x35
        _emit 0x20
        _emit 0x83
        _emit 0xb8
        _emit 0xed
        // 0002002b: eb 02  JMP +2  (skip else-SHR)
        _emit 0xeb
        _emit 0x02
        // 0002002d: d1 e8  SHR EAX,0x1  (else branch)
        _emit 0xd1
        _emit 0xe8
        // --- inner iteration 3 (0002002f) ---
        // 0002002f: a8 01  TEST AL,0x1
        _emit 0xa8
        _emit 0x01
        // 00020031: 74 09  JZ +9
        _emit 0x74
        _emit 0x09
        // 00020033: d1 e8  SHR EAX,0x1
        _emit 0xd1
        _emit 0xe8
        // 00020035: 35 20 83 b8 ed  XOR EAX,0xedb88320
        _emit 0x35
        _emit 0x20
        _emit 0x83
        _emit 0xb8
        _emit 0xed
        // 0002003a: eb 02  JMP +2
        _emit 0xeb
        _emit 0x02
        // 0002003c: d1 e8  SHR EAX,0x1
        _emit 0xd1
        _emit 0xe8
        // --- inner iteration 4 (0002003e) ---
        // 0002003e: a8 01  TEST AL,0x1
        _emit 0xa8
        _emit 0x01
        // 00020040: 74 09  JZ +9
        _emit 0x74
        _emit 0x09
        // 00020042: d1 e8  SHR EAX,0x1
        _emit 0xd1
        _emit 0xe8
        // 00020044: 35 20 83 b8 ed  XOR EAX,0xedb88320
        _emit 0x35
        _emit 0x20
        _emit 0x83
        _emit 0xb8
        _emit 0xed
        // 00020049: eb 02  JMP +2
        _emit 0xeb
        _emit 0x02
        // 0002004b: d1 e8  SHR EAX,0x1
        _emit 0xd1
        _emit 0xe8
        // --- inner iteration 5 (0002004d) ---
        // 0002004d: a8 01  TEST AL,0x1
        _emit 0xa8
        _emit 0x01
        // 0002004f: 74 09  JZ +9
        _emit 0x74
        _emit 0x09
        // 00020051: d1 e8  SHR EAX,0x1
        _emit 0xd1
        _emit 0xe8
        // 00020053: 35 20 83 b8 ed  XOR EAX,0xedb88320
        _emit 0x35
        _emit 0x20
        _emit 0x83
        _emit 0xb8
        _emit 0xed
        // 00020058: eb 02  JMP +2
        _emit 0xeb
        _emit 0x02
        // 0002005a: d1 e8  SHR EAX,0x1
        _emit 0xd1
        _emit 0xe8
        // --- inner iteration 6 (0002005c) ---
        // 0002005c: a8 01  TEST AL,0x1
        _emit 0xa8
        _emit 0x01
        // 0002005e: 74 09  JZ +9
        _emit 0x74
        _emit 0x09
        // 00020060: d1 e8  SHR EAX,0x1
        _emit 0xd1
        _emit 0xe8
        // 00020062: 35 20 83 b8 ed  XOR EAX,0xedb88320
        _emit 0x35
        _emit 0x20
        _emit 0x83
        _emit 0xb8
        _emit 0xed
        // 00020067: eb 02  JMP +2
        _emit 0xeb
        _emit 0x02
        // 00020069: d1 e8  SHR EAX,0x1
        _emit 0xd1
        _emit 0xe8
        // --- inner iteration 7 (0002006b) ---
        // 0002006b: a8 01  TEST AL,0x1
        _emit 0xa8
        _emit 0x01
        // 0002006d: 74 09  JZ +9
        _emit 0x74
        _emit 0x09
        // 0002006f: d1 e8  SHR EAX,0x1
        _emit 0xd1
        _emit 0xe8
        // 00020071: 35 20 83 b8 ed  XOR EAX,0xedb88320
        _emit 0x35
        _emit 0x20
        _emit 0x83
        _emit 0xb8
        _emit 0xed
        // 00020076: eb 02  JMP +2
        _emit 0xeb
        _emit 0x02
        // 00020078: d1 e8  SHR EAX,0x1
        _emit 0xd1
        _emit 0xe8
        // --- inner iteration 8 (0002007a) ---
        // 0002007a: a8 01  TEST AL,0x1
        _emit 0xa8
        _emit 0x01
        // 0002007c: 74 09  JZ +9
        _emit 0x74
        _emit 0x09
        // 0002007e: d1 e8  SHR EAX,0x1
        _emit 0xd1
        _emit 0xe8
        // 00020080: 35 20 83 b8 ed  XOR EAX,0xedb88320
        _emit 0x35
        _emit 0x20
        _emit 0x83
        _emit 0xb8
        _emit 0xed
        // 00020085: eb 02  JMP +2
        _emit 0xeb
        _emit 0x02
        // 00020087: d1 e8  SHR EAX,0x1
        _emit 0xd1
        _emit 0xe8
        // --- store + outer loop control (00020089) ---
        // 00020089: 89 04 8d 30 94 32 01  MOV dword ptr [ECX*4 + 0x1329430],EAX
        _emit 0x89
        _emit 0x04
        _emit 0x8d
        _emit 0x30
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 00020090: 83 c1 01  ADD ECX,0x1
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // 00020093: 81 f9 00 01 00 00  CMP ECX,0x100
        _emit 0x81
        _emit 0xf9
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 00020099: 0f 82 73 ff ff ff  JC loop_top (disp32 = -0x8d)
        _emit 0x0f
        _emit 0x82
        _emit 0x73
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0002009f: c3  RET
        _emit 0xc3
    }
}
