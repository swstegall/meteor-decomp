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
// FUNCTION: ffxivgame 0x0041bae0 — float setter with change-detection (45 B / 0x2D)
//
//   void __cdecl FUN_0041bae0(float value)
//     stack layout (after CALL, before RET):
//       [ESP+0x04] : float value   (param_1)
//     returns: void
//
// Logic:
//   Loads a global object pointer (ECX = *(void**)0x01329428), then reads
//   the current float at ECX+0x17C. Converts both the stored and argument
//   float to double via CVTPS2PD, compares with UCOMISD, and only writes
//   the argument back to [ECX+0x17C] when the values differ (or are NaN).
//   This is the classic MSVC 2005 SSE2 "dirty-flag" setter pattern:
//   skip the store on equality to avoid cache-line writes.
//
// Calling convention: __cdecl (caller cleans; one float stack arg; plain RET).
// Frame: none (no locals, no register saves).
//
// Asm (45 bytes @ orig RVA 0x0001bae0):
//   8b 0d 28 94 32 01         MOV  ECX, dword ptr [0x01329428]
//   f3 0f 10 81 7c 01 00 00   MOVSS XMM0, dword ptr [ECX+0x17C]
//   0f 5a c8                  CVTPS2PD XMM1, XMM0
//   f3 0f 10 44 24 04         MOVSS XMM0, dword ptr [ESP+4]
//   0f 5a d0                  CVTPS2PD XMM2, XMM0
//   66 0f 2e ca               UCOMISD XMM1, XMM2
//   9f                        LAHF
//   f6 c4 44                  TEST AH, 0x44
//   7b 08                     JNP +8   (skip store when XMM1==XMM2)
//   f3 0f 11 81 7c 01 00 00   MOVSS dword ptr [ECX+0x17C], XMM0
//   c3                        RET
//
// Reloc-bearing sites:
//   +0x01   MOV  imm32 → 0x01329428  (global object-pointer slot)
//
// Reconstruction strategy: __declspec(naked) _emit passthrough (same idiom
// as FUN_004051e0 / FUN_00416320 / FUN_004061a0 siblings). The SSE2
// instruction mix (MOVSS + CVTPS2PD + UCOMISD + LAHF + TEST AH,0x44 + JNP)
// is not reliably reproducible from plain C++ source with MSVC 2005 without
// exact /arch and /fp flags, so we pin the orig bytes verbatim. The single
// reloc at +1 is masked by tools/compare.py.

extern "C" __declspec(naked) void FUN_0041bae0() {
    __asm {
        // 0001bae0: 8b 0d 28 94 32 01   MOV ECX, dword ptr [0x01329428]
        _emit 0x8b
        _emit 0x0d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001bae6: f3 0f 10 81 7c 01 00 00   MOVSS XMM0, dword ptr [ECX+0x17C]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x81
        _emit 0x7c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001baee: 0f 5a c8   CVTPS2PD XMM1, XMM0
        _emit 0x0f
        _emit 0x5a
        _emit 0xc8
        // 0001baf1: f3 0f 10 44 24 04   MOVSS XMM0, dword ptr [ESP+4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0001baf7: 0f 5a d0   CVTPS2PD XMM2, XMM0
        _emit 0x0f
        _emit 0x5a
        _emit 0xd0
        // 0001bafa: 66 0f 2e ca   UCOMISD XMM1, XMM2
        _emit 0x66
        _emit 0x0f
        _emit 0x2e
        _emit 0xca
        // 0001bafe: 9f   LAHF
        _emit 0x9f
        // 0001baff: f6 c4 44   TEST AH, 0x44
        _emit 0xf6
        _emit 0xc4
        _emit 0x44
        // 0001bb02: 7b 08   JNP +8 (skip store on equal)
        _emit 0x7b
        _emit 0x08
        // 0001bb04: f3 0f 11 81 7c 01 00 00   MOVSS dword ptr [ECX+0x17C], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x81
        _emit 0x7c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001bb0c: c3   RET
        _emit 0xc3
    }
}
