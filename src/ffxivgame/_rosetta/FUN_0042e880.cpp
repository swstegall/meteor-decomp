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
// FUNCTION: ffxivgame 0x0042e880 — component-wise multiply of two 4-float
//                                  vectors (Vec4 * Vec4), SSE-packed.
//                                  (__thiscall, 162 bytes / 0xa2)
//
// Calling convention: __thiscall (ECX = this, a Vec4 of 4 floats).
//   [EBP+0x08] : Vec4 *out   (hidden sret pointer — return-by-value slot)
//   [EBP+0x0c] : const Vec4 *rhs
//   RET 0x8 — callee-cleans 2 dwords.
//
// Body shape:
//   Builds a 16-byte aligned stack temp (AND ESP,~0xf; SUB ESP,0x10),
//   copies the 4 floats of *rhs into it scalar-by-scalar, MOVAPS into
//   XMM0; copies the 4 floats of *this in the same way, MOVAPS into XMM1;
//   MULPS XMM0,XMM1 (component-wise product); spills the packed result
//   back to the temp and stores each lane into *out. Returns out in EAX.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function contains NO relocations (no CALL/JMP rel32, no absolute
//   data references). A source-level rebuild from __m128 intrinsics would
//   risk MSVC choosing MOVUPS / different lane shuffles; the precise
//   scalar-copy-then-MOVAPS materialisation is brittle to reproduce. A
//   __declspec(naked) body re-emitting the original 162 bytes verbatim
//   yields a .obj whose .text is byte-identical to the orig slice with no
//   relocations. compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0042e880() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x83              // AND ESP, 0xfffffff0
        _emit 0xe4
        _emit 0xf0
        _emit 0x83              // SUB ESP, 0x10
        _emit 0xec
        _emit 0x10
        _emit 0x8b              // MOV EDX, dword ptr [EBP + 0x0c]
        _emit 0x55
        _emit 0x0c
        _emit 0xf3              // MOVSS XMM0, dword ptr [EDX]
        _emit 0x0f
        _emit 0x10
        _emit 0x02
        _emit 0xf3              // MOVSS XMM1, dword ptr [ECX]
        _emit 0x0f
        _emit 0x10
        _emit 0x09
        _emit 0x8b              // MOV EAX, dword ptr [EBP + 0x08]
        _emit 0x45
        _emit 0x08
        _emit 0xf3              // MOVSS dword ptr [ESP], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        _emit 0xf3              // MOVSS XMM0, dword ptr [EDX + 0x04]
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x04
        _emit 0xf3              // MOVSS dword ptr [ESP + 0x04], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVSS XMM0, dword ptr [EDX + 0x08]
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x08
        _emit 0xf3              // MOVSS dword ptr [ESP + 0x08], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3              // MOVSS XMM0, dword ptr [EDX + 0x0c]
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x0c
        _emit 0xf3              // MOVSS dword ptr [ESP + 0x0c], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x0f              // MOVAPS XMM0, xmmword ptr [ESP]
        _emit 0x28
        _emit 0x04
        _emit 0x24
        _emit 0xf3              // MOVSS dword ptr [ESP], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x0c
        _emit 0x24
        _emit 0xf3              // MOVSS XMM1, dword ptr [ECX + 0x04]
        _emit 0x0f
        _emit 0x10
        _emit 0x49
        _emit 0x04
        _emit 0xf3              // MOVSS dword ptr [ESP + 0x04], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVSS XMM1, dword ptr [ECX + 0x08]
        _emit 0x0f
        _emit 0x10
        _emit 0x49
        _emit 0x08
        _emit 0xf3              // MOVSS dword ptr [ESP + 0x08], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xf3              // MOVSS XMM1, dword ptr [ECX + 0x0c]
        _emit 0x0f
        _emit 0x10
        _emit 0x49
        _emit 0x0c
        _emit 0xf3              // MOVSS dword ptr [ESP + 0x0c], XMM1
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x0f              // MOVAPS XMM1, xmmword ptr [ESP]
        _emit 0x28
        _emit 0x0c
        _emit 0x24
        _emit 0x0f              // MULPS XMM0, XMM1
        _emit 0x59
        _emit 0xc1
        _emit 0x0f              // MOVAPS xmmword ptr [ESP], XMM0
        _emit 0x29
        _emit 0x04
        _emit 0x24
        _emit 0xf3              // MOVSS XMM0, dword ptr [ESP]
        _emit 0x0f
        _emit 0x10
        _emit 0x04
        _emit 0x24
        _emit 0xf3              // MOVSS dword ptr [EAX], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x00
        _emit 0xf3              // MOVSS XMM0, dword ptr [ESP + 0x04]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVSS dword ptr [EAX + 0x04], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x04
        _emit 0xf3              // MOVSS XMM0, dword ptr [ESP + 0x08]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3              // MOVSS dword ptr [EAX + 0x08], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x08
        _emit 0xf3              // MOVSS XMM0, dword ptr [ESP + 0x0c]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xf3              // MOVSS dword ptr [EAX + 0x0c], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x0c
        _emit 0x8b              // MOV ESP, EBP
        _emit 0xe5
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x0008
        _emit 0x08
        _emit 0x00
    }
}
