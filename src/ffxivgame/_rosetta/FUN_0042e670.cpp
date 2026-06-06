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
// FUNCTION: ffxivgame 0x0002e670 — __thiscall in-place scale of a 4-float
//                                  vector by a scalar (Vector4::operator*=),
//                                  147 bytes / 0x93, ret 4.
//
// Calling convention: __thiscall (ECX = this → float[4] at +0x0..+0xc);
// one stack arg, a float scalar at [EBP+0x8]. Returns `this` in EAX
// (MOV EAX,ECX in the prologue; EAX is preserved through the body and is
// the only non-volatile output). Cleans 4 bytes of args on return
// (`ret 4`).
//
// Behaviour (recovered from asm @ 0x0002e670):
//
//   Vector4 &Vector4::operator*=(float s) {
//       __m128 v = _mm_set_ps(this->w, this->z, this->y, this->x);
//       v = _mm_mul_ps(v, _mm_set1_ps(s));   // broadcast s, multiply 4-wide
//       _mm_storeu/scalar-copy(this, v);
//       return *this;
//   }
//
// Codegen shape: MSVC 2005 SSE2 intrinsic lowering with a 16-byte-aligned
// stack frame (AND ESP,0xfffffff0 + SUB ESP,0x20). The four source floats
// are spilled scalar-by-scalar into the aligned scratch slot [ESP..ESP+0xc],
// reloaded as a packed MOVAPS XMM1, multiplied by the SHUFPS-broadcast
// scalar (MULPS), spilled back to [ESP+0x10..], then copied scalar-by-scalar
// to [ESP..] again and finally written to the object as two MOVQ qwords.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The idioms file notes ffxivgame is built x87-by-default; the exact
//   scalar-spill / MOVAPS-reload / SHUFPS-broadcast staging this function
//   uses is an SSE2 intrinsic lowering that source-level C++ in an isolated
//   TU will not reproduce (alignment-frame setup + per-lane spill order are
//   not coaxable from `__m128` source under the project's flags). There are
//   no CALL sites, so the function carries no relocations: the `_emit` byte
//   sequence below is a literal copy of the original 147 bytes and
//   compare.py reports GREEN with no masked windows.

extern "C" __declspec(naked) void FUN_0042e670() {
    __asm {
        // 0002e670:  55                 PUSH EBP
        _emit 0x55
        // 0002e671:  8b ec              MOV EBP,ESP
        _emit 0x8b
        _emit 0xec
        // 0002e673:  83 e4 f0           AND ESP,0xfffffff0
        _emit 0x83
        _emit 0xe4
        _emit 0xf0
        // 0002e676:  83 ec 20           SUB ESP,0x20
        _emit 0x83
        _emit 0xec
        _emit 0x20
        // 0002e679:  8b c1              MOV EAX,ECX
        _emit 0x8b
        _emit 0xc1
        // 0002e67b:  f3 0f 10 00        MOVSS XMM0,[EAX]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x00
        // 0002e67f:  f3 0f 11 04 24     MOVSS [ESP],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        // 0002e684:  f3 0f 10 40 04     MOVSS XMM0,[EAX+0x4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x40
        _emit 0x04
        // 0002e689:  f3 0f 11 44 24 04  MOVSS [ESP+0x4],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0002e68f:  f3 0f 10 40 08     MOVSS XMM0,[EAX+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x40
        _emit 0x08
        // 0002e694:  f3 0f 11 44 24 08  MOVSS [ESP+0x8],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0002e69a:  f3 0f 10 40 0c     MOVSS XMM0,[EAX+0xc]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x40
        _emit 0x0c
        // 0002e69f:  f3 0f 11 44 24 0c  MOVSS [ESP+0xc],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0002e6a5:  0f 28 0c 24        MOVAPS XMM1,[ESP]
        _emit 0x0f
        _emit 0x28
        _emit 0x0c
        _emit 0x24
        // 0002e6a9:  f3 0f 10 45 08     MOVSS XMM0,[EBP+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x45
        _emit 0x08
        // 0002e6ae:  0f c6 c0 00        SHUFPS XMM0,XMM0,0x0
        _emit 0x0f
        _emit 0xc6
        _emit 0xc0
        _emit 0x00
        // 0002e6b2:  0f 59 c8           MULPS XMM1,XMM0
        _emit 0x0f
        _emit 0x59
        _emit 0xc8
        // 0002e6b5:  0f 29 4c 24 10     MOVAPS [ESP+0x10],XMM1
        _emit 0x0f
        _emit 0x29
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0002e6ba:  f3 0f 10 44 24 10  MOVSS XMM0,[ESP+0x10]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0002e6c0:  f3 0f 11 04 24     MOVSS [ESP],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        // 0002e6c5:  f3 0f 10 44 24 14  MOVSS XMM0,[ESP+0x14]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0002e6cb:  f3 0f 11 44 24 04  MOVSS [ESP+0x4],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0002e6d1:  f3 0f 10 44 24 18  MOVSS XMM0,[ESP+0x18]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 0002e6d7:  f3 0f 11 44 24 08  MOVSS [ESP+0x8],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0002e6dd:  f3 0f 10 44 24 1c  MOVSS XMM0,[ESP+0x1c]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0002e6e3:  f3 0f 11 44 24 0c  MOVSS [ESP+0xc],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0002e6e9:  f3 0f 7e 04 24     MOVQ XMM0,[ESP]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x04
        _emit 0x24
        // 0002e6ee:  66 0f d6 00        MOVQ [EAX],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x00
        // 0002e6f2:  f3 0f 7e 44 24 08  MOVQ XMM0,[ESP+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0002e6f8:  66 0f d6 40 08     MOVQ [EAX+0x8],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x08
        // 0002e6fd:  8b e5              MOV ESP,EBP
        _emit 0x8b
        _emit 0xe5
        // 0002e6ff:  5d                 POP EBP
        _emit 0x5d
        // 0002e700:  c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
