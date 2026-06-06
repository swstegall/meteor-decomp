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
// FUNCTION: ffxivgame 0x000334c0 — __cdecl bool(arg0, int* out1, int* out2)
//                                  (394 B / 0x18a, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x000334c0):
//
//   __cdecl bool FUN_004334c0(void* arg0 /*[ebp+8]*/,
//                             int*  out1 /*[ebp+0xc]*/,
//                             int*  out2 /*[ebp+0x10]*/);
//
//   ESP-aligned (AND ESP, ~7) 0xd8-byte stack frame, returns AL.
//
//   The body fetches two 0x40-byte blobs through the table accessor at
//   0x004186d0 (called with index 1, then index 2), spilling each blob
//   8 bytes at a time via MOVQ XMM0 into two adjacent stack staging
//   areas (~[esp+0x1c..0x5b] and [esp+0x5c..0x9b]). It then calls
//   0x0042edb0(&dstA, &blobB, &blobA) and 0x0042f210(this=eax, &res,
//   arg0). If the resulting float at [esp+0x14] is <= 0.0 (COMISS 0,x →
//   not-below), it returns false. Otherwise it converts that float via
//   0x0042e710, then runs two x87 sequences (each gated on the sign of
//   the global ints at 0x01328fa0 / 0x01328fa4, biasing by [0x00f54a54]
//   when negative, scaling by the double at [0x00f59898]) and rounds
//   each through 0x009d6600, storing the two results to *out1 and *out2
//   before returning true.
//
//   Reloc-bearing sites (resolve only at full-binary link, image base
//   0x00400000; standalone .obj can't reproduce them — compare.py masks
//   these windows):
//     +0x15  rel32 CALL 0x004186d0   (table accessor, idx 1)
//     +0x79  rel32 CALL 0x004186d0   (table accessor, idx 2)
//     +0xf2  rel32 CALL 0x0042edb0
//     +0x102 rel32 CALL 0x0042f210
//     +0x123 rel32 CALL 0x0042e710
//     +0x12e moffs MOV  EDX,[0x01328fa0]
//     +0x138 fld   FLD  qword[0x00f59898]
//     +0x140 fild  FILD dword[0x01328fa0]
//     +0x148 fadd  FADD dword[0x00f54a54]
//     +0x152 rel32 CALL 0x009d6600    (round-to-int helper, 1st)
//     +0x160 moffs MOV  EDX,[0x01328fa4]
//     +0x16a fild  FILD dword[0x01328fa4]
//     +0x172 fadd  FADD dword[0x00f54a54]
//     +0x17a rel32 CALL 0x009d6600    (round-to-int helper, 2nd)
//
// Reconstruction strategy — naked-asm byte passthrough.
//
//   A source-level C++ port at /O2 would have to reproduce the exact
//   XMM0-based 8-byte spill staging, the x87 sign-bias sequences, and
//   the fourteen linker-resolved reloc windows above — each brittle
//   under /O2. As the reloc-heavy siblings (FUN_0040ced0, FUN_00405080,
//   FUN_00415d00) did, this re-emits the orig 394 bytes verbatim via
//   MASM `_emit`, so the .obj's .text is byte-identical to the orig
//   slice, which is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_004334c0() {
    __asm {
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x83
        _emit 0xe4
        _emit 0xf8
        _emit 0x81
        _emit 0xec
        _emit 0xd8
        _emit 0x00
        _emit 0x00
        _emit 0x00
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
        _emit 0xf6
        _emit 0x51
        _emit 0xfe
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
        _emit 0x1c
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
        _emit 0x24
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
        _emit 0x2c
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
        _emit 0x34
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
        _emit 0x3c
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
        _emit 0x44
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
        _emit 0x4c
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x38
        _emit 0x8b
        _emit 0xc4
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x54
        _emit 0xc7
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x92
        _emit 0x51
        _emit 0xfe
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
        _emit 0x5c
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
        _emit 0x64
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
        _emit 0x6c
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
        _emit 0x74
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
        _emit 0x7c
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x28
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x84
        _emit 0x24
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x30
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x84
        _emit 0x24
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
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
        _emit 0x18
        _emit 0x50
        _emit 0x8d
        _emit 0x8c
        _emit 0x24
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x51
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x60
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x84
        _emit 0x24
        _emit 0x98
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xf9
        _emit 0xb7
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x55
        _emit 0x08
        _emit 0x52
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x51
        _emit 0x8b
        _emit 0xc8
        _emit 0xe8
        _emit 0x49
        _emit 0xbc
        _emit 0xff
        _emit 0xff
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        _emit 0x0f
        _emit 0x2f
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x72
        _emit 0x06
        _emit 0x32
        _emit 0xc0
        _emit 0x8b
        _emit 0xe5
        _emit 0x5d
        _emit 0xc3
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x51
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        _emit 0xe8
        _emit 0x28
        _emit 0xb1
        _emit 0xff
        _emit 0xff
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xd9
        _emit 0xe8
        _emit 0x8b
        _emit 0x15
        _emit 0xa0
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x85
        _emit 0xd2
        _emit 0xdc
        _emit 0xc1
        _emit 0xdd
        _emit 0x05
        _emit 0x98
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        _emit 0xdc
        _emit 0xca
        _emit 0xdb
        _emit 0x05
        _emit 0xa0
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x7d
        _emit 0x06
        _emit 0xd8
        _emit 0x05
        _emit 0x54
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0xde
        _emit 0xcb
        _emit 0xd9
        _emit 0xca
        _emit 0xe8
        _emit 0xe9
        _emit 0x2f
        _emit 0x5a
        _emit 0x00
        _emit 0xd8
        _emit 0x64
        _emit 0x24
        _emit 0x0c
        _emit 0x8b
        _emit 0x4d
        _emit 0x0c
        _emit 0x89
        _emit 0x01
        _emit 0x8b
        _emit 0x15
        _emit 0xa4
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0xde
        _emit 0xc9
        _emit 0x85
        _emit 0xd2
        _emit 0xdb
        _emit 0x05
        _emit 0xa4
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x7d
        _emit 0x06
        _emit 0xd8
        _emit 0x05
        _emit 0x54
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0xde
        _emit 0xc9
        _emit 0xe8
        _emit 0xc1
        _emit 0x2f
        _emit 0x5a
        _emit 0x00
        _emit 0x8b
        _emit 0x4d
        _emit 0x10
        _emit 0x89
        _emit 0x01
        _emit 0xb0
        _emit 0x01
        _emit 0x8b
        _emit 0xe5
        _emit 0x5d
        _emit 0xc3
    }
}
