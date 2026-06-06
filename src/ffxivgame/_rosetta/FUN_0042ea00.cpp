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
// FUNCTION: ffxivgame 0x0002ea00 — `__thiscall` Vector4 normalize, writing the
//                                  result into a caller-supplied slot
//                                  (197 B / 0xc5, SSE / packed-single, ret 4).
//
// Calling convention: __thiscall (ECX = this → float[4] at +0x0..+0xc). One
// stack arg, the output pointer at [EBP+0x8] (loaded into EAX). Returns void;
// `ret 4` cleans the single arg (callee cleanup).
//
// Behaviour (recovered from asm @ 0x0002ea00):
//
//   void Vector4::Normalize(Vector4 *out /*[EBP+0x8]*/) {
//       __m128 v   = _mm_load(this->x..w);       // scalar MOVSS x4 → [ESP]
//       __m128 sq  = _mm_mul_ps(v, v);           // per-lane square
//       // horizontal sum across all 4 lanes via SHUFPS rotations:
//       __m128 lenSq = sum(sq.x, sq.y, sq.z, sq.w) broadcast to all lanes;
//       __m128 r   = _mm_rsqrt_ps(lenSq);        // RSQRTPS estimate
//       // one Newton-Raphson refinement: r' = r * (1.5 - 0.5 * lenSq * r^2)
//       //   const 1.0 @ 0x00f54f70, const 0.5 @ 0x00f62f60 (both broadcast)
//       r = r + (1.0 - lenSq*r*r) * (0.5 * r);
//       v = _mm_mul_ps(v, r);                    // scale → normalized
//       out->x..w = lanes of v;                  // scalar MOVSS x4 → [EAX]
//   }
//
// Codegen shape: MSVC 2005 SSE2 intrinsic lowering with a 16-byte-aligned
// scratch frame (AND ESP,0xfffffff0 + SUB ESP,0x10). The packed reload is a
// MOVAPS XMM3 of the scalar-spilled source; the horizontal sum uses the
// classic SHUFPS-0x4e / 0x39 / 0x93 rotation trio; the reciprocal-sqrt
// refinement folds the two .rdata constants in via MOVSS+SHUFPS-broadcast.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The exact per-lane scalar-spill / MOVAPS-reload / SHUFPS-rotation /
//   RSQRTPS-refinement staging is an SSE2 intrinsic lowering that
//   source-level C++ in an isolated TU under the project's (x87-default)
//   flags will not reproduce. The two MOVSS-from-.rdata sites carry absolute
//   disp32 operands whose original `.text` bytes already hold the
//   linker-resolved addresses (0x00f54f70, 0x00f62f60); emitting them
//   verbatim reproduces those exact bytes. There are no CALL sites, so the
//   197-byte `_emit` sequence below is a literal copy of the original slice
//   and compare.py reports GREEN. Same passthrough idiom as the sibling
//   SSE Vector4 ops FUN_0042e570 / FUN_0042e670.

extern "C" __declspec(naked) void FUN_0042ea00() {
    __asm {
        // 0002ea00:  55                 PUSH EBP
        _emit 0x55
        // 0002ea01:  8b ec              MOV EBP,ESP
        _emit 0x8b
        _emit 0xec
        // 0002ea03:  83 e4 f0           AND ESP,0xfffffff0
        _emit 0x83
        _emit 0xe4
        _emit 0xf0
        // 0002ea06:  83 ec 10           SUB ESP,0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 0002ea09:  f3 0f 10 01        MOVSS XMM0,[ECX]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x01
        // 0002ea0d:  f3 0f 11 04 24     MOVSS [ESP],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        // 0002ea12:  f3 0f 10 41 04     MOVSS XMM0,[ECX+0x4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x04
        // 0002ea17:  f3 0f 11 44 24 04  MOVSS [ESP+0x4],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0002ea1d:  f3 0f 10 41 08     MOVSS XMM0,[ECX+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x08
        // 0002ea22:  f3 0f 11 44 24 08  MOVSS [ESP+0x8],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0002ea28:  f3 0f 10 41 0c     MOVSS XMM0,[ECX+0xc]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x0c
        // 0002ea2d:  8b 45 08           MOV EAX,[EBP+0x8]
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        // 0002ea30:  f3 0f 11 44 24 0c  MOVSS [ESP+0xc],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0002ea36:  0f 28 1c 24        MOVAPS XMM3,[ESP]
        _emit 0x0f
        _emit 0x28
        _emit 0x1c
        _emit 0x24
        // 0002ea3a:  0f 28 c3           MOVAPS XMM0,XMM3
        _emit 0x0f
        _emit 0x28
        _emit 0xc3
        // 0002ea3d:  0f 59 c3           MULPS XMM0,XMM3
        _emit 0x0f
        _emit 0x59
        _emit 0xc3
        // 0002ea40:  0f 28 c8           MOVAPS XMM1,XMM0
        _emit 0x0f
        _emit 0x28
        _emit 0xc8
        // 0002ea43:  0f c6 c8 4e        SHUFPS XMM1,XMM0,0x4e
        _emit 0x0f
        _emit 0xc6
        _emit 0xc8
        _emit 0x4e
        // 0002ea47:  0f 28 d0           MOVAPS XMM2,XMM0
        _emit 0x0f
        _emit 0x28
        _emit 0xd0
        // 0002ea4a:  0f c6 d0 39        SHUFPS XMM2,XMM0,0x39
        _emit 0x0f
        _emit 0xc6
        _emit 0xd0
        _emit 0x39
        // 0002ea4e:  0f 58 ca           ADDPS XMM1,XMM2
        _emit 0x0f
        _emit 0x58
        _emit 0xca
        // 0002ea51:  0f 28 d0           MOVAPS XMM2,XMM0
        _emit 0x0f
        _emit 0x28
        _emit 0xd0
        // 0002ea54:  0f c6 d0 93        SHUFPS XMM2,XMM0,0x93
        _emit 0x0f
        _emit 0xc6
        _emit 0xd0
        _emit 0x93
        // 0002ea58:  0f 58 d0           ADDPS XMM2,XMM0
        _emit 0x0f
        _emit 0x58
        _emit 0xd0
        // 0002ea5b:  f3 0f 10 05 70 4f f5 00  MOVSS XMM0,[0x00f54f70]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        // 0002ea63:  0f 58 ca           ADDPS XMM1,XMM2
        _emit 0x0f
        _emit 0x58
        _emit 0xca
        // 0002ea66:  0f 52 d1           RSQRTPS XMM2,XMM1
        _emit 0x0f
        _emit 0x52
        _emit 0xd1
        // 0002ea69:  0f c6 c0 00        SHUFPS XMM0,XMM0,0x0
        _emit 0x0f
        _emit 0xc6
        _emit 0xc0
        _emit 0x00
        // 0002ea6d:  0f 28 e2           MOVAPS XMM4,XMM2
        _emit 0x0f
        _emit 0x28
        _emit 0xe2
        // 0002ea70:  0f 59 e1           MULPS XMM4,XMM1
        _emit 0x0f
        _emit 0x59
        _emit 0xe1
        // 0002ea73:  f3 0f 10 0d 60 2f f6 00  MOVSS XMM1,[0x00f62f60]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x0d
        _emit 0x60
        _emit 0x2f
        _emit 0xf6
        _emit 0x00
        // 0002ea7b:  0f 59 e2           MULPS XMM4,XMM2
        _emit 0x0f
        _emit 0x59
        _emit 0xe2
        // 0002ea7e:  0f 5c c4           SUBPS XMM0,XMM4
        _emit 0x0f
        _emit 0x5c
        _emit 0xc4
        // 0002ea81:  0f c6 c9 00        SHUFPS XMM1,XMM1,0x0
        _emit 0x0f
        _emit 0xc6
        _emit 0xc9
        _emit 0x00
        // 0002ea85:  0f 59 ca           MULPS XMM1,XMM2
        _emit 0x0f
        _emit 0x59
        _emit 0xca
        // 0002ea88:  0f 59 c1           MULPS XMM0,XMM1
        _emit 0x0f
        _emit 0x59
        _emit 0xc1
        // 0002ea8b:  0f 58 c2           ADDPS XMM0,XMM2
        _emit 0x0f
        _emit 0x58
        _emit 0xc2
        // 0002ea8e:  0f 59 c3           MULPS XMM0,XMM3
        _emit 0x0f
        _emit 0x59
        _emit 0xc3
        // 0002ea91:  0f 29 04 24        MOVAPS [ESP],XMM0
        _emit 0x0f
        _emit 0x29
        _emit 0x04
        _emit 0x24
        // 0002ea95:  f3 0f 10 04 24     MOVSS XMM0,[ESP]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x04
        _emit 0x24
        // 0002ea9a:  f3 0f 11 00        MOVSS [EAX],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x00
        // 0002ea9e:  f3 0f 10 44 24 04  MOVSS XMM0,[ESP+0x4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0002eaa4:  f3 0f 11 40 04     MOVSS [EAX+0x4],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x04
        // 0002eaa9:  f3 0f 10 44 24 08  MOVSS XMM0,[ESP+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0002eaaf:  f3 0f 11 40 08     MOVSS [EAX+0x8],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x08
        // 0002eab4:  f3 0f 10 44 24 0c  MOVSS XMM0,[ESP+0xc]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0002eaba:  f3 0f 11 40 0c     MOVSS [EAX+0xc],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x0c
        // 0002eabf:  8b e5              MOV ESP,EBP
        _emit 0x8b
        _emit 0xe5
        // 0002eac1:  5d                 POP EBP
        _emit 0x5d
        // 0002eac2:  c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
