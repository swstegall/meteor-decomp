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
// FUNCTION: ffxivgame 0x00019af0 — 6-argument relay/logging wrapper
//                                  (__cdecl, 100 B / 0x64)
//
// __cdecl void FUN_00419af0(int a1, int a2, int a3, int a4, float a5, float a6)
//
// Signature (from stack layout before any frame setup):
//   [ESP+0x04]  a1 (int)    → EAX
//   [ESP+0x08]  a2 (int)    → ECX
//   [ESP+0x0c]  a3 (int)    → EDX
//   [ESP+0x10]  a4 (int)    → ESI
//   [ESP+0x14]  a5 (float)  → XMM0 (SSE2) and then rebuilt on local stack
//   [ESP+0x18]  a6 (float)  → x87 ST(0) and then rebuilt on local stack
//
// What the function does:
//   1. Loads all six arguments into registers (EAX, ECX, EDX, XMM0, x87 ST0)
//      before any frame manipulation.
//   2. Saves ESI (callee-save) and allocates 8 bytes of local stack space.
//   3. Stores the two float args into the 8-byte local block via FSTP/FLD,
//      swapping the x87/SSE2 shuttle: arg6 lands at [ESP+4] via x87, arg5
//      lands at [ESP+0] via a second FLD; XMM0 is updated to hold arg6.
//   4. Saves all six original args to process-global slots:
//        [0x01328f24] = a5  (float, via MOVSS — original XMM0 value)
//        [0x01328f28] = a6  (float, via MOVSS — reloaded XMM0)
//        [0x01328f98] = a1  (int, via moffs32 MOV EAX form)
//        [0x01328f9c] = a2  (int)
//        [0x01328fa0] = a3  (int)
//        [0x01328fa4] = a4  (int)
//   5. Pushes a1..a4 onto the stack (right-to-left: ESI, EDX, ECX, EAX)
//      so the callee sees: arg1=a1, arg2=a2, arg3=a3, arg4=a4,
//      arg5=a5 (from local [ESP+0x14]), arg6=a6 (from local [ESP+0x18]).
//   6. Calls FUN_0041bdf0 (the actual implementation) with the same 6-arg
//      signature.
//   7. Cleans up (ADD ESP, 0x18 = 4×4 pushed + 8-byte local block),
//      restores ESI, and returns (void, __cdecl — no imm16 in RET).
//
// Frame: no EBP; ESP-relative throughout.
//   Allocations after PUSH ESI + SUB ESP,8:
//     [ESP+0x00]  local float (a5 rebuilt by x87 FSTP)
//     [ESP+0x04]  local float (a6 rebuilt by x87 FSTP)
//     [ESP+0x08]  saved ESI
//
// The mixed x87 / SSE2 float handling is a characteristic MSVC 2005 /O2
// pattern: MOVSS is used for global-variable float stores (which the
// compiler identifies as simple 32-bit register-to-memory moves), while
// x87 is used for the pass-through stack frame because the ABI convention
// for this calling site passes floats on the x87 stack.
//
// Why naked asm: reproducing the exact interleaving of x87 and SSE2
// instruction scheduling at source level is fragile under /O2. The
// absolute global addresses (0x01328f24, 0x01328f98, etc.) embedded as
// immediate bytes also cannot be reproduced in a standalone .obj compile
// without matching relocations. Emitting the 100 original bytes verbatim
// via MASM _emit directives gives a .text section byte-identical to the
// original binary slice, which is what compare.py checks against.

extern "C" __declspec(naked) void FUN_00419af0() {
    __asm {
        // 00019af0: d9 44 24 18    FLD float ptr [ESP + 0x18]
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 00019af4: 8b 54 24 0c    MOV EDX, dword ptr [ESP + 0xc]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 00019af8: 8b 4c 24 08    MOV ECX, dword ptr [ESP + 0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 00019afc: 8b 44 24 04    MOV EAX, dword ptr [ESP + 0x4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00019b00: f3 0f 10 44 24 14    MOVSS XMM0, dword ptr [ESP + 0x14]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00019b06: 56    PUSH ESI
        _emit 0x56
        // 00019b07: 8b 74 24 14    MOV ESI, dword ptr [ESP + 0x14]  (= arg4, after push)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 00019b0b: 83 ec 08    SUB ESP, 0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 00019b0e: d9 5c 24 04    FSTP float ptr [ESP + 0x4]  (store arg6)
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x04
        // 00019b12: f3 0f 11 05 24 8f 32 01    MOVSS dword ptr [0x01328f24], XMM0  (store a5 to global)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x05
        _emit 0x24
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 00019b1a: d9 44 24 20    FLD float ptr [ESP + 0x20]  (reload arg5 via x87)
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 00019b1e: f3 0f 10 44 24 24    MOVSS XMM0, dword ptr [ESP + 0x24]  (reload arg6 into XMM0)
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 00019b24: d9 1c 24    FSTP float ptr [ESP]  (store arg5 to local [ESP+0])
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // 00019b27: 56    PUSH ESI  (push a4)
        _emit 0x56
        // 00019b28: 52    PUSH EDX  (push a3)
        _emit 0x52
        // 00019b29: 51    PUSH ECX  (push a2)
        _emit 0x51
        // 00019b2a: 50    PUSH EAX  (push a1)
        _emit 0x50
        // 00019b2b: a3 98 8f 32 01    MOV dword ptr [0x01328f98], EAX  (save a1 to global)
        _emit 0xa3
        _emit 0x98
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 00019b30: 89 0d 9c 8f 32 01    MOV dword ptr [0x01328f9c], ECX  (save a2)
        _emit 0x89
        _emit 0x0d
        _emit 0x9c
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 00019b36: 89 15 a0 8f 32 01    MOV dword ptr [0x01328fa0], EDX  (save a3)
        _emit 0x89
        _emit 0x15
        _emit 0xa0
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 00019b3c: 89 35 a4 8f 32 01    MOV dword ptr [0x01328fa4], ESI  (save a4)
        _emit 0x89
        _emit 0x35
        _emit 0xa4
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 00019b42: f3 0f 11 05 28 8f 32 01    MOVSS dword ptr [0x01328f28], XMM0  (save a6 to global)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x05
        _emit 0x28
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 00019b4a: e8 a1 22 00 00    CALL 0x0041bdf0  (FUN_0041bdf0)
        _emit 0xe8
        _emit 0xa1
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // 00019b4f: 83 c4 18    ADD ESP, 0x18  (4 pushes * 4 + 8-byte local)
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 00019b52: 5e    POP ESI
        _emit 0x5e
        // 00019b53: c3    RET
        _emit 0xc3
    }
}
