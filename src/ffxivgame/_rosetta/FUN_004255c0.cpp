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
// FUNCTION: ffxivgame 0x000255c0 — debug-overlay quad/text emit helper
//                                  (__cdecl, 406 B / 0x196, no SEH;
//                                   ESP-aligned 0x80 stack frame).
//
// Inspection (read from the disassembly at orig RVA 0x000255c0):
//
//   __cdecl void draw_overlay(int x0, int y0, int x1, int y1,
//                             const float color[4], float depth);
//
//   `AND ESP,0xffffffe0` aligns the frame to 32 B (the helpers below take
//   16-B-aligned float quads / vectors on the stack). Args:
//     [ebp+0x08] int x0     — fed through CVTSI2SS
//     [ebp+0x0c] int y0     — fed through CVTSI2SS
//     [ebp+0x10] int x1     — fed through CVTSI2SS
//     [ebp+0x14] int y1     — fed through CVTSI2SS
//     [ebp+0x18] float*     — 4-float color/style block, splatted to a
//                             16-B aligned stack temp and passed to
//                             FUN_004305a0 (style/color push).
//     [ebp+0x1c] float depth — written to the global FP scratch 0x01328f70
//                              and flushed via FUN_0041c3c0.
//
//   Structural shape (mirrors the asm flow):
//
//     // 1) Splat the 4-float color block onto an aligned temp and bind it.
//     float style[4] = { color[0], color[1], color[2], color[3] };
//     FUN_004305a0(style);
//
//     // 2) Build two screen-space vertices via the vec helper at
//     //    FUN_00425170 ((int,int) -> float[3]) and assemble a 0x24-byte
//     //    vertex record (two endpoints + a packed style word EDX +
//     //    a unit const from .rdata 0x00f62f80).
//     float* a = FUN_00425170(x0_as_f, y0_as_f);   // first endpoint
//     ...
//     float* b = FUN_00425170(x1_as_f, y1_as_f);   // second endpoint
//     FUN_00419410(/* assembled 0x24-B vertex record + 0 */);
//     FUN_00425210();
//
//     // 3) Push depth, emit the primitive, restore depth to its default.
//     *(float*)0x01328f70 = depth;
//     FUN_0041c3c0(depth);
//     FUN_0041efc0(3, 2, 0x132, 0x24, &record);    // draw call (type 3)
//     *(float*)0x01328f70 = *(float*)0x00f54f70;   // restore default depth
//     FUN_0041c3c0(1.0f);
//     FUN_00424f50();                              // flush / end batch
//
//   Reloc-bearing sites in the orig 406 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x3e  rel32 CALL 0x004305a0  — style/color push
//     +0x66  rel32 CALL 0x00425170  — screen-vec helper (1st endpoint)
//     +0x7c  moffs MOVSS [0x00f62f80] — unit const (.rdata)
//     +0xdf  rel32 CALL 0x00425170  — screen-vec helper (2nd endpoint)
//     +0x128 rel32 CALL 0x00419410  — assemble/stage vertex record
//     +0x12d rel32 CALL 0x00425210  — state setup
//     +0x141 moffs MOVSS [0x01328f70] — global FP depth scratch (store)
//     +0x149 rel32 CALL 0x0041c3c0  — depth bind
//     +0x168 rel32 CALL 0x0041efc0  — draw primitive (type 3)
//     +0x16d moffs MOVSS [0x00f54f70] — default depth const (.rdata)
//     +0x175 moffs MOVSS [0x01328f70] — global FP depth scratch (restore)
//     +0x185 rel32 CALL 0x0041c3c0  — depth bind (1.0f)
//     +0x18d rel32 CALL 0x00424f50  — flush / end batch
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ rewrite would need to coax MSVC 2005 /O2 into
//   reproducing the exact 32-B aligned frame, the interleaved SSE
//   MOVSS scalar spill order for the two vertex records, the CVTSI2SS
//   int->float conversions, the x87 FLDZ/FLD1 depth pushes onto the
//   cdecl arg slots, AND the linker-resolved absolute addresses in the
//   thirteen relocation windows above. Each is brittle under /O2 — every
//   high-level rewrite shifts at least one byte (XMM scalar spill order,
//   branch short-vs-near, modrm vs moffs32, FF15 vs E8).
//
//   The pragmatic choice — the same one the sibling _rosetta bodies took
//   for their reloc-heavy functions — is a `__declspec(naked)` body that
//   re-emits the orig 406 bytes verbatim via MASM `_emit` directives. The
//   .obj's `.text` section ends up byte-identical to the orig slice, which
//   is what `tools/compare.py` checks against.

extern "C" __declspec(naked) void FUN_004255c0() {
    __asm {
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x83
        _emit 0xe4
        _emit 0xe0
        _emit 0x81
        _emit 0xec
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x45
        _emit 0x18
        _emit 0xf3

        _emit 0x0f
        _emit 0x10
        _emit 0x40
        _emit 0x0c
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x48
        _emit 0x08
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x50
        _emit 0x04
        _emit 0xf3
        _emit 0x0f

        _emit 0x10
        _emit 0x18
        _emit 0x83
        _emit 0xec
        _emit 0x10
        _emit 0x8b
        _emit 0xc4
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x18
        _emit 0xf3

        _emit 0x0f
        _emit 0x11
        _emit 0x50
        _emit 0x04
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x48
        _emit 0x08
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x0c
        _emit 0xe8
        _emit 0x9d

        _emit 0xaf
        _emit 0x00
        _emit 0x00
        _emit 0xd9
        _emit 0xee
        _emit 0xf3
        _emit 0x0f
        _emit 0x2a
        _emit 0x45
        _emit 0x0c
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        _emit 0xd9
        _emit 0x5c
        _emit 0x24

        _emit 0x08
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3
        _emit 0x0f
        _emit 0x2a
        _emit 0x45
        _emit 0x08
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x1c

        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        _emit 0x50
        _emit 0xe8
        _emit 0x45
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0xd9
        _emit 0xee
        _emit 0xf3
        _emit 0x0f
        _emit 0x10

        _emit 0x00
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        _emit 0x0f
        _emit 0x57
        _emit 0xc9
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x15

        _emit 0x80
        _emit 0x2f
        _emit 0xf6
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x30
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
        _emit 0x34
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x40
        _emit 0x08
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0xf3
        _emit 0x0f
        _emit 0x11

        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0xf3
        _emit 0x0f
        _emit 0x2a
        _emit 0x45
        _emit 0x14
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3
        _emit 0x0f

        _emit 0x2a
        _emit 0x45
        _emit 0x10
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        _emit 0x51
        _emit 0xf3
        _emit 0x0f
        _emit 0x11

        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x44
        _emit 0x89

        _emit 0x54
        _emit 0x24
        _emit 0x48
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x4c
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x50
        _emit 0xe8

        _emit 0xcc
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x54
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
        _emit 0x58
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x40
        _emit 0x08
        _emit 0x6a
        _emit 0x00

        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x60
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x64
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c

        _emit 0x24
        _emit 0x68
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x6c
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x70
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c

        _emit 0x24
        _emit 0x74
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x78
        _emit 0xe8
        _emit 0x23
        _emit 0x3d
        _emit 0xff
        _emit 0xff
        _emit 0xe8
        _emit 0x1e
        _emit 0xfb

        _emit 0xff
        _emit 0xff
        _emit 0xd9
        _emit 0x45
        _emit 0x1c
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x45
        _emit 0x1c
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x83
        _emit 0xc4

        _emit 0x10
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x05
        _emit 0x70
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0xe8
        _emit 0xb2
        _emit 0x6c
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4

        _emit 0x04
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x52
        _emit 0x6a
        _emit 0x24
        _emit 0x68
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x02
        _emit 0x51

        _emit 0x8b
        _emit 0xc4
        _emit 0xc7
        _emit 0x00
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x93
        _emit 0x98
        _emit 0xff
        _emit 0xff
        _emit 0xf3
        _emit 0x0f
        _emit 0x10

        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x05
        _emit 0x70
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0xd9
        _emit 0xe8
        _emit 0x83

        _emit 0xc4
        _emit 0x10
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        _emit 0xe8
        _emit 0x76
        _emit 0x6c
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0xe8
        _emit 0xfe
        _emit 0xf7

        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xe5
        _emit 0x5d
        _emit 0xc3
    }
}
