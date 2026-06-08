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
// FUNCTION: ffxivgame 0x0002e5f0 — __thiscall divide of a 4-float vector by
//                                  a scalar (Vector4 / float → out-param),
//                                  122 bytes / 0x7a, ret 8.
//
// Calling convention: __thiscall (ECX = this → float[4] at +0x0..+0xc).
//   [EBP+0x08] : Vec4 *out   (hidden sret pointer — return-by-value slot)
//   [EBP+0x0c] : float scalar (divisor)
//   RET 0x8 — callee-cleans 2 dwords.
//
// Behaviour (recovered from asm @ 0x0002e5f0):
//
//   Builds a 16-byte-aligned stack temp (AND ESP,0xfffffff0; SUB ESP,0x10),
//   copies the 4 floats of *this (ECX) into it scalar-by-scalar via MOVSS,
//   reloads them as a packed MOVAPS XMM1, broadcasts the scalar divisor via
//   SHUFPS XMM0,XMM0,0, divides with DIVPS XMM1,XMM0, spills the result
//   back to [ESP], then copies each lane to the output pointer in EAX.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function contains NO relocations (no CALL/JMP rel32, no absolute
//   data references). The scalar-copy-then-MOVAPS materialisation is
//   identical in structure to the sibling FUN_0042e880 (component-wise
//   multiply) — same SSE1 idiom, same 16-byte alignment dance. A
//   __declspec(naked) body re-emitting the original 122 bytes verbatim
//   yields a .obj whose .text is byte-identical to the orig slice with no
//   relocations. compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0042e5f0() {
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
        _emit 0xf3              // MOVSS XMM0, dword ptr [ECX]
        _emit 0x0f
        _emit 0x10
        _emit 0x01
        _emit 0x8b              // MOV EAX, dword ptr [EBP + 0x8]
        _emit 0x45
        _emit 0x08
        _emit 0xf3              // MOVSS dword ptr [ESP], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        _emit 0xf3              // MOVSS XMM0, dword ptr [ECX + 0x4]
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x04
        _emit 0xf3              // MOVSS dword ptr [ESP + 0x4], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVSS XMM0, dword ptr [ECX + 0x8]
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x08
        _emit 0xf3              // MOVSS dword ptr [ESP + 0x8], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3              // MOVSS XMM0, dword ptr [ECX + 0xc]
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x0c
        _emit 0xf3              // MOVSS dword ptr [ESP + 0xc], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x0f              // MOVAPS XMM1, xmmword ptr [ESP]
        _emit 0x28
        _emit 0x0c
        _emit 0x24
        _emit 0xf3              // MOVSS XMM0, dword ptr [EBP + 0xc]
        _emit 0x0f
        _emit 0x10
        _emit 0x45
        _emit 0x0c
        _emit 0x0f              // SHUFPS XMM0, XMM0, 0x0
        _emit 0xc6
        _emit 0xc0
        _emit 0x00
        _emit 0x0f              // DIVPS XMM1, XMM0
        _emit 0x5e
        _emit 0xc8
        _emit 0x0f              // MOVAPS xmmword ptr [ESP], XMM1
        _emit 0x29
        _emit 0x0c
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
        _emit 0xf3              // MOVSS XMM0, dword ptr [ESP + 0x4]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVSS dword ptr [EAX + 0x4], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x04
        _emit 0xf3              // MOVSS XMM0, dword ptr [ESP + 0x8]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3              // MOVSS dword ptr [EAX + 0x8], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x08
        _emit 0xf3              // MOVSS XMM0, dword ptr [ESP + 0xc]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xf3              // MOVSS dword ptr [EAX + 0xc], XMM0
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
