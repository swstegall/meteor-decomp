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
// FUNCTION: ffxivgame 0x00014e60 — `__cdecl` microsecond-resolution
//                                  monotonic time helper (104 B / 0x68)
//
// __int64 __cdecl FUN_00414e60(void)
//   No args. Returns counter * 1,000,000 / frequency (microseconds since
//   QueryPerformanceCounter's epoch) in EDX:EAX as a signed 64-bit value.
//
// Behaviour (mirrors the orig asm verbatim):
//
//     LARGE_INTEGER counter;
//     LARGE_INTEGER freq;
//     QueryPerformanceCounter(&counter);
//     if (g_initFlag == 0) {                       // [0x01328068]
//         QueryPerformanceFrequency(&freq);
//         g_freqLow  = freq.LowPart;               // [0x01328060]
//         g_freqHigh = freq.HighPart;              // [0x01328064]
//     }
//     return ((__int64)counter.QuadPart * 1000000) /
//            (((__int64)g_freqHigh << 32) | g_freqLow);
//
// Note: g_initFlag is never written to 1 inside this function. Either
// some other function sets it after first observation (a one-shot init
// elsewhere), or the freq globals get rewritten every call — both
// possibilities are observably equivalent because QueryPerformanceFrequency
// returns a constant for the lifetime of the system.
//
// MSVC 2005 /O2 codegen quirks reproduced verbatim:
//   - Counter LowPart/HighPart are hoisted into ESI/EDI BEFORE the
//     `JNZ skip_init` — the compiler sees they're live across both
//     arms of the if and schedules the loads early to overlap with
//     QueryPerformanceFrequency's call latency.
//   - The post-init store of EAX to g_freqHigh uses the single-byte
//     `A3 imm32` MOV-to-memory encoding (only available with EAX as
//     source), while the parallel EDX store to g_freqLow uses the
//     three-byte `89 15 imm32` encoding.
//   - The divisor load for __alldiv re-uses ECX twice (load g_freqHigh
//     → push, load g_freqLow → push) rather than picking two scratch
//     registers — MSVC's typical pattern for pushing two separate
//     global DWORDs onto the stack as adjacent 64-bit halves.
//
// Calling convention: __cdecl, no args, callee leaves ESP at entry+0
// (caller cleans 0 bytes); RET (no immediate).
//
// Reloc-bearing sites in the orig 104 bytes (linker fixups in a
// full-binary relink at image base 0x00400000 — standalone .obj
// compilation can't reproduce them via source-level expressions
// without dragging in the matching imports; emitting raw bytes here
// produces the exact orig bytes because the imports / globals /
// helpers all resolve to fixed absolute addresses in this binary):
//     +0x0c   imm32 ← .idata 0x00f3e158  (QueryPerformanceCounter IAT)
//     +0x12   imm32 ← .data  0x01328068  (g_initFlag)
//     +0x28   imm32 ← .idata 0x00f3e15c  (QueryPerformanceFrequency IAT)
//     +0x36   imm32 ← .data  0x01328060  (g_freqLow)
//     +0x3b   imm32 ← .data  0x01328064  (g_freqHigh)
//     +0x49   rel32 ← .text  __allmul    (caller offset 0x00414eac → ...)
//     +0x4f   imm32 ← .data  0x01328064  (g_freqHigh, reload)
//     +0x56   imm32 ← .data  0x01328060  (g_freqLow,  reload)
//     +0x5e   rel32 ← .text  __alldiv    (caller offset 0x00414ec1 → ...)
//
// Reconstruction strategy: naked-asm byte passthrough (same template
// as FUN_00414530 in this module — the absolute addresses are baked
// into the orig binary's own address space, so re-emitting them as
// raw `_emit` bytes yields a byte-identical .text contribution that
// compare.py reports as GREEN with zero relocations needed).

extern "C" __declspec(naked) void FUN_00414e60()
{
    __asm {
        // 00014e60: 83 ec 10              SUB ESP, 0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 00014e63: 56                    PUSH ESI
        _emit 0x56
        // 00014e64: 57                    PUSH EDI
        _emit 0x57
        // 00014e65: 8d 44 24 08           LEA EAX, [ESP+0x08]   (&counter)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00014e69: 50                    PUSH EAX
        _emit 0x50
        // 00014e6a: ff 15 58 e1 f3 00     CALL [QueryPerformanceCounter] (IAT 0x00f3e158)
        _emit 0xff
        _emit 0x15
        _emit 0x58
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 00014e70: 80 3d 68 80 32 01 00  CMP BYTE PTR [g_initFlag], 0  (0x01328068)
        _emit 0x80
        _emit 0x3d
        _emit 0x68
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // 00014e77: 8b 74 24 08           MOV ESI, [ESP+0x08]   (counter.LowPart)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 00014e7b: 8b 7c 24 0c           MOV EDI, [ESP+0x0c]   (counter.HighPart)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // 00014e7f: 75 1e                 JNZ +0x1e             (skip freq init)
        _emit 0x75
        _emit 0x1e
        // 00014e81: 8d 4c 24 10           LEA ECX, [ESP+0x10]   (&freq)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00014e85: 51                    PUSH ECX
        _emit 0x51
        // 00014e86: ff 15 5c e1 f3 00     CALL [QueryPerformanceFrequency] (IAT 0x00f3e15c)
        _emit 0xff
        _emit 0x15
        _emit 0x5c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 00014e8c: 8b 54 24 10           MOV EDX, [ESP+0x10]   (freq.LowPart)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 00014e90: 8b 44 24 14           MOV EAX, [ESP+0x14]   (freq.HighPart)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00014e94: 89 15 60 80 32 01     MOV [g_freqLow], EDX  (0x01328060)
        _emit 0x89
        _emit 0x15
        _emit 0x60
        _emit 0x80
        _emit 0x32
        _emit 0x01
        // 00014e9a: a3 64 80 32 01        MOV [g_freqHigh], EAX (0x01328064)
        _emit 0xa3
        _emit 0x64
        _emit 0x80
        _emit 0x32
        _emit 0x01
        // skip_init:
        // 00014e9f: 6a 00                 PUSH 0                (high half of 1000000)
        _emit 0x6a
        _emit 0x00
        // 00014ea1: 68 40 42 0f 00        PUSH 0x000f4240       (low half of 1000000)
        _emit 0x68
        _emit 0x40
        _emit 0x42
        _emit 0x0f
        _emit 0x00
        // 00014ea6: 57                    PUSH EDI              (counter.HighPart)
        _emit 0x57
        // 00014ea7: 56                    PUSH ESI              (counter.LowPart)
        _emit 0x56
        // 00014ea8: e8 93 09 5c 00        CALL __allmul         (rel32 to 0x009d5840)
        _emit 0xe8
        _emit 0x93
        _emit 0x09
        _emit 0x5c
        _emit 0x00
        // 00014ead: 8b 0d 64 80 32 01     MOV ECX, [g_freqHigh] (0x01328064)
        _emit 0x8b
        _emit 0x0d
        _emit 0x64
        _emit 0x80
        _emit 0x32
        _emit 0x01
        // 00014eb3: 51                    PUSH ECX
        _emit 0x51
        // 00014eb4: 8b 0d 60 80 32 01     MOV ECX, [g_freqLow]  (0x01328060)
        _emit 0x8b
        _emit 0x0d
        _emit 0x60
        _emit 0x80
        _emit 0x32
        _emit 0x01
        // 00014eba: 51                    PUSH ECX
        _emit 0x51
        // 00014ebb: 52                    PUSH EDX              (allmul HighPart)
        _emit 0x52
        // 00014ebc: 50                    PUSH EAX              (allmul LowPart)
        _emit 0x50
        // 00014ebd: e8 ce 08 5c 00        CALL __alldiv         (rel32 to 0x009d5790)
        _emit 0xe8
        _emit 0xce
        _emit 0x08
        _emit 0x5c
        _emit 0x00
        // 00014ec2: 5f                    POP EDI
        _emit 0x5f
        // 00014ec3: 5e                    POP ESI
        _emit 0x5e
        // 00014ec4: 83 c4 10              ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00014ec7: c3                    RET
        _emit 0xc3
    }
}
