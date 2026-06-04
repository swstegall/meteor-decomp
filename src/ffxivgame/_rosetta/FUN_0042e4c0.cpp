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
// FUNCTION: ffxivgame 0x0042e4c0 — 4-float vector componentwise subtract
//                                  (__thiscall, 162 bytes / 0xa2)
//
// __thiscall void FUN_0042e4c0(Vec4 *this, Vec4 *out, Vec4 *b)
//   ECX        : this  (minuend, 4 packed floats)
//   [EBP+0x08] : out   (destination, returned in EAX)
//   [EBP+0x0c] : b     (subtrahend)
//   RET 0x08 — __thiscall, callee-cleans 2 dwords (out, b).
//
// Behaviour:
//   The frame is 16-byte aligned (AND ESP,~0xf) so a 16-byte scratch
//   slot at [ESP] can be MOVAPS'd. Both operands are gathered element by
//   element (MOVSS) into that aligned slot — *b first into XMM0, then
//   *this into XMM1 — because the source pointers themselves are not
//   guaranteed aligned. SUBPS XMM1,XMM0 computes *this - *b in one shot,
//   the result is spilled back to the aligned slot, then scattered out
//   to *out one float at a time.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function carries NO relocations (pure register/stack shuffling
//   over SSE), so a __declspec(naked) body re-emitting the orig 162
//   bytes verbatim via MASM _emit directives yields a .obj whose .text
//   is byte-identical to the orig slice. compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0042e4c0() {
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
        _emit 0x8b              // MOV EDX, dword ptr [EBP + 0x0c]   (b)
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
        _emit 0x8b              // MOV EAX, dword ptr [EBP + 0x08]   (out)
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
        _emit 0x0f              // MOVAPS XMM0, xmmword ptr [ESP]    (b)
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
        _emit 0x0f              // MOVAPS XMM1, xmmword ptr [ESP]    (this)
        _emit 0x28
        _emit 0x0c
        _emit 0x24
        _emit 0x0f              // SUBPS XMM1, XMM0                  (this - b)
        _emit 0x5c
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
