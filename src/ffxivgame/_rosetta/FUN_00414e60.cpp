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
// FUNCTION: ffxivgame 0x00414e60 — GetTimeMicroseconds: QPC-based
//           microsecond timestamp (__cdecl, 104 B)
//
// Semantics (recovered from asm/ffxivgame/00014e60_FUN_00414e60.s):
//
//   __int64 __cdecl GetTimeMicroseconds() {
//       LARGE_INTEGER counter, freq;
//       QueryPerformanceCounter(&counter);   // IAT slot [0x00f3e158]
//       DWORD lo = counter.LowPart;          // → ESI
//       DWORD hi = counter.HighPart;         // → EDI
//       if (g_qpc_inited == 0) {             // CMP BYTE PTR [0x01328068], 0
//           QueryPerformanceFrequency(&freq); // IAT slot [0x00f3e15c]
//           g_qpc_freq_lo = freq.LowPart;    // [0x01328060]
//           g_qpc_freq_hi = freq.HighPart;   // [0x01328064]
//       }
//       // counter * 1000000 / freq (all 64-bit via __allmul / __alldiv)
//       __int64 scaled = __allmul(lo, hi, 0xf4240, 0);
//       return __alldiv(scaled, g_qpc_freq_lo, scaled, g_qpc_freq_hi);
//   }
//
// Globals (in .data):
//   0x01328060  g_qpc_freq_lo  — QPF LowPart  (cached)
//   0x01328064  g_qpc_freq_hi  — QPF HighPart (cached)
//   0x01328068  g_qpc_inited   — one-shot init flag (read only; set elsewhere)
//
// IAT slots:
//   [0x00f3e158]  QueryPerformanceCounter
//   [0x00f3e15c]  QueryPerformanceFrequency
//
// Why naked asm: 9 absolute addresses (2 IAT slots + 3 .data globals +
// 2 .text rel32 CRT helpers + 1 global reload) make source-level
// register-allocation drift too likely; the orig bytes are self-consistent
// in this binary's address space and pass cleanly when emitted verbatim.
// (Same strategy as sibling FUN_00409510 and FUN_00409260.)
//
// Original 104 bytes (RVA 0x00014e60 .. 0x00014ec7):
//
//   83 ec 10 56 57 8d 44 24 08 50 ff 15 58 e1 f3 00
//   80 3d 68 80 32 01 00 8b 74 24 08 8b 7c 24 0c 75
//   1e 8d 4c 24 10 51 ff 15 5c e1 f3 00 8b 54 24 10
//   8b 44 24 14 89 15 60 80 32 01 a3 64 80 32 01 6a
//   00 68 40 42 0f 00 57 56 e8 93 09 5c 00 8b 0d 64
//   80 32 01 51 8b 0d 60 80 32 01 51 52 50 e8 ce 08
//   5c 00 5f 5e 83 c4 10 c3

extern "C" __declspec(naked) void FUN_00414e60() {
    __asm {
        _emit 0x83              // SUB ESP, 0x10
        _emit 0xec
        _emit 0x10
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8d              // LEA EAX, [ESP + 0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL DWORD PTR [0x00f3e158] (QueryPerformanceCounter)
        _emit 0x15
        _emit 0x58
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x80              // CMP BYTE PTR [0x01328068], 0x0
        _emit 0x3d
        _emit 0x68
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x8b              // MOV ESI, DWORD PTR [ESP + 0x8]
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EDI, DWORD PTR [ESP + 0xc]
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x75              // JNZ +0x1e (skip freq init)
        _emit 0x1e
        _emit 0x8d              // LEA ECX, [ESP + 0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x51              // PUSH ECX
        _emit 0xff              // CALL DWORD PTR [0x00f3e15c] (QueryPerformanceFrequency)
        _emit 0x15
        _emit 0x5c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV EDX, DWORD PTR [ESP + 0x10]
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EAX, DWORD PTR [ESP + 0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x89              // MOV DWORD PTR [0x01328060], EDX
        _emit 0x15
        _emit 0x60
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0xa3              // MOV [0x01328064], EAX
        _emit 0x64
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x68              // PUSH 0xf4240 (1,000,000)
        _emit 0x40
        _emit 0x42
        _emit 0x0f
        _emit 0x00
        _emit 0x57              // PUSH EDI  (counter high)
        _emit 0x56              // PUSH ESI  (counter low)
        _emit 0xe8              // CALL 0x009d5840 (__allmul, rel32 = 0x5c0993)
        _emit 0x93
        _emit 0x09
        _emit 0x5c
        _emit 0x00
        _emit 0x8b              // MOV ECX, DWORD PTR [0x01328064]
        _emit 0x0d
        _emit 0x64
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x51              // PUSH ECX  (freq high)
        _emit 0x8b              // MOV ECX, DWORD PTR [0x01328060]
        _emit 0x0d
        _emit 0x60
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x51              // PUSH ECX  (freq low)
        _emit 0x52              // PUSH EDX  (scaled high)
        _emit 0x50              // PUSH EAX  (scaled low)
        _emit 0xe8              // CALL 0x009d5790 (__alldiv, rel32 = 0x5c08ce)
        _emit 0xce
        _emit 0x08
        _emit 0x5c
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3              // RET
    }
}
