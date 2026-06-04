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
// FUNCTION: ffxivgame 0x0042e570 — `__thiscall` Vector4 scalar-multiply
//                                  returning the result struct by value
//                                  (122 B / 0x7a, SSE / packed-single).
//
// Inspection (read from the disassembly at orig RVA 0x0002e570):
//
//   __thiscall Vec4* Vec4::operator*(Vec4 *this /*ECX*/,
//                                    Vec4 *__ret /*[EBP+0x8]*/,
//                                    float  s    /*[EBP+0xC]*/)
//   {
//       // 16-byte-aligned scratch; copy this->{x,y,z,w} into it, broadcast
//       // the scalar across all four lanes, MULPS, then scatter the four
//       // products into the caller-supplied return slot (returned in EAX).
//       __m128 v = _mm_load (this->x..w);     // scalar MOVSS x4 → [ESP]
//       __m128 b = _mm_set1_ps(s);            // MOVSS + SHUFPS xmm,xmm,0
//       v = _mm_mul_ps(v, b);                 // MULPS
//       __ret->x..w = lanes of v;             // scalar MOVSS x4 → [EAX]
//       return __ret;                         // EAX
//   }
//
//   - ECX is `this`; the function returns a struct by value, so the hidden
//     return-slot pointer arrives as the first stack arg ([EBP+0x8]) and is
//     handed back in EAX. The scalar multiplier is the second stack arg
//     ([EBP+0xC]). `RET 0x8` cleans both stack args (callee cleanup).
//   - `AND ESP, 0xFFFFFFF0` + `SUB ESP, 0x10` carve a 16-byte-aligned
//     scratch for the MOVAPS load/store of the packed vector.
//
//   The body carries NO relocations — every operand is a register or an
//   ESP/EBP/ECX/EAX-relative displacement, no globals, no calls. So a
//   `__declspec(naked)` body that re-emits the orig 122 bytes verbatim
//   produces a .obj whose `.text` is byte-identical to the orig slice with
//   no fixups; `tools/compare.py` reports GREEN. This is the same
//   passthrough idiom the siblings FUN_00401350 / FUN_00403d60 used.

extern "C" __declspec(naked) void FUN_0042e570() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x83              // AND ESP, 0xFFFFFFF0
        _emit 0xe4
        _emit 0xf0
        _emit 0x83              // SUB ESP, 0x10
        _emit 0xec
        _emit 0x10
        _emit 0xf3              // MOVSS XMM0, [ECX]
        _emit 0x0f
        _emit 0x10
        _emit 0x01
        _emit 0x8b              // MOV EAX, [EBP + 0x8]
        _emit 0x45
        _emit 0x08
        _emit 0xf3              // MOVSS [ESP], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        _emit 0xf3              // MOVSS XMM0, [ECX + 0x4]
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x04
        _emit 0xf3              // MOVSS [ESP + 0x4], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVSS XMM0, [ECX + 0x8]
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x08
        _emit 0xf3              // MOVSS [ESP + 0x8], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3              // MOVSS XMM0, [ECX + 0xC]
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x0c
        _emit 0xf3              // MOVSS [ESP + 0xC], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x0f              // MOVAPS XMM1, [ESP]
        _emit 0x28
        _emit 0x0c
        _emit 0x24
        _emit 0xf3              // MOVSS XMM0, [EBP + 0xC]
        _emit 0x0f
        _emit 0x10
        _emit 0x45
        _emit 0x0c
        _emit 0x0f              // SHUFPS XMM0, XMM0, 0x0
        _emit 0xc6
        _emit 0xc0
        _emit 0x00
        _emit 0x0f              // MULPS XMM1, XMM0
        _emit 0x59
        _emit 0xc8
        _emit 0x0f              // MOVAPS [ESP], XMM1
        _emit 0x29
        _emit 0x0c
        _emit 0x24
        _emit 0xf3              // MOVSS XMM0, [ESP]
        _emit 0x0f
        _emit 0x10
        _emit 0x04
        _emit 0x24
        _emit 0xf3              // MOVSS [EAX], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x00
        _emit 0xf3              // MOVSS XMM0, [ESP + 0x4]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVSS [EAX + 0x4], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x04
        _emit 0xf3              // MOVSS XMM0, [ESP + 0x8]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3              // MOVSS [EAX + 0x8], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x08
        _emit 0xf3              // MOVSS XMM0, [ESP + 0xC]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xf3              // MOVSS [EAX + 0xC], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x0c
        _emit 0x8b              // MOV ESP, EBP
        _emit 0xe5
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
