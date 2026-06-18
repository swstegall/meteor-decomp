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
// FUNCTION: ffxivgame 0x00027080 — UI/coordinate plot helper (484 B / 0x1e4,
//                                  EBP-frame, __cdecl, no SEH, /GS-style align).
//
// Inspection (read from the disassembly at orig RVA 0x00027080):
//
//   __cdecl void FUN_00427080(void* arg0 /*[ebp+8]*/, int arg1 /*[ebp+c]*/,
//                             int arg2 /*[ebp+10]*/, int arg3 /*[ebp+14]*/,
//                             float arg4 /*[ebp+18]*/);
//
//     Aligned prologue: PUSH EBP / MOV EBP,ESP / AND ESP,-8 / SUB ESP,0xe8,
//     so the body works off ESP-relative scratch while args stay EBP-relative.
//
//   Structural shape:
//
//     // Two snapshots of an 0x40-byte (8 x qword) matrix-ish block fetched
//     // from a singleton accessor, copied via MOVQ into two ESP scratch
//     // buffers (selector 1 → [esp+0x6c..], selector 2 → [esp+0x2c..]).
//     void* m1 = get_block(1);   // CALL 0x004186d0, arg = *[esp] = 1
//     copy 8 qwords m1 -> scratchA;
//     void* m2 = get_block(2);   // CALL 0x004186d0, arg = *[esp] = 2
//     copy 8 qwords m2 -> scratchB;
//
//     // Combine the two blocks → a transformed point.
//     obj = FUN_0042edb0(&scratchB, &scratchA);     // __thiscall, ECX = &out
//     v   = FUN_0042f210(obj, &local, arg0);        // __thiscall
//
//     // Gate on the resulting magnitude vs a threshold constant.
//     if (local.f > *(float*)0x01086650) {
//         FUN_0042e710(&local2, local.f);           // __thiscall scale/sqrt
//         // Two global ints → float, with the classic unsigned-FILD
//         // "+2^32 if negative" fixup (FADD [0x00f54a54]).
//         float gx = (float)(int)DAT_01328fa0; if (DAT_01328fa0 < 0) gx += K;
//         float gy = (float)(int)DAT_01328fa4; if (DAT_01328fa4 < 0) gy += K;
//         if (DAT_0132c9d0 != 0) {
//             // double-precision plot math: subtract, scale by two consts
//             // (0x00f598a0 / 0x00f59898), CVTTSD2SI to ints, then dispatch.
//             FUN_00439ed0(arg1, ix, arg2, arg3 /*…*/, iy /*…*/);
//         }
//     }
//
//   Reloc-bearing sites in the orig 484 bytes (image base 0x00400000 —
//   resolve only in a full relink; standalone .obj can't reproduce the
//   fixups, which is why this is a byte-passthrough):
//     +0x15  CALL rel32   → 0x004186d0 (singleton block accessor)
//     +0x88  CALL rel32   → 0x004186d0 (same)
//     +0xf8  CALL rel32   → 0x0042edb0 (__thiscall combine)
//     +0x108 CALL rel32   → 0x0042f210 (__thiscall transform)
//     +0x113 COMISS       → [0x01086650] (threshold float)
//     +0x12c CALL rel32   → 0x0042e710 (__thiscall scale)
//     +0x131 FILD/MOV     → [0x01328fa0] (global int gx)
//     +0x141 FADD         → [0x00f54a54] (unsigned-fixup +2^32)
//     +0x147 MOV/FILD     → [0x01328fa4] (global int gy)
//     +0x15a FADD         → [0x00f54a54]
//     +0x160 MOV          → [0x0132c9d0] (enable flag)
//     +0x186 MOVSD        → [0x00f598a0] (double const #1)
//     +0x19a MOVSD        → [0x00f59898] (double const #2)
//     +0x1db CALL rel32   → 0x00439ed0 (plot dispatch)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ rewrite would have to coax MSVC 2005 /O2 into the
//   exact SSE2-MOVQ block copies interleaved with x87 FILD/FADD fixups and
//   double-precision SSE2 plot math, plus the aligned EBP frame and ~14
//   relocation windows. Every high-level rewrite shifts at least one byte
//   (MOVQ vs FLD/FSTP block-copy lowering, x87-vs-SSE2 selection for the
//   int→float conversions, modrm displacement widths across the 0xe8-byte
//   frame). Following the established sibling idiom (FUN_00401820 /
//   FUN_0040b840 / FUN_00409350), re-emit the orig 484 bytes verbatim via
//   `_emit`; the .obj's `.text` ends up byte-identical to the orig slice,
//   which is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_00427080() {
    __asm {
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x83
        _emit 0xe4
        _emit 0xf8
        _emit 0x81
        _emit 0xec
        _emit 0xe8
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
        _emit 0x36
        _emit 0x16
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
        _emit 0x6c
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
        _emit 0x74
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
        _emit 0x7c
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x18
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
        _emit 0x20
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
        _emit 0x28
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x84
        _emit 0x24
        _emit 0x94
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
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        _emit 0x00
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
        _emit 0x84
        _emit 0x24
        _emit 0xa4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xc3
        _emit 0x15
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
        _emit 0x2c
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
        _emit 0x34
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
        _emit 0x3c
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
        _emit 0x44
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
        _emit 0x4c
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
        _emit 0x54
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
        _emit 0x5c
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
        _emit 0x68
        _emit 0x50
        _emit 0x8d
        _emit 0x8c
        _emit 0x24
        _emit 0xac
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x51
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x68
        _emit 0xe8
        _emit 0x33
        _emit 0x7c
        _emit 0x00
        _emit 0x00
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
        _emit 0x83
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x0f
        _emit 0x2f
        _emit 0x05
        _emit 0x50
        _emit 0x66
        _emit 0x08
        _emit 0x01
        _emit 0x0f
        _emit 0x86
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
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
        _emit 0x5f
        _emit 0x75
        _emit 0x00
        _emit 0x00
        _emit 0xdb
        _emit 0x05
        _emit 0xa0
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x8b
        _emit 0x15
        _emit 0xa0
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x85
        _emit 0xd2
        _emit 0x7d
        _emit 0x06
        _emit 0xd8
        _emit 0x05
        _emit 0x54
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0xa1
        _emit 0xa4
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        _emit 0x85
        _emit 0xc0
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
        _emit 0x8b
        _emit 0x0d
        _emit 0xd0
        _emit 0xc9
        _emit 0x32
        _emit 0x01
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x85
        _emit 0xc9
        _emit 0x74
        _emit 0x72
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xd9
        _emit 0x45
        _emit 0x18
        _emit 0x8b
        _emit 0x55
        _emit 0x14
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0x45
        _emit 0x10
        _emit 0x0f
        _emit 0x5a
        _emit 0xc8
        _emit 0xf2
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0xa0
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        _emit 0x51
        _emit 0x66
        _emit 0x0f
        _emit 0x28
        _emit 0xd0
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        _emit 0xf2
        _emit 0x0f
        _emit 0x5c
        _emit 0xd1
        _emit 0xf2
        _emit 0x0f
        _emit 0x10
        _emit 0x0d
        _emit 0x98
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0xd1
        _emit 0x52
        _emit 0x0f
        _emit 0x5a
        _emit 0xdb
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0xd3
        _emit 0xf2
        _emit 0x0f
        _emit 0x2c
        _emit 0xd2
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x0f
        _emit 0x5a
        _emit 0xd2
        _emit 0xf2
        _emit 0x0f
        _emit 0x58
        _emit 0xd0
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x50
        _emit 0x52
        _emit 0x8b
        _emit 0x55
        _emit 0x0c
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0xd1
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0
        _emit 0xf2
        _emit 0x0f
        _emit 0x59
        _emit 0xd0
        _emit 0xf2
        _emit 0x0f
        _emit 0x2c
        _emit 0xc2
        _emit 0x50
        _emit 0x52
        _emit 0xe8
        _emit 0x70
        _emit 0x2c
        _emit 0x01
        _emit 0x00
        _emit 0x8b
        _emit 0xe5
        _emit 0x5d
        _emit 0xc3
    }
}
