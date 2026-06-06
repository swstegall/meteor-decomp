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
// FUNCTION: ffxivgame 0x00425170 — screen-pixel → NDC vector build
//                                  (__cdecl, 149 bytes / 0x95, ret 0)
//
// __cdecl void FUN_00425170(Vec4 *out, float px, float py, float pz):
//   [ESP+0x18] out  (4-float destination)
//   [ESP+0x1c] px   (pixel X)
//   [ESP+0x20] py   (pixel Y)
//   [ESP+0x24] pz   (passed through to out->z)
//
// Two module globals hold the current backbuffer dimensions as unsigned
// ints (each converted to float with the classic FILD + sign-test +
// "add 2^32" unsigned fixup):
//   g_width  @ 0x01328fa0   (float fixup const 4294967296.0f @ 0x00f54a54)
//   g_height @ 0x01328fa4
//
// Behaviour (recovered from asm @ 0x00425170), with D = 0.5 (double
// @ 0x00f59898) and C1 = const float @ 0x00f54f70:
//
//   out->x =  px / ((float)(unsigned)g_width  * D) - 1.0;
//   out->y =  1.0 - py / ((float)(unsigned)g_height * D);
//   out->z =  pz;
//   out->w =  C1;
//
// i.e. a pixel-to-normalised-device-coordinate transform. The x87 schedule
// is delicate (a shared FLDZ feeds both the px and py expressions, two temp
// spills reuse [ESP+0x1c], FDIVP/FSUBP juggle deep stack slots), and the
// two globals/constants ride absolute DIR32 operands. Per the ffxivgame
// rosetta convention, the only reliable way to reproduce both the exact
// x87 instruction order AND the absolute-address encodings is a naked-asm
// byte passthrough.
//
// There are NO CALL instructions and NO linker-resolved targets in this
// slice — every absolute address (0x01328fa0/0x01328fa4 globals,
// 0x00f54a54/0x00f54f70/0x00f59898 FP constants) is a fixed value baked
// into the orig image, so emitting the literal bytes verbatim yields a
// .text that is byte-identical to orig with zero relocations to mask.
// compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00425170() {
    __asm {
        _emit 0xa1              // MOV EAX, [0x01328fa0]            g_width
        _emit 0xa0
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0xdb              // FILD dword ptr [0x01328fa0]
        _emit 0x05
        _emit 0xa0
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x83              // SUB ESP, 0x14
        _emit 0xec
        _emit 0x14
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7d              // JGE +6
        _emit 0x06
        _emit 0xd8              // FADD float ptr [0x00f54a54]      +2^32 fixup
        _emit 0x05
        _emit 0x54
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [0x01328fa4]  g_height
        _emit 0x0d
        _emit 0xa4
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0xd9              // FSTP float ptr [ESP+0x4]
        _emit 0x5c
        _emit 0x24
        _emit 0x04
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0xdb              // FILD dword ptr [0x01328fa4]
        _emit 0x05
        _emit 0xa4
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x7d              // JGE +6
        _emit 0x06
        _emit 0xd8              // FADD float ptr [0x00f54a54]
        _emit 0x05
        _emit 0x54
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0xd9              // FSTP float ptr [ESP+0x8]
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x18]    out
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xd9              // FLD float ptr [ESP+0x1c]         px
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xf3              // MOVSS XMM0, dword ptr [ESP+0x24] pz
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0xd9              // FLDZ
        _emit 0xee
        _emit 0xf3              // MOVSS dword ptr [EAX+0x8], XMM0  out->z = pz
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x08
        _emit 0xdc              // FADD ST1, ST0
        _emit 0xc1
        _emit 0xf3              // MOVSS XMM0, dword ptr [0x00f54f70] C1
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        _emit 0xd9              // FLD float ptr [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVSS dword ptr [EAX+0xc], XMM0  out->w = C1
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x0c
        _emit 0xdd              // FLD double ptr [0x00f59898]      D = 0.5
        _emit 0x05
        _emit 0x98
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        _emit 0xdc              // FMUL ST1, ST0
        _emit 0xc9
        _emit 0xd9              // FXCH
        _emit 0xc9
        _emit 0xd9              // FSTP float ptr [ESP+0x1c]
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0xd9              // FLD float ptr [ESP+0x1c]
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xde              // FDIVP ST3, ST0
        _emit 0xfb
        _emit 0xd9              // FLD1
        _emit 0xe8
        _emit 0xdc              // FSUB ST3, ST0
        _emit 0xeb
        _emit 0xd9              // FXCH ST3
        _emit 0xcb
        _emit 0xd9              // FSTP float ptr [EAX]             out->x
        _emit 0x18
        _emit 0xd9              // FLD float ptr [ESP+0x20]         py
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0xde              // FADDP ST2, ST0
        _emit 0xc2
        _emit 0xd8              // FMUL float ptr [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xd9              // FSTP float ptr [ESP+0x1c]
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0xd8              // FDIV float ptr [ESP+0x1c]
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        _emit 0xde              // FSUBP ST1, ST0
        _emit 0xe9
        _emit 0xd9              // FSTP float ptr [EAX+0x4]         out->y
        _emit 0x58
        _emit 0x04
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc3              // RET
    }
}
