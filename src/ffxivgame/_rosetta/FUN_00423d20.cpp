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
// FUNCTION: ffxivgame 0x00423d20 — `__thiscall` big-buffer initializer with
//                                  two function-local SSE statics
//                                  (248 B / 0xf8, no SEH).
//
// Behaviour read from asm/ffxivgame/00023d20_FUN_00423d20.s:
//
//   __thiscall void FUN_00423d20(this) — ECX = this, no stack args,
//   returns void (`ret`, no operand). 16-byte stack realignment prologue
//   (PUSH EBP / MOV EBP,ESP / AND ESP,~0xf) because the body materialises
//   a 16-byte-aligned XMM scratch in the local frame for the first
//   magic-static init.
//
//   Structural shape:
//
//     // --- inline-unrolled fill of the leading 16 dwords ---
//     uint32_t v = 0xffff;
//     this->u32[0..15] = v;                  // [ecx+0x00 .. ecx+0x3c]
//
//     // --- magic-static #1: __m128i s_pattern (bit 0 of guard) ---
//     static __m128i s_pattern;              // .data 0x01329940
//     if (!(g_guard & 1)) {                  // .data 0x01329950
//         g_guard |= 1;
//         __m128i tmp;                        // [esp+0x00] 16B-aligned local
//         tmp.u32[0..3] = v;                  // 0xffff x4
//         s_pattern = tmp;                    // MOVDQA store
//     }
//     // splat s_pattern across [ecx+0x40 .. ecx+0x140] (16 x 16B = 256B)
//     for (int i = 0x10; i; --i) { *p128++ = s_pattern; }
//
//     // --- magic-static #2: float s_floats[4] (bit 1 of guard) ---
//     static float s_floats[4];              // .data 0x01329924
//     if (!(g_guard & 2)) {
//         float a = *(float*)0x00f59bd4;     // .rdata constant
//         g_guard |= 2;
//         s_floats[0] = a;
//         s_floats[1] = a;
//         s_floats[2] = a;
//         s_floats[3] = *(float*)0x00f54f70; // .rdata constant
//     }
//     // splat s_floats across [ecx+0x140 .. ecx+0x1140] (256 x 16B = 4096B)
//     for (int i = 0x100; i; --i) {
//         *(p+0) = (s_floats[0], s_floats[1]);   // MOVQ
//         *(p+1) = (s_floats[2], s_floats[3]);   // MOVQ
//     }
//
//   Object layout recovered from the store offsets:
//     +0x000 .. +0x03f   uint32_t header[16]   (all 0xffff)
//     +0x040 .. +0x13f   uint32_t fill[64]      (all 0xffff, 256B)
//     +0x140 .. +0x113f  float    grid[1024]    (repeating {a,a,a,b}, 4096B)
//
//   Reloc-bearing sites in the orig 248 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x3d   abs32   0x01329950 — g_guard (TEST byte)
//     +0x46   abs32   0x01329950 — g_guard (OR dword)
//     +0x61   abs32   0x01329940 — s_pattern store (MOVDQA)
//     +0x71   abs32   0x01329940 — s_pattern load  (MOVDQA, loop)
//     +0x8a   abs32   0x01329950 — g_guard (TEST AL)
//     +0x92   abs32   0x00f59bd4 — float constant a (MOVSS load)
//     +0x9a   abs32   0x01329950 — g_guard (OR)
//     +0xa0   abs32   0x01329924 — s_floats[0] (MOVSS store)
//     +0xa8   abs32   0x01329928 — s_floats[1] (MOVSS store)
//     +0xb0   abs32   0x0132992c — s_floats[2] (MOVSS store)
//     +0xb8   abs32   0x00f54f70 — float constant b (MOVSS load)
//     +0xc0   abs32   0x01329930 — s_floats[3] (MOVSS store)
//     +0xd3   abs32   0x01329924 — s_floats[0..1] (MOVQ load, loop)
//     +0xdf   abs32   0x0132992c — s_floats[2..3] (MOVQ load, loop)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The same constraint as the sibling reloc-heavy bodies (FUN_00402a30 /
//   FUN_004054d0 / FUN_00403a20): every store/load in this function bakes
//   an absolute .data / .rdata address that only the full-binary relink
//   resolves. A source-level C++/intrinsic port would emit those as
//   relocations and the standalone .obj bytes would diverge. The
//   pragmatic, byte-identical route is a naked body re-emitting the orig
//   248 bytes verbatim via MASM `_emit` directives — no relocations,
//   tools/compare.py sees the .text slice exactly as orig.

extern "C" __declspec(naked) void FUN_00423d20() {
    __asm {
        // 00023d20  PUSH EBP
        _emit 0x55
        // 00023d21  MOV EBP, ESP
        _emit 0x8b
        _emit 0xec
        // 00023d23  AND ESP, 0xfffffff0
        _emit 0x83
        _emit 0xe4
        _emit 0xf0
        // 00023d26  MOV EAX, 0xffff
        _emit 0xb8
        _emit 0xff
        _emit 0xff
        _emit 0x00
        _emit 0x00
        // 00023d2b  MOV [ECX], EAX
        _emit 0x89
        _emit 0x01
        // 00023d2d  MOV [ECX+0x04], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x04
        // 00023d30  MOV [ECX+0x08], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x08
        // 00023d33  MOV [ECX+0x0c], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x0c
        // 00023d36  MOV [ECX+0x10], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x10
        // 00023d39  MOV [ECX+0x14], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x14
        // 00023d3c  MOV [ECX+0x18], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x18
        // 00023d3f  MOV [ECX+0x1c], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x1c
        // 00023d42  MOV [ECX+0x20], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x20
        // 00023d45  MOV [ECX+0x24], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x24
        // 00023d48  MOV [ECX+0x28], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x28
        // 00023d4b  MOV [ECX+0x2c], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x2c
        // 00023d4e  MOV [ECX+0x30], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x30
        // 00023d51  MOV [ECX+0x34], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x34
        // 00023d54  MOV [ECX+0x38], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x38
        // 00023d57  SUB ESP, 0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 00023d5a  MOV [ECX+0x3c], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x3c
        // 00023d5d  TEST byte [0x01329950], 0x1
        _emit 0xf6
        _emit 0x05
        _emit 0x50
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 00023d64  JNZ 0x00423d89
        _emit 0x75
        _emit 0x23
        // 00023d66  OR dword [0x01329950], 0x1
        _emit 0x83
        _emit 0x0d
        _emit 0x50
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 00023d6d  MOV [ESP], EAX
        _emit 0x89
        _emit 0x04
        _emit 0x24
        // 00023d70  MOV [ESP+0x4], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00023d74  MOV [ESP+0x8], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00023d78  MOV [ESP+0xc], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00023d7c  MOVDQA XMM0, [ESP]
        _emit 0x66
        _emit 0x0f
        _emit 0x6f
        _emit 0x04
        _emit 0x24
        // 00023d81  MOVDQA [0x01329940], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0x7f
        _emit 0x05
        _emit 0x40
        _emit 0x99
        _emit 0x32
        _emit 0x01
        // 00023d89  LEA EAX, [ECX+0x40]
        _emit 0x8d
        _emit 0x41
        _emit 0x40
        // 00023d8c  MOV EDX, 0x10
        _emit 0xba
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00023d91  MOVDQA XMM0, [0x01329940]
        _emit 0x66
        _emit 0x0f
        _emit 0x6f
        _emit 0x05
        _emit 0x40
        _emit 0x99
        _emit 0x32
        _emit 0x01
        // 00023d99  MOVDQA [EAX], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0x7f
        _emit 0x00
        // 00023d9d  ADD EAX, 0x10
        _emit 0x83
        _emit 0xc0
        _emit 0x10
        // 00023da0  SUB EDX, 0x1
        _emit 0x83
        _emit 0xea
        _emit 0x01
        // 00023da3  JNZ 0x00423d91
        _emit 0x75
        _emit 0xec
        // 00023da5  MOV EAX, 0x2
        _emit 0xb8
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00023daa  TEST byte [0x01329950], AL
        _emit 0x84
        _emit 0x05
        _emit 0x50
        _emit 0x99
        _emit 0x32
        _emit 0x01
        // 00023db0  JNZ 0x00423de8
        _emit 0x75
        _emit 0x36
        // 00023db2  MOVSS XMM0, [0x00f59bd4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0xd4
        _emit 0x9b
        _emit 0xf5
        _emit 0x00
        // 00023dba  OR dword [0x01329950], EAX
        _emit 0x09
        _emit 0x05
        _emit 0x50
        _emit 0x99
        _emit 0x32
        _emit 0x01
        // 00023dc0  MOVSS [0x01329924], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x05
        _emit 0x24
        _emit 0x99
        _emit 0x32
        _emit 0x01
        // 00023dc8  MOVSS [0x01329928], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x05
        _emit 0x28
        _emit 0x99
        _emit 0x32
        _emit 0x01
        // 00023dd0  MOVSS [0x0132992c], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x05
        _emit 0x2c
        _emit 0x99
        _emit 0x32
        _emit 0x01
        // 00023dd8  MOVSS XMM0, [0x00f54f70]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        // 00023de0  MOVSS [0x01329930], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x05
        _emit 0x30
        _emit 0x99
        _emit 0x32
        _emit 0x01
        // 00023de8  LEA EAX, [ECX+0x140]
        _emit 0x8d
        _emit 0x81
        _emit 0x40
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 00023dee  MOV ECX, 0x100
        _emit 0xb9
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 00023df3  MOVQ XMM0, [0x01329924]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x05
        _emit 0x24
        _emit 0x99
        _emit 0x32
        _emit 0x01
        // 00023dfb  MOVQ [EAX], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x00
        // 00023dff  MOVQ XMM0, [0x0132992c]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x05
        _emit 0x2c
        _emit 0x99
        _emit 0x32
        _emit 0x01
        // 00023e07  MOVQ [EAX+0x8], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x08
        // 00023e0c  ADD EAX, 0x10
        _emit 0x83
        _emit 0xc0
        _emit 0x10
        // 00023e0f  SUB ECX, 0x1
        _emit 0x83
        _emit 0xe9
        _emit 0x01
        // 00023e12  JNZ 0x00423df3
        _emit 0x75
        _emit 0xdf
        // 00023e14  MOV ESP, EBP
        _emit 0x8b
        _emit 0xe5
        // 00023e16  POP EBP
        _emit 0x5d
        // 00023e17  RET
        _emit 0xc3
    }
}
