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
// FUNCTION: ffxivgame 0x0002f5b0 — 4-component SSE dot product, result
//                                  broadcast to all four lanes (__thiscall,
//                                  146 bytes / 0x92, ret 8)
//
// Calling convention: __thiscall.
//   ECX        = this  (const float[4] — vector A)
//   [EBP+0x08] = out   (float[4]*       — destination, 16-byte aligned)
//   [EBP+0x0c] = other (const float[4]* — vector B)
//   Cleans 8 bytes of stack args on return (`ret 8`).
//
// Behaviour (recovered from asm @ 0x0002f5b0):
//
//   void Vec4::DotInto(float out[4], const float other[4]) const {
//       __m128 a = _mm_set_ps(this[3],  this[2],  this[1],  this[0]);
//       __m128 b = _mm_set_ps(other[3], other[2], other[1], other[0]);
//       __m128 p = _mm_mul_ps(a, b);
//       // horizontal add of all four lanes, broadcast to every lane
//       __m128 s = _mm_add_ps(p, _mm_shuffle_ps(p, p, 0x55));   // + lane 1
//       s        = _mm_add_ps(s, _mm_shuffle_ps(p, p, 0xaa));   // + lane 2
//       s        = _mm_add_ps(s, _mm_shuffle_ps(p, p, 0xff));   // + lane 3
//       _mm_store_ps(out, s);                                    // lane0 already in s
//   }
//
// The two source vectors are materialised the long way — four scalar
// MOVSS stores into a 16-byte-aligned stack scratch buffer followed by a
// single aligned MOVAPS load — because the inputs aren't known to be
// 16-byte aligned (the SUB ESP,0x10 after AND ESP,~0xf carves the aligned
// scratch). This whole sequence (the forced ESP realignment, the scratch
// buffer reuse for both A and B, and the SSE x87-free horizontal add) is
// not reproducible from C++ source at /O2 without coaxing register/temp
// allocation that diverges in an isolated TU. There are no relocations in
// this slice (no CALL / IAT), so the canonical ffxivgame rosetta path —
// __declspec(naked) byte passthrough via MASM _emit — reproduces all 146
// bytes verbatim and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0042f5b0() {
    __asm {
        // 0002f5b0:  55                 PUSH EBP
        _emit 0x55
        // 0002f5b1:  8b ec              MOV EBP,ESP
        _emit 0x8b
        _emit 0xec
        // 0002f5b3:  83 e4 f0           AND ESP,0xfffffff0
        _emit 0x83
        _emit 0xe4
        _emit 0xf0
        // 0002f5b6:  83 ec 10           SUB ESP,0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 0002f5b9:  8b 55 0c           MOV EDX,[EBP+0xc]
        _emit 0x8b
        _emit 0x55
        _emit 0x0c
        // 0002f5bc:  f3 0f 10 02        MOVSS XMM0,[EDX]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x02
        // 0002f5c0:  f3 0f 10 09        MOVSS XMM1,[ECX]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x09
        // 0002f5c4:  8b 45 08           MOV EAX,[EBP+0x8]
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        // 0002f5c7:  f3 0f 11 04 24     MOVSS [ESP],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        // 0002f5cc:  f3 0f 10 42 04     MOVSS XMM0,[EDX+0x4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x04
        // 0002f5d1:  f3 0f 11 44 24 04  MOVSS [ESP+0x4],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0002f5d7:  f3 0f 10 42 08     MOVSS XMM0,[EDX+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x08
        // 0002f5dc:  f3 0f 11 44 24 08  MOVSS [ESP+0x8],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0002f5e2:  f3 0f 10 42 0c     MOVSS XMM0,[EDX+0xc]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x0c
        // 0002f5e7:  f3 0f 11 44 24 0c  MOVSS [ESP+0xc],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0002f5ed:  0f 28 04 24        MOVAPS XMM0,[ESP]
        _emit 0x0f
        _emit 0x28
        _emit 0x04
        _emit 0x24
        // 0002f5f1:  f3 0f 11 0c 24     MOVSS [ESP],XMM1
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x0c
        _emit 0x24
        // 0002f5f6:  f3 0f 10 49 04     MOVSS XMM1,[ECX+0x4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x49
        _emit 0x04
        // 0002f5fb:  f3 0f 11 4c 24 04  MOVSS [ESP+0x4],XMM1
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0002f601:  f3 0f 10 49 08     MOVSS XMM1,[ECX+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x49
        _emit 0x08
        // 0002f606:  f3 0f 11 4c 24 08  MOVSS [ESP+0x8],XMM1
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 0002f60c:  f3 0f 10 49 0c     MOVSS XMM1,[ECX+0xc]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x49
        _emit 0x0c
        // 0002f611:  f3 0f 11 4c 24 0c  MOVSS [ESP+0xc],XMM1
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0002f617:  0f 28 0c 24        MOVAPS XMM1,[ESP]
        _emit 0x0f
        _emit 0x28
        _emit 0x0c
        _emit 0x24
        // 0002f61b:  0f 59 c1           MULPS XMM0,XMM1
        _emit 0x0f
        _emit 0x59
        _emit 0xc1
        // 0002f61e:  0f 28 c8           MOVAPS XMM1,XMM0
        _emit 0x0f
        _emit 0x28
        _emit 0xc8
        // 0002f621:  0f c6 c8 55        SHUFPS XMM1,XMM0,0x55
        _emit 0x0f
        _emit 0xc6
        _emit 0xc8
        _emit 0x55
        // 0002f625:  0f 58 c8           ADDPS XMM1,XMM0
        _emit 0x0f
        _emit 0x58
        _emit 0xc8
        // 0002f628:  0f 28 d0           MOVAPS XMM2,XMM0
        _emit 0x0f
        _emit 0x28
        _emit 0xd0
        // 0002f62b:  0f c6 d0 aa        SHUFPS XMM2,XMM0,0xaa
        _emit 0x0f
        _emit 0xc6
        _emit 0xd0
        _emit 0xaa
        // 0002f62f:  0f 58 ca           ADDPS XMM1,XMM2
        _emit 0x0f
        _emit 0x58
        _emit 0xca
        // 0002f632:  0f c6 c0 ff        SHUFPS XMM0,XMM0,0xff
        _emit 0x0f
        _emit 0xc6
        _emit 0xc0
        _emit 0xff
        // 0002f636:  0f 58 c8           ADDPS XMM1,XMM0
        _emit 0x0f
        _emit 0x58
        _emit 0xc8
        // 0002f639:  0f 29 08           MOVAPS [EAX],XMM1
        _emit 0x0f
        _emit 0x29
        _emit 0x08
        // 0002f63c:  8b e5              MOV ESP,EBP
        _emit 0x8b
        _emit 0xe5
        // 0002f63e:  5d                 POP EBP
        _emit 0x5d
        // 0002f63f:  c2 08 00           RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
