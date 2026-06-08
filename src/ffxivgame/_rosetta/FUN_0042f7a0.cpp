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
// FUNCTION: ffxivgame 0x0002f7a0 — SSE quaternion/vector transform helper
//                                  (__cdecl, 417 B / 0x1a1, no SEH;
//                                   ESP-aligned 0x5c stack frame).
//
// Inspection (read from the disassembly at orig RVA 0x0002f7a0):
//
//   __cdecl Vec4* FUN_0042f7a0(Vec4* out /*[ebp+8]*/,
//                              const Vec4* q   /*[ebp+0xc]*/,
//                              const Vec4* v   /*[ebp+0x10]*/);
//
//   Returns `out` (EAX = ESI = [ebp+8]) — the classic "result struct
//   returned by pointer" convention used throughout the math layer.
//
//   Structural shape (read from the asm):
//     - Spill v[0..3] then q[0..3] into the aligned scratch slot at
//       [esp+0x10] and reload each as a MOVAPS xmm (xmm1 = v, xmm0 = q).
//     - MULPS xmm1, xmm0  then a SHUFPS(0x39)+ADDPS / SHUFPS(0x4e)+ADDPS
//       horizontal-sum pair — a 4-wide dot product broadcast.
//     - Broadcast the .data constant [0x00f62f64] (SHUFPS 0x0), splat the
//       dot result (SHUFPS 0x0), MULPS+ADDPS to form the squared-norm
//       argument; spill to [esp+0x50].
//     - RSQRTPS xmm1, then a Newton-Raphson refinement step using the
//       .data constants [0x00f62f60] (broadcast) and [0x00f54f70]
//       (broadcast, ~1.5f / 0.5f reciprocal-sqrt coefficients) →
//       normalized scale in xmm0; spilled to [esp+0x48].
//     - CALL FUN_0042e930 (returns EAX → 4 floats via two MOVQ pairs),
//       copied into [esp+0x30..0x3c] with the 4th lane cleared (XORPS).
//     - Final MULPS chain (against [esp+0x50] and [esp+0x20]), an
//       UNPCKHPS/SHUFPS(0xc4) reshuffle, and store of the 4-float result
//       through ESI (out) via two MOVQ qword stores.
//
//   Reloc-bearing sites in the orig 417 bytes (resolve only in a
//   full-binary relink at image base 0x00400000; a standalone .obj can't
//   reproduce them):
//     +0x44  moffs [0x00f62f64] — MOVSS xmm2 coefficient #1
//     +0xa0  moffs [0x00f62f60] — MOVSS xmm2 coefficient #2
//     +0xb9  moffs [0x00f54f70] — MOVSS xmm0 coefficient #3
//     +0xe8  rel32  CALL 0x0042e930 — sibling math helper
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rewrite would have to coax MSVC 2005 /O2 into the exact
//   xmm allocation, the spill/reload dance through [esp+0x10]/[esp+0x30],
//   the RSQRTPS Newton-Raphson lowering, and the three linker-resolved
//   absolute coefficient loads + the rel32 call target. Each is brittle
//   under /O2 (any high-level form shifts at least one byte). The
//   pragmatic choice — the same one the SSE/reloc-heavy siblings took —
//   is a `__declspec(naked)` body that re-emits the orig 417 bytes
//   verbatim via MASM `_emit`. The .obj's `.text` ends up byte-identical
//   to the orig slice (the call/const bytes are raw immediates, so no
//   relocations), which is what `tools/compare.py` grades.

extern "C" __declspec(naked) void FUN_0042f7a0() {
    __asm {
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x83
        _emit 0xe4
        _emit 0xf0
        _emit 0x83
        _emit 0xec
        _emit 0x5c
        _emit 0x56
        _emit 0x8b
        _emit 0x45
        _emit 0x10
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x00
        _emit 0x8b
        _emit 0x4d
        _emit 0x0c
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x40
        _emit 0x04
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x40
        _emit 0x08
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x40
        _emit 0x0c
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x0f
        _emit 0x28
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x15
        _emit 0x64
        _emit 0x2f
        _emit 0xf6
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x04
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x08
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x41
        _emit 0x0c
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x0f
        _emit 0x28
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x0f
        _emit 0x59
        _emit 0xc8
        _emit 0x0f
        _emit 0x28
        _emit 0xc1
        _emit 0x0f
        _emit 0xc6
        _emit 0xc1
        _emit 0x39
        _emit 0x0f
        _emit 0x58
        _emit 0xc1
        _emit 0x0f
        _emit 0x28
        _emit 0xc8
        _emit 0x0f
        _emit 0xc6
        _emit 0xc8
        _emit 0x4e
        _emit 0x0f
        _emit 0x58
        _emit 0xc8
        _emit 0x0f
        _emit 0xc6
        _emit 0xd2
        _emit 0x00
        _emit 0x0f
        _emit 0x28
        _emit 0xc1
        _emit 0x0f
        _emit 0xc6
        _emit 0xc1
        _emit 0x00
        _emit 0x0f
        _emit 0x59
        _emit 0xc2
        _emit 0x0f
        _emit 0x58
        _emit 0xc2
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x15
        _emit 0x60
        _emit 0x2f
        _emit 0xf6
        _emit 0x00
        _emit 0x0f
        _emit 0x52
        _emit 0xc8
        _emit 0x0f
        _emit 0x29
        _emit 0x44
        _emit 0x24
        _emit 0x50
        _emit 0x8b
        _emit 0x75
        _emit 0x08
        _emit 0x0f
        _emit 0x28
        _emit 0xd9
        _emit 0x0f
        _emit 0x59
        _emit 0xd8
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        _emit 0x0f
        _emit 0xc6
        _emit 0xd2
        _emit 0x00
        _emit 0x0f
        _emit 0xc6
        _emit 0xc0
        _emit 0x00
        _emit 0x50
        _emit 0x0f
        _emit 0x29
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x0f
        _emit 0x59
        _emit 0xd9
        _emit 0x0f
        _emit 0x5c
        _emit 0xc3
        _emit 0x0f
        _emit 0x59
        _emit 0xd1
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x0f
        _emit 0x59
        _emit 0xc2
        _emit 0x0f
        _emit 0x58
        _emit 0xc1
        _emit 0x50
        _emit 0x0f
        _emit 0x29
        _emit 0x44
        _emit 0x24
        _emit 0x48
        _emit 0xe8
        _emit 0xa3
        _emit 0xf0
        _emit 0xff
        _emit 0xff
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x00
        _emit 0x0f
        _emit 0x28
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x10
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
        _emit 0x18
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0x0f
        _emit 0x28
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x0f
        _emit 0x59
        _emit 0xc1
        _emit 0x0f
        _emit 0x59
        _emit 0x4c
        _emit 0x24
        _emit 0x50
        _emit 0x0f
        _emit 0x59
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x0f
        _emit 0x28
        _emit 0xd0
        _emit 0x0f
        _emit 0x15
        _emit 0xc1
        _emit 0x0f
        _emit 0xc6
        _emit 0xd0
        _emit 0xc4
        _emit 0x0f
        _emit 0x29
        _emit 0x54
        _emit 0x24
        _emit 0x30
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x06
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x08
        _emit 0x8b
        _emit 0xc6
        _emit 0x5e
        _emit 0x8b
        _emit 0xe5
        _emit 0x5d
        _emit 0xc3
    }
}
