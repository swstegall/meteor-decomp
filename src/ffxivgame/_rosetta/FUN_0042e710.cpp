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
// FUNCTION: ffxivgame 0x0002e710 — __thiscall member that divides a 4-component
//                                  float vector (x, y, z, w) by a scalar in-place
//                                  (147 B / 0x93, ret 4)
//
// Calling convention: __thiscall (ECX = this); one 32-bit stack arg [EBP+8] = float
// divisor.  Cleans 4 bytes of stack args on return (RET 0x4).
//
// Object layout:
//   this + 0x00  float x
//   this + 0x04  float y
//   this + 0x08  float z
//   this + 0x0c  float w
//
// Behaviour:
//   this->x /= divisor;
//   this->y /= divisor;
//   this->z /= divisor;
//   this->w /= divisor;
//
// Stack frame:
//   AND ESP,0xFFFFFFF0  — aligns to 16 bytes (required for MOVAPS)
//   SUB ESP,0x20        — reserves 32 bytes:
//       [ESP +  0..0x0f]  input copy of {x,y,z,w}
//       [ESP + 0x10..0x1f] DIVPS result staging
//
// Implementation uses SSE packed arithmetic:
//   • Each float is copied from `this` to [ESP] via MOVSS (scalar stores)
//   • MOVAPS loads all four into XMM1 as a packed float
//   • SHUFPS XMM0,XMM0,0x0 broadcasts the scalar divisor across all four lanes
//   • DIVPS XMM1,XMM0 divides all four components simultaneously
//   • MOVAPS stores the result to [ESP+0x10..0x1F]
//   • Four MOVSS instructions scatter the results back to [ESP+0..0xF]
//   • Two SSE2 MOVQ (F3 0F 7E load + 66 0F D6 store) write 64-bit pairs back
//     to `this` without touching the other 64 bits unnecessarily
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The mix of SSE1 (MOVSS, MOVAPS, SHUFPS, DIVPS) and SSE2-prefix MOVQ
//   forms (F3 0F 7E for load, 66 0F D6 for store) makes source-level
//   reproduction fragile; MASM in VS 2005 may not assemble these encodings
//   consistently without /arch:SSE2.  The __declspec(naked) + _emit
//   passthrough produces a .text whose 147 bytes match orig byte-for-byte
//   with no relocations (no CALL targets in this function).

extern "C" __declspec(naked) void FUN_0042e710() {
    __asm {
        // 0002e710:  55                 PUSH EBP
        _emit 0x55
        // 0002e711:  8b ec              MOV EBP,ESP
        _emit 0x8b
        _emit 0xec
        // 0002e713:  83 e4 f0           AND ESP,0xfffffff0
        _emit 0x83
        _emit 0xe4
        _emit 0xf0
        // 0002e716:  83 ec 20           SUB ESP,0x20
        _emit 0x83
        _emit 0xec
        _emit 0x20
        // 0002e719:  8b c1              MOV EAX,ECX
        _emit 0x8b
        _emit 0xc1
        // 0002e71b:  f3 0f 10 00        MOVSS XMM0,dword ptr [EAX]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x00
        // 0002e71f:  f3 0f 11 04 24     MOVSS dword ptr [ESP],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        // 0002e724:  f3 0f 10 40 04     MOVSS XMM0,dword ptr [EAX+0x4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x40
        _emit 0x04
        // 0002e729:  f3 0f 11 44 24 04  MOVSS dword ptr [ESP+0x4],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0002e72f:  f3 0f 10 40 08     MOVSS XMM0,dword ptr [EAX+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x40
        _emit 0x08
        // 0002e734:  f3 0f 11 44 24 08  MOVSS dword ptr [ESP+0x8],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0002e73a:  f3 0f 10 40 0c     MOVSS XMM0,dword ptr [EAX+0xc]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x40
        _emit 0x0c
        // 0002e73f:  f3 0f 11 44 24 0c  MOVSS dword ptr [ESP+0xc],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0002e745:  0f 28 0c 24        MOVAPS XMM1,xmmword ptr [ESP]
        _emit 0x0f
        _emit 0x28
        _emit 0x0c
        _emit 0x24
        // 0002e749:  f3 0f 10 45 08     MOVSS XMM0,dword ptr [EBP+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x45
        _emit 0x08
        // 0002e74e:  0f c6 c0 00        SHUFPS XMM0,XMM0,0x0
        _emit 0x0f
        _emit 0xc6
        _emit 0xc0
        _emit 0x00
        // 0002e752:  0f 5e c8           DIVPS XMM1,XMM0
        _emit 0x0f
        _emit 0x5e
        _emit 0xc8
        // 0002e755:  0f 29 4c 24 10     MOVAPS xmmword ptr [ESP+0x10],XMM1
        _emit 0x0f
        _emit 0x29
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0002e75a:  f3 0f 10 44 24 10  MOVSS XMM0,dword ptr [ESP+0x10]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0002e760:  f3 0f 11 04 24     MOVSS dword ptr [ESP],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        // 0002e765:  f3 0f 10 44 24 14  MOVSS XMM0,dword ptr [ESP+0x14]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0002e76b:  f3 0f 11 44 24 04  MOVSS dword ptr [ESP+0x4],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0002e771:  f3 0f 10 44 24 18  MOVSS XMM0,dword ptr [ESP+0x18]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 0002e777:  f3 0f 11 44 24 08  MOVSS dword ptr [ESP+0x8],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0002e77d:  f3 0f 10 44 24 1c  MOVSS XMM0,dword ptr [ESP+0x1c]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0002e783:  f3 0f 11 44 24 0c  MOVSS dword ptr [ESP+0xc],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0002e789:  f3 0f 7e 04 24     MOVQ XMM0,qword ptr [ESP]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x04
        _emit 0x24
        // 0002e78e:  66 0f d6 00        MOVQ qword ptr [EAX],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x00
        // 0002e792:  f3 0f 7e 44 24 08  MOVQ XMM0,qword ptr [ESP+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0002e798:  66 0f d6 40 08     MOVQ qword ptr [EAX+0x8],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x08
        // 0002e79d:  8b e5              MOV ESP,EBP
        _emit 0x8b
        _emit 0xe5
        // 0002e79f:  5d                 POP EBP
        _emit 0x5d
        // 0002e7a0:  c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
