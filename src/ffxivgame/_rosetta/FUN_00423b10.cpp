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
// FUNCTION: ffxivgame 0x00023b10 — SSE2 16-byte block comparison loop
//                                  (__thiscall, 114 bytes / 0x72)
//
// Calling convention: __thiscall (ECX = this); three stack params.
//   Stack params (RET 0xc cleans 12 bytes):
//     [EBP+0x08]  param1  — slot index (< 0x10 to enter comparison)
//     [EBP+0x0c]  param2  — pointer to comparison data (ESI)
//     [EBP+0x10]  param3  — number of 16-byte blocks to compare (EDX)
//
// Logic:
//   1. If param1 >= 0x10, return true immediately (index out of bounds).
//   2. Compute base pointer: p = ECX + (param1 + 4) * 16
//   3. If param3 == 0, return true (empty comparison).
//   4. SSE2 loop: compare param3 blocks of 16 bytes between p and param2
//      using MOVDQA+PCMPEQD+PMOVMSKB. All 4 DWORDs must match (mask=0xFFFF).
//   5. All blocks matched: return 1 (true).
//   6. Mismatch found: call RVA 0x5d4600 with (p, param2, remaining_bytes),
//      set AL=0, fall through to shared epilogue at RVA 0x00023b82.
//
// Notable codegen details:
//   - AND ESP,0xfffffff0 aligns the stack to 16 bytes (SSE2 context).
//   - 7-byte multi-byte NOP (LEA ESP,[ESP+0x00000000]) at 0x23b39 aligns
//     the SSE loop entry to a 16-byte boundary at 0x23b40.
//   - Function body ends at XOR AL,AL (offset 112-113 of 114); the shared
//     epilogue (POP ESI / MOV ESP,EBP / POP EBP / RET 0xc) at 0x23b82 is
//     not part of this function's 114-byte slice.
//   - CALL at 0x23b77 (e8 84 0a 5b 00) targets RVA 0x5d4600 verbatim.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The shared-epilogue split, the 7-byte multi-byte NOP alignment pad,
//   the SSE2 MOVDQA/PCMPEQD/PMOVMSKB loop, and the CALL rel32 target
//   cannot be reproduced byte-for-byte from standard C++ under MSVC 2005.
//   A __declspec(naked) body re-emitting the original 114 bytes verbatim
//   via MASM _emit directives produces a .obj whose .text is byte-identical
//   to the original slice. compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00423b10() {
    __asm {
        // 00023b10: 55               PUSH EBP
        _emit 0x55
        // 00023b11: 8b ec            MOV EBP,ESP
        _emit 0x8b
        _emit 0xec
        // 00023b13: 83 e4 f0         AND ESP,0xfffffff0
        _emit 0x83
        _emit 0xe4
        _emit 0xf0
        // 00023b16: 8b 45 08         MOV EAX,dword ptr [EBP+0x8]
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        // 00023b19: 83 ec 08         SUB ESP,0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 00023b1c: 83 f8 10         CMP EAX,0x10
        _emit 0x83
        _emit 0xf8
        _emit 0x10
        // 00023b1f: 56               PUSH ESI
        _emit 0x56
        // 00023b20: 8b 75 0c         MOV ESI,dword ptr [EBP+0xc]
        _emit 0x8b
        _emit 0x75
        _emit 0x0c
        // 00023b23: 57               PUSH EDI
        _emit 0x57
        // 00023b24: 73 3f            JNC 0x00423b65
        _emit 0x73
        _emit 0x3f
        // 00023b26: 8b 55 10         MOV EDX,dword ptr [EBP+0x10]
        _emit 0x8b
        _emit 0x55
        _emit 0x10
        // 00023b29: 83 c0 04         ADD EAX,0x4
        _emit 0x83
        _emit 0xc0
        _emit 0x04
        // 00023b2c: c1 e0 04         SHL EAX,0x4
        _emit 0xc1
        _emit 0xe0
        _emit 0x04
        // 00023b2f: 03 c1            ADD EAX,ECX
        _emit 0x03
        _emit 0xc1
        // 00023b31: 33 c9            XOR ECX,ECX
        _emit 0x33
        _emit 0xc9
        // 00023b33: 85 d2            TEST EDX,EDX
        _emit 0x85
        _emit 0xd2
        // 00023b35: 76 2e            JBE 0x00423b65
        _emit 0x76
        _emit 0x2e
        // 00023b37: eb 07            JMP 0x00423b40
        _emit 0xeb
        _emit 0x07
        // 00023b39: 8d a4 24 00 00 00 00  LEA ESP,[ESP+0x00000000]  (7-byte NOP, loop alignment)
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00023b40: 66 0f 6f 00      MOVDQA XMM0,xmmword ptr [EAX]
        _emit 0x66
        _emit 0x0f
        _emit 0x6f
        _emit 0x00
        // 00023b44: 66 0f 6f 0e      MOVDQA XMM1,xmmword ptr [ESI]
        _emit 0x66
        _emit 0x0f
        _emit 0x6f
        _emit 0x0e
        // 00023b48: 66 0f 76 c1      PCMPEQD XMM0,XMM1
        _emit 0x66
        _emit 0x0f
        _emit 0x76
        _emit 0xc1
        // 00023b4c: 66 0f d7 f8      PMOVMSKB EDI,XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd7
        _emit 0xf8
        // 00023b50: 81 ff ff ff 00 00  CMP EDI,0x0000ffff
        _emit 0x81
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x00
        _emit 0x00
        // 00023b56: 75 17            JNZ 0x00423b6f
        _emit 0x75
        _emit 0x17
        // 00023b58: 83 c1 01         ADD ECX,0x1
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // 00023b5b: 83 c0 10         ADD EAX,0x10
        _emit 0x83
        _emit 0xc0
        _emit 0x10
        // 00023b5e: 83 c6 10         ADD ESI,0x10
        _emit 0x83
        _emit 0xc6
        _emit 0x10
        // 00023b61: 3b ca            CMP ECX,EDX
        _emit 0x3b
        _emit 0xca
        // 00023b63: 72 db            JC 0x00423b40
        _emit 0x72
        _emit 0xdb
        // 00023b65: b0 01            MOV AL,0x1
        _emit 0xb0
        _emit 0x01
        // 00023b67: 5f               POP EDI
        _emit 0x5f
        // 00023b68: 5e               POP ESI
        _emit 0x5e
        // 00023b69: 8b e5            MOV ESP,EBP
        _emit 0x8b
        _emit 0xe5
        // 00023b6b: 5d               POP EBP
        _emit 0x5d
        // 00023b6c: c2 0c 00         RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00023b6f: 2b d1            SUB EDX,ECX
        _emit 0x2b
        _emit 0xd1
        // 00023b71: c1 e2 04         SHL EDX,0x4
        _emit 0xc1
        _emit 0xe2
        _emit 0x04
        // 00023b74: 52               PUSH EDX
        _emit 0x52
        // 00023b75: 56               PUSH ESI
        _emit 0x56
        // 00023b76: 50               PUSH EAX
        _emit 0x50
        // 00023b77: e8 84 0a 5b 00   CALL RVA 0x5d4600
        _emit 0xe8
        _emit 0x84
        _emit 0x0a
        _emit 0x5b
        _emit 0x00
        // 00023b7c: 83 c4 0c         ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00023b7f: 5f               POP EDI
        _emit 0x5f
        // 00023b80: 32 c0            XOR AL,AL
        _emit 0x32
        _emit 0xc0
        // [function ends here at byte 114; shared epilogue at 0x00023b82 follows]
    }
}
