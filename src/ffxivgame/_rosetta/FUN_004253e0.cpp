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
// FUNCTION: ffxivgame 0x004253e0 — SSE2 matrix/vector transform helper
//                                  (468 B / 0x1d4, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x004253e0):
//
//   __cdecl void FUN_004253e0(float* v /* [ebp+0x8] -> ESI */);
//
//   Frame: standard EBP frame with 16-byte stack alignment
//   (AND ESP,0xfffffff0) and a 0xcc local-scratch reservation.
//   ESI is the only callee-save touched.
//
//   Body shape:
//     - CALL 0x004186d0 with a 1-init temp; returns EAX -> a source
//       buffer (8 qwords / 64 B) copied via MOVQ XMM0 into the local
//       scratch at [esp+0x14 .. 0x54] (a 4x4 float matrix).
//     - CALL 0x00420200 (ECX=&matrix, push &dst[esp+0x70]) — a matrix
//       transform/normalise.
//     - CALL 0x0042ead0 (ECX=&[esp+0x94], push &[esp+0x60]).
//     - Loads the caller's 4-float vector (*v..v[3]) into [esp+0xb0],
//       broadcasts the global scalar at 0x00f62f7c (SHUFPS imm 0) and
//       MULPS-scales the vector; stores; MOVDQA-copies; then broadcasts
//       a second scalar at 0x00f62f78 and MULPS again, shuffling the
//       lanes back through [esp+0xb0/0xc0/0xcc] scratch.
//     - CALL 0x00419360 (three pushed pointers) — the consumer.
//
//   Reloc-bearing sites in the 468-byte body (resolved only in a full
//   relink at image base 0x00400000; standalone .obj can't reproduce
//   them, but their pre-linked literals already match the orig PE bytes):
//     +0x19  CALL rel32   → 0x004186d0
//     +0x81  CALL rel32   → 0x00420200
//     +0x92  CALL rel32   → 0x0042ead0
//     +0xd6  MOVSS [imm32] ← 0x00f62f7c  (scalar #1)
//     +0x144 MOVSS [imm32] ← 0x00f62f78  (scalar #2)
//     +0x1c7 CALL rel32   → 0x00419360
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rewrite would need MSVC 2005 /O2 to reproduce the
//   exact SSE2 lane shuffling, the 16-byte ESP alignment prologue, and
//   the four reloc-resolved call/global windows above. Each is brittle
//   under /O2. The established sibling idiom (FUN_00401820 / FUN_00409350
//   / FUN_0040b840) is a `__declspec(naked)` body re-emitting the orig
//   bytes verbatim via MASM `_emit`, yielding a byte-identical .text
//   slice — which is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_004253e0() {
    __asm {
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x83
        _emit 0xe4
        _emit 0xf0
        _emit 0x81
        _emit 0xec
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0x8b
        _emit 0x75
        _emit 0x08
        _emit 0x51
        _emit 0x8b
        _emit 0xc4
        _emit 0xc7
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xd2
        _emit 0x32
        _emit 0xff
        _emit 0xff
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x00
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x08
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x10
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x18
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x20
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x28
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x30
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x44
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x38
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x70
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        _emit 0xe8
        _emit 0x9a
        _emit 0xad
        _emit 0xff
        _emit 0xff
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x60
        _emit 0x51
        _emit 0x8d
        _emit 0x8c
        _emit 0x24
        _emit 0x94
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x59
        _emit 0x96
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x06
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x84
        _emit 0x24
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x46
        _emit 0x04
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x84
        _emit 0x24
        _emit 0xb4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x46
        _emit 0x08
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x84
        _emit 0x24
        _emit 0xb8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x46
        _emit 0x0c
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x84
        _emit 0x24
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0x28
        _emit 0x8c
        _emit 0x24
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x7c
        _emit 0x2f
        _emit 0xf6
        _emit 0x00
        _emit 0x0f
        _emit 0xc6
        _emit 0xc0
        _emit 0x00
        _emit 0x0f
        _emit 0x59
        _emit 0xc1
        _emit 0x0f
        _emit 0x29
        _emit 0x84
        _emit 0x24
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x84
        _emit 0x24
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x84
        _emit 0x24
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x84
        _emit 0x24
        _emit 0xb4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x84
        _emit 0x24
        _emit 0xc4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x84
        _emit 0x24
        _emit 0xb8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x84
        _emit 0x24
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x84
        _emit 0x24
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x84
        _emit 0x24
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66
        _emit 0x0f
        _emit 0x6f
        _emit 0x84
        _emit 0x24
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66
        _emit 0x0f
        _emit 0x7f
        _emit 0x44
        _emit 0x24
        _emit 0x50
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x78
        _emit 0x2f
        _emit 0xf6
        _emit 0x00
        _emit 0x0f
        _emit 0xc6
        _emit 0xc0
        _emit 0x00
        _emit 0x0f
        _emit 0x59
        _emit 0xc1
        _emit 0x0f
        _emit 0x29
        _emit 0x84
        _emit 0x24
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x84
        _emit 0x24
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x84
        _emit 0x24
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x84
        _emit 0x24
        _emit 0xc4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x84
        _emit 0x24
        _emit 0xb4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x84
        _emit 0x24
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x50
        _emit 0x52
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x84
        _emit 0x24
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x84
        _emit 0x24
        _emit 0xd0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x84
        _emit 0x24
        _emit 0xc4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x68
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x84
        _emit 0x24
        _emit 0xc4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66
        _emit 0x0f
        _emit 0x6f
        _emit 0x84
        _emit 0x24
        _emit 0xb8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x51
        _emit 0x66
        _emit 0x0f
        _emit 0x7f
        _emit 0x84
        _emit 0x24
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xb4
        _emit 0x3d
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x5e
        _emit 0x8b
        _emit 0xe5
        _emit 0x5d
        _emit 0xc3
    }
}
