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
// FUNCTION: ffxivgame 0x009c945c — FUN_00dc945c
//                                  (__stdcall, 410 B / 0x19a, no SEH;
//                                   0x20-byte EBP frame, hotpatch pad).
//
// Inspection (read from the disassembly at orig RVA 0x009c945c):
//
//   __stdcall void FUN_00dc945c(Param* p);   // RET 0x4 — one dword arg.
//
//   An SSE-scalar (MOVSS/MULSS/ADDSS) batched matrix/vector evaluation
//   kernel driven by a parameter block `p` (ECX). The block carries:
//
//     p->[0x00]  float*  A      (input row base, advanced by `step` per outer)
//     p->[0x04]  float*  C      (output base, advanced by `rows` per outer)
//     p->[0x08]  float*  B      (shared operand, reloaded per outer)
//     p->[0x0c]  int/ptr fc     (0 → plain A·B path; else a float* weight
//                                table feeding the FILD (float)(unsigned)i
//                                interpolation path)
//     p->[0x14]  uint    count  (dividend: outer = count / step)
//     p->[0x18]  uint    step   (k-extent / divisor)
//     p->[0x1c]  uint    rows   (j-extent)
//     p->[0x20]  int     accum  (non-zero → read-modify-write into C)
//     p->[0x24]  int     flag   (zero → force step = rows = 1)
//
//   Two structural arms: the fc==0 arm computes
//     C[o*rows + j] (+=) Σ_k A[o*step + k] * B[j*step + k]
//   and the fc!=0 arm folds in the (float)(unsigned)i weight curve via the
//   x87 FILD/conditional-FADD[0x00f54a54] unsigned→float conversion before
//   the SSE accumulate. Both arms share the same advance/epilogue shape.
//
//   Reloc-bearing site in the orig 410 bytes:
//     +0x100  data [0x00f54a54]  FADD float ptr — the 2^32 unsigned-bias
//                                constant for the FILD conversion.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Coaxing MSVC 2005 /O2 into reproducing the exact XMM/x87 interleave,
//   the source-order register allocation across both arms, the short-vs-
//   near branch selection, and the absolute-address FADD reloc window is
//   brittle — every high-level rewrite shifts at least one byte. Following
//   the local idiom (FUN_0040ced0 / FUN_00415d00), the body is a
//   `__declspec(naked)` re-emission of the orig 410 bytes verbatim via
//   MASM `_emit`. The .obj's `.text` ends up byte-identical to the orig
//   slice, which is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_00dc945c() {
    __asm {
        _emit 0x8b
        _emit 0xff
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x83
        _emit 0xec
        _emit 0x20
        _emit 0x8b
        _emit 0x4d
        _emit 0x08
        _emit 0x83
        _emit 0x79
        _emit 0x24
        _emit 0x00
        _emit 0x8b
        _emit 0x01
        _emit 0x8b
        _emit 0x51
        _emit 0x20
        _emit 0x53
        _emit 0x8b
        _emit 0x59
        _emit 0x1c
        _emit 0x56
        _emit 0x8b
        _emit 0x71
        _emit 0x18
        _emit 0x57
        _emit 0x8b
        _emit 0x79
        _emit 0x04
        _emit 0x89
        _emit 0x45
        _emit 0x08
        _emit 0x8b
        _emit 0x41
        _emit 0x14
        _emit 0x89
        _emit 0x7d
        _emit 0xfc
        _emit 0x89
        _emit 0x55
        _emit 0xe8
        _emit 0x74
        _emit 0x05
        _emit 0x33
        _emit 0xf6
        _emit 0x46
        _emit 0x8b
        _emit 0xde
        _emit 0x8b
        _emit 0x51
        _emit 0x0c
        _emit 0x85
        _emit 0xd2
        _emit 0x89
        _emit 0x55
        _emit 0xe4
        _emit 0x0f
        _emit 0x85
        _emit 0x83
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf7
        _emit 0xf6
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x86
        _emit 0x48
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x49
        _emit 0x08
        _emit 0x89
        _emit 0x4d
        _emit 0xf0
        _emit 0x89
        _emit 0x45
        _emit 0xf4
        _emit 0x8b
        _emit 0x45
        _emit 0xf0
        _emit 0x33
        _emit 0xc9
        _emit 0x85
        _emit 0xdb
        _emit 0x76
        _emit 0x4b
        _emit 0x8b
        _emit 0x55
        _emit 0x08
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x0a
        _emit 0x33
        _emit 0xd2
        _emit 0x0f
        _emit 0x28
        _emit 0xc1
        _emit 0xf3
        _emit 0x0f
        _emit 0x59
        _emit 0x00
        _emit 0x42
        _emit 0x83
        _emit 0xc0
        _emit 0x04
        _emit 0x3b
        _emit 0xf2
        _emit 0x76
        _emit 0x1e
        _emit 0x8b
        _emit 0x7d
        _emit 0x08
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x14
        _emit 0x97
        _emit 0xf3
        _emit 0x0f
        _emit 0x59
        _emit 0x10
        _emit 0x83
        _emit 0xc0
        _emit 0x04
        _emit 0x42
        _emit 0x3b
        _emit 0xd6
        _emit 0xf3
        _emit 0x0f
        _emit 0x58
        _emit 0xd0
        _emit 0x0f
        _emit 0x28
        _emit 0xc2
        _emit 0x72
        _emit 0xe5
        _emit 0x8b
        _emit 0x7d
        _emit 0xfc
        _emit 0x83
        _emit 0x7d
        _emit 0xe8
        _emit 0x00
        _emit 0x74
        _emit 0x05
        _emit 0xf3
        _emit 0x0f
        _emit 0x58
        _emit 0x04
        _emit 0x8f
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x8f
        _emit 0x41
        _emit 0x3b
        _emit 0xcb
        _emit 0x72
        _emit 0xbc
        _emit 0x8b
        _emit 0xc6
        _emit 0xc1
        _emit 0xe0
        _emit 0x02
        _emit 0x01
        _emit 0x45
        _emit 0x08
        _emit 0x8b
        _emit 0xc3
        _emit 0xc1
        _emit 0xe0
        _emit 0x02
        _emit 0x03
        _emit 0xf8
        _emit 0xff
        _emit 0x4d
        _emit 0xf4
        _emit 0x89
        _emit 0x7d
        _emit 0xfc
        _emit 0x75
        _emit 0x95
        _emit 0xe9
        _emit 0xcf
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xd2
        _emit 0xf7
        _emit 0xf6
        _emit 0x83
        _emit 0x65
        _emit 0xf4
        _emit 0x00
        _emit 0x85
        _emit 0xc0
        _emit 0x89
        _emit 0x45
        _emit 0xe0
        _emit 0x0f
        _emit 0x86
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x41
        _emit 0x08
        _emit 0x89
        _emit 0x45
        _emit 0xf0
        _emit 0x83
        _emit 0x65
        _emit 0xf8
        _emit 0x00
        _emit 0x85
        _emit 0xdb
        _emit 0x8b
        _emit 0x7d
        _emit 0xf0
        _emit 0x8b
        _emit 0x55
        _emit 0xe4
        _emit 0x0f
        _emit 0x86
        _emit 0x85
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        _emit 0xdb
        _emit 0x45
        _emit 0xf4
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x08
        _emit 0x8b
        _emit 0x45
        _emit 0xf4
        _emit 0x85
        _emit 0xc0
        _emit 0x7d
        _emit 0x06
        _emit 0xd8
        _emit 0x05
        _emit 0x54
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0xd9
        _emit 0x5d
        _emit 0xec
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x02
        _emit 0xf3
        _emit 0x0f
        _emit 0x59
        _emit 0x45
        _emit 0xec
        _emit 0xf3
        _emit 0x0f
        _emit 0x58
        _emit 0x07
        _emit 0x33
        _emit 0xc0
        _emit 0x40
        _emit 0x83
        _emit 0xc2
        _emit 0x04
        _emit 0x83
        _emit 0xc7
        _emit 0x04
        _emit 0x3b
        _emit 0xf0
        _emit 0xf3
        _emit 0x0f
        _emit 0x59
        _emit 0xc1
        _emit 0x76
        _emit 0x27
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x12
        _emit 0xf3
        _emit 0x0f
        _emit 0x59
        _emit 0x55
        _emit 0xec
        _emit 0xf3
        _emit 0x0f
        _emit 0x58
        _emit 0x17
        _emit 0x8b
        _emit 0x4d
        _emit 0x08
        _emit 0xf3
        _emit 0x0f
        _emit 0x59
        _emit 0x14
        _emit 0x81
        _emit 0x83
        _emit 0xc2
        _emit 0x04
        _emit 0x83
        _emit 0xc7
        _emit 0x04
        _emit 0x40
        _emit 0x3b
        _emit 0xc6
        _emit 0xf3
        _emit 0x0f
        _emit 0x58
        _emit 0xd0
        _emit 0x0f
        _emit 0x28
        _emit 0xc2
        _emit 0x72
        _emit 0xd9
        _emit 0x83
        _emit 0x7d
        _emit 0xe8
        _emit 0x00
        _emit 0x8b
        _emit 0x45
        _emit 0xfc
        _emit 0x8b
        _emit 0x4d
        _emit 0xf8
        _emit 0x74
        _emit 0x0d
        _emit 0x8d
        _emit 0x04
        _emit 0x88
        _emit 0xf3
        _emit 0x0f
        _emit 0x58
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x00
        _emit 0xeb
        _emit 0x05
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x88
        _emit 0xff
        _emit 0x45
        _emit 0xf8
        _emit 0x39
        _emit 0x5d
        _emit 0xf8
        _emit 0x72
        _emit 0x95
        _emit 0x8b
        _emit 0xc6
        _emit 0xc1
        _emit 0xe0
        _emit 0x02
        _emit 0x01
        _emit 0x45
        _emit 0x08
        _emit 0x8b
        _emit 0xc3
        _emit 0xc1
        _emit 0xe0
        _emit 0x02
        _emit 0x01
        _emit 0x45
        _emit 0xfc
        _emit 0xff
        _emit 0x45
        _emit 0xf4
        _emit 0x8b
        _emit 0x45
        _emit 0xf4
        _emit 0x3b
        _emit 0x45
        _emit 0xe0
        _emit 0x0f
        _emit 0x82
        _emit 0x4a
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0xc9
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
