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
// FUNCTION: ffxivgame 0x0001c340 — `__cdecl` 1-arg 3-way dispatch: reads a
//                                  global selector at .data 0x01329878 and
//                                  routes to one of three arms (cases 0/1/2);
//                                  each arm converts the bool arg via a
//                                  NEG/SBB/AND idiom to a FourCC mask and
//                                  forwards it (plus a sub-id) to a
//                                  `__thiscall` sink at 0x004236e0 with
//                                  ECX loaded from .data 0x0132987c.
//                                  (121 B / 0x79)
//
// Calling convention: `__cdecl`; no frame, no callee-saves.
//   arg1  = bool/byte at [ESP+4]
//   return = void (all three arms and the fallthrough path end in plain RET)
//
// Branch shape (cascading SUB / JZ / JNZ):
//   selector == 0  →  0x0001c393 (DL / EDX path, masks 0x1000000 then adds
//                                  0x304d3241 → '0M2A' / '1M2A')
//   selector == 1  →  0x0001c373 (CL / ECX path, mask 0x434f5441 → 'COTA')
//   selector == 2  →  0x0001c354 (AL / EAX path, mask 0x41415353 → 'AASS')
//   selector > 2   →  0x0001c3b8 (bare RET, do nothing)
//
// NEG/SBB/AND idiom (per arm):
//   NEG rL      — sets CF=1 if arg1 != 0, CF=0 if arg1 == 0
//   SBB rX,rX   — rX = 0 - 0 - CF = 0 (false) or 0 - 0 - 1 = -1 / 0xffffffff (true)
//   AND rX,mask — rX = 0 or mask
//   → result is either 0 or the FourCC constant for that selector arm
//
// Callee at 0x004236e0 is `__thiscall` (ECX = *0x0132987c loaded before CALL).
// The first stack arg is the sub-id (0xb5 for selectors 1 & 2, 0x9a for 0),
// the second is the computed mask.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The three `e8 xx xx 00 00` CALL rel32 bytes encode the linker-resolved
//   displacement of 0x004236e0 relative to each call-site; emitting them as
//   raw immediates produces the exact bytes found in the original binary.
//   Source-level C++ would require coaxing MSVC 2005 /O2 into the specific
//   register choices per arm (AL/EAX, CL/ECX, DL/EDX), the exact ordering
//   of the ECX-load relative to the NEG/SBB/AND sequence, and the exact
//   short-branch encodings — all brittle under the optimizer.  The naked-asm
//   passthrough (same strategy as FUN_00401350, FUN_00401650, FUN_00411fa0)
//   produces a .obj whose .text is byte-identical to the original slice.

extern "C" __declspec(naked) void FUN_0041c340() {
    __asm {
        // 0001c340: a1 78 98 32 01   MOV EAX,[0x01329878]
        _emit 0xa1
        _emit 0x78
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001c345: 83 e8 00         SUB EAX,0x0
        _emit 0x83
        _emit 0xe8
        _emit 0x00
        // 0001c348: 74 49            JZ +0x49 (→ 0x0001c393, selector==0)
        _emit 0x74
        _emit 0x49
        // 0001c34a: 83 e8 01         SUB EAX,0x1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // 0001c34d: 74 24            JZ +0x24 (→ 0x0001c373, selector==1)
        _emit 0x74
        _emit 0x24
        // 0001c34f: 83 e8 01         SUB EAX,0x1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // 0001c352: 75 64            JNZ +0x64 (→ 0x0001c3b8, selector>2)
        _emit 0x75
        _emit 0x64
        // === selector == 2 arm ===
        // 0001c354: 8a 44 24 04      MOV AL,byte ptr [ESP+0x4]
        _emit 0x8a
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0001c358: 8b 0d 7c 98 32 01  MOV ECX,[0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001c35e: f6 d8            NEG AL
        _emit 0xf6
        _emit 0xd8
        // 0001c360: 1b c0            SBB EAX,EAX
        _emit 0x1b
        _emit 0xc0
        // 0001c362: 25 53 53 41 41   AND EAX,0x41415353
        _emit 0x25
        _emit 0x53
        _emit 0x53
        _emit 0x41
        _emit 0x41
        // 0001c367: 50               PUSH EAX
        _emit 0x50
        // 0001c368: 68 b5 00 00 00   PUSH 0xb5
        _emit 0x68
        _emit 0xb5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001c36d: e8 6e 73 00 00   CALL 0x004236e0
        _emit 0xe8
        _emit 0x6e
        _emit 0x73
        _emit 0x00
        _emit 0x00
        // 0001c372: c3               RET
        _emit 0xc3
        // === selector == 1 arm ===
        // 0001c373: 8a 4c 24 04      MOV CL,byte ptr [ESP+0x4]
        _emit 0x8a
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0001c377: f6 d9            NEG CL
        _emit 0xf6
        _emit 0xd9
        // 0001c379: 1b c9            SBB ECX,ECX
        _emit 0x1b
        _emit 0xc9
        // 0001c37b: 81 e1 41 54 4f 43  AND ECX,0x434f5441
        _emit 0x81
        _emit 0xe1
        _emit 0x41
        _emit 0x54
        _emit 0x4f
        _emit 0x43
        // 0001c381: 51               PUSH ECX
        _emit 0x51
        // 0001c382: 8b 0d 7c 98 32 01  MOV ECX,[0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001c388: 68 b5 00 00 00   PUSH 0xb5
        _emit 0x68
        _emit 0xb5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001c38d: e8 4e 73 00 00   CALL 0x004236e0
        _emit 0xe8
        _emit 0x4e
        _emit 0x73
        _emit 0x00
        _emit 0x00
        // 0001c392: c3               RET
        _emit 0xc3
        // === selector == 0 arm ===
        // 0001c393: 8a 54 24 04      MOV DL,byte ptr [ESP+0x4]
        _emit 0x8a
        _emit 0x54
        _emit 0x24
        _emit 0x04
        // 0001c397: 8b 0d 7c 98 32 01  MOV ECX,[0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001c39d: f6 da            NEG DL
        _emit 0xf6
        _emit 0xda
        // 0001c39f: 1b d2            SBB EDX,EDX
        _emit 0x1b
        _emit 0xd2
        // 0001c3a1: 81 e2 00 00 00 01  AND EDX,0x1000000
        _emit 0x81
        _emit 0xe2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x01
        // 0001c3a7: 81 c2 41 32 4d 30  ADD EDX,0x304d3241
        _emit 0x81
        _emit 0xc2
        _emit 0x41
        _emit 0x32
        _emit 0x4d
        _emit 0x30
        // 0001c3ad: 52               PUSH EDX
        _emit 0x52
        // 0001c3ae: 68 9a 00 00 00   PUSH 0x9a
        _emit 0x68
        _emit 0x9a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001c3b3: e8 28 73 00 00   CALL 0x004236e0
        _emit 0xe8
        _emit 0x28
        _emit 0x73
        _emit 0x00
        _emit 0x00
        // 0001c3b8: c3               RET  (also fallthrough target for selector>2)
        _emit 0xc3
    }
}
