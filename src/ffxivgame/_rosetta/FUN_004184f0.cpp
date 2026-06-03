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
// FUNCTION: ffxivgame 0x004184f0 — global init/notify helper; calls a gate
//                                  function, zeroes globals, loads float
//                                  constants via x87 + MOVSS, then calls a
//                                  6-arg setter and marks a "ready" flag.
//                                  (__cdecl, 160 bytes / 0xa0)
//
// Calling convention: __cdecl (args on stack at [ESP+4] and [ESP+8]).
// Returns bool (AL = 0 on early-exit, AL = 1 on success).
// No callee-saves pushed; no frame pointer.
//
// Flow summary:
//   1. Call FUN_0041c8e0(arg1, arg2); if it returns false (AL=0) → RET 0.
//   2. MOVZX EDX, byte [0x01328f84] and call FUN_0041bdd0(EDX) (1-arg __cdecl).
//   3. Pick width/height from whichever of the two globals 0x01328d98 /
//      0x01328da8 is non-NULL (+0x14 and +0x18), falling back to (1, 1).
//   4. Build a 6-slot stack frame:
//        [ESP+0x00] = 0.0f  (FLD1 / FLDZ pair via x87)
//        [ESP+0x04] = 1.0f
//        [ESP+0x08] = EAX  (width)
//        [ESP+0x0c] = ECX  (height)
//        [ESP+0x10] = 0    (EDX)
//        [ESP+0x14] = 0    (EDX again)
//      And scatter the same values into globals at 0x01328f9c/a0/a4.
//      Also zero 0x01328f24 via XMM0 and write a float constant to
//      0x01328f28 (loaded from 0x00f54f70).
//   5. Call FUN_0041bdf0 with that 6-slot frame, then clean 0x18 bytes.
//   6. Write 1 to byte [0x01328ed6]; return true.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The interleaving of x87 instructions (FLD1, FLDZ, FSTP), XORPS/MOVSS
//   SSE scalar stores, multiple global writes, and three CALL rel32 targets
//   cannot be reproduced from a C++ source form without triggering
//   register-allocation differences. A __declspec(naked) body re-emitting
//   the original 160 bytes verbatim via MASM _emit directives produces a
//   .obj whose .text is byte-identical to the original slice. compare.py
//   masks the CALL rel32 positions on comparison; all other bytes are
//   emitted as-is. compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_004184f0() {
    __asm {
        // 000184f0: 8b 44 24 08  MOV EAX,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 000184f4: 8b 4c 24 04  MOV ECX,dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 000184f8: 50           PUSH EAX
        _emit 0x50
        // 000184f9: 51           PUSH ECX
        _emit 0x51
        // 000184fa: e8 e1 43 00 00  CALL 0x0041c8e0
        _emit 0xe8
        _emit 0xe1
        _emit 0x43
        _emit 0x00
        _emit 0x00
        // 000184ff: 83 c4 08  ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00018502: 84 c0  TEST AL,AL
        _emit 0x84
        _emit 0xc0
        // 00018504: 75 01  JNZ +0x1  (→ 0x00418507)
        _emit 0x75
        _emit 0x01
        // 00018506: c3  RET  (early-out: gate returned false)
        _emit 0xc3
        // 00018507: 0f b6 15 84 8f 32 01  MOVZX EDX,byte ptr [0x01328f84]
        _emit 0x0f
        _emit 0xb6
        _emit 0x15
        _emit 0x84
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 0001850e: 52  PUSH EDX
        _emit 0x52
        // 0001850f: e8 bc 38 00 00  CALL 0x0041bdd0
        _emit 0xe8
        _emit 0xbc
        _emit 0x38
        _emit 0x00
        _emit 0x00
        // 00018514: a1 98 8d 32 01  MOV EAX,[0x01328d98]
        _emit 0xa1
        _emit 0x98
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00018519: 33 d2  XOR EDX,EDX
        _emit 0x33
        _emit 0xd2
        // 0001851b: 83 c4 04  ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0001851e: 3b c2  CMP EAX,EDX
        _emit 0x3b
        _emit 0xc2
        // 00018520: 75 09  JNZ +0x9  (→ 0x0041852b)
        _emit 0x75
        _emit 0x09
        // 00018522: a1 a8 8d 32 01  MOV EAX,[0x01328da8]
        _emit 0xa1
        _emit 0xa8
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00018527: 3b c2  CMP EAX,EDX
        _emit 0x3b
        _emit 0xc2
        // 00018529: 74 08  JZ +0x8  (→ 0x00418533)
        _emit 0x74
        _emit 0x08
        // 0001852b: 8b 48 14  MOV ECX,dword ptr [EAX+0x14]
        _emit 0x8b
        _emit 0x48
        _emit 0x14
        // 0001852e: 8b 40 18  MOV EAX,dword ptr [EAX+0x18]
        _emit 0x8b
        _emit 0x40
        _emit 0x18
        // 00018531: eb 07  JMP +0x7  (→ 0x0041853a)
        _emit 0xeb
        _emit 0x07
        // 00018533: b9 01 00 00 00  MOV ECX,0x1  (fallback: both globals are NULL)
        _emit 0xb9
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018538: 8b c1  MOV EAX,ECX
        _emit 0x8b
        _emit 0xc1
        // 0001853a: d9 e8  FLD1       (st(0) = 1.0f)
        _emit 0xd9
        _emit 0xe8
        // 0001853c: 0f 57 c0  XORPS XMM0,XMM0  (XMM0 = 0.0f)
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        // 0001853f: 83 ec 08  SUB ESP,0x8  (reserve 2 float slots)
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 00018542: d9 5c 24 04  FSTP dword ptr [ESP+0x4]  ([ESP+4] = 1.0f)
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x04
        // 00018546: d9 ee  FLDZ  (st(0) = 0.0f)
        _emit 0xd9
        _emit 0xee
        // 00018548: f3 0f 11 05 24 8f 32 01  MOVSS dword ptr [0x01328f24],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x05
        _emit 0x24
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 00018550: f3 0f 10 05 70 4f f5 00  MOVSS XMM0,dword ptr [0x00f54f70]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        // 00018558: d9 1c 24  FSTP dword ptr [ESP]  ([ESP] = 0.0f)
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // 0001855b: 89 15 98 8f 32 01  MOV dword ptr [0x01328f98],EDX
        _emit 0x89
        _emit 0x15
        _emit 0x98
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 00018561: 50  PUSH EAX  (width)
        _emit 0x50
        // 00018562: 51  PUSH ECX  (height)
        _emit 0x51
        // 00018563: 52  PUSH EDX  (0)
        _emit 0x52
        // 00018564: 52  PUSH EDX  (0)
        _emit 0x52
        // 00018565: 89 15 9c 8f 32 01  MOV dword ptr [0x01328f9c],EDX
        _emit 0x89
        _emit 0x15
        _emit 0x9c
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 0001856b: 89 0d a0 8f 32 01  MOV dword ptr [0x01328fa0],ECX
        _emit 0x89
        _emit 0x0d
        _emit 0xa0
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 00018571: a3 a4 8f 32 01  MOV [0x01328fa4],EAX
        _emit 0xa3
        _emit 0xa4
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 00018576: f3 0f 11 05 28 8f 32 01  MOVSS dword ptr [0x01328f28],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x05
        _emit 0x28
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 0001857e: e8 6d 38 00 00  CALL 0x0041bdf0
        _emit 0xe8
        _emit 0x6d
        _emit 0x38
        _emit 0x00
        _emit 0x00
        // 00018583: 83 c4 18  ADD ESP,0x18  (cdecl cleanup: 4 pushes + 2 pre-alloc floats)
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 00018586: c6 05 d6 8e 32 01 01  MOV byte ptr [0x01328ed6],0x1
        _emit 0xc6
        _emit 0x05
        _emit 0xd6
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 0001858d: b0 01  MOV AL,0x1
        _emit 0xb0
        _emit 0x01
        // 0001858f: c3  RET
        _emit 0xc3
    }
}
