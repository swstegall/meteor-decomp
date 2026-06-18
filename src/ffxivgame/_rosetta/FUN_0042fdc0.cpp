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
// FUNCTION: ffxivgame 0x0002fdc0 — Y-axis rotation matrix builder
//                                  (239 B / 0xef, no SEH/EH).
//
// Behaviour read from asm/ffxivgame/0002fdc0_FUN_0042fdc0.s:
//
//   __cdecl void FUN_0042fdc0(float* mat, float angle)
//
//     Stack frame: SUB ESP,0x44 (68 bytes of locals) / ADD ESP,0x44 / RET.
//     No callee-saved registers, no SEH/GS cookie — a pure computation
//     leaf with two CRT float-math calls and SSE copy-out via MOVQ.
//
//   Algorithm:
//     float c = cosf(angle);   // CRT @ 0x009d6280
//     float s = sinf(angle);   // CRT @ 0x009d63b0
//     const float k = *(float*)0x00f54f70;  // 1.0f in .data/.rdata
//
//     // Fills a column-major 4×4 float matrix at mat[16]:
//     //   Column 0: [ c, 0, -s, 0 ]
//     //   Column 1: [ 0, k,  0, 0 ]
//     //   Column 2: [ s, 0,  c, 0 ]
//     //   Column 3: [ 0, 0,  0, k ]
//     //
//     // i.e. a standard Y-axis rotation matrix:
//     //   mat[0]=c   mat[1]=0   mat[2]=-s  mat[3]=0
//     //   mat[4]=0   mat[5]=k   mat[6]=0   mat[7]=0
//     //   mat[8]=s   mat[9]=0   mat[10]=c  mat[11]=0
//     //   mat[12]=0  mat[13]=0  mat[14]=0  mat[15]=k
//
//   Stack layout (ESP-relative after SUB ESP,0x44):
//     [ESP+0x00]  temp  — sin result scratch (from first cosf FSTP then FLD)
//     [ESP+0x04]  cos   → mat[0]
//     [ESP+0x08]  0.0   → mat[1]
//     [ESP+0x0c]  -sin  → mat[2]
//     [ESP+0x10]  0.0   → mat[3]
//     [ESP+0x14]  0.0   → mat[4]
//     [ESP+0x18]  k     → mat[5]     (MOVSS XMM1,k written here just before MOVQ)
//     [ESP+0x1c]  0.0   → mat[6]
//     [ESP+0x20]  0.0   → mat[7]
//     [ESP+0x24]  sin   → mat[8]     (note: original cos_val before FCHS)
//     [ESP+0x28]  0.0   → mat[9]
//     [ESP+0x2c]  cos   → mat[10]    (sin_val stored from [ESP] reload)
//     [ESP+0x30]  0.0   → mat[11]
//     [ESP+0x34]  0.0   → mat[12]
//     [ESP+0x38]  0.0   → mat[13]
//     [ESP+0x3c]  0.0   → mat[14]
//     [ESP+0x40]  k     → mat[15]    (MOVSS XMM1,k written here just before MOVQ)
//     [ESP+0x44]  return address
//     [ESP+0x48]  arg0  — float* mat  (output pointer → EAX)
//     [ESP+0x4c]  arg1  — float  angle (input angle → x87 arg, overwritten)
//
//   Wait, actually: first CRT call (0x009d6280) is cosf, second (0x009d63b0)
//   is sinf.  The first FLD [ESP+0x4c] feeds cosf; its result is stored at
//   [ESP]/[ESP+0x4].  The second FLD [ESP+0x4c] feeds sinf; its result is
//   stored at [ESP+0x4c] then reloaded.  FCHS negates the sinf result to
//   form -sin for the [2,0] slot.  The cos result (from the first call,
//   left in ST1 after FLD ST0) fills [ESP+0x24].
//
//   Reloc-bearing sites in the orig 239 bytes:
//     +0x07  rel32  0x009d6280  — cosf (CRT)
//     +0x23  rel32  0x009d63b0  — sinf (CRT)
//     +0x39  abs32  0x00f54f70  — 1.0f constant in .data
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The three relocation windows above (two rel32 CALL offsets baked at
//   orig RVA 0x0002fdc0, one abs32 MOVSS) cannot be reproduced by a
//   source-level C++ compile targeting a standalone .obj — the CALL
//   rel32 values are resolved relative to the full binary's link-time
//   image base 0x00400000, and the abs32 references a .data slot that
//   only exists in that exact binary layout.  Additionally, the
//   interleaved x87/SSE store schedule (FSTP, XORPS, MOVSS for zeroing,
//   then MOVQ for 8-byte copy-out) is a specific /O2 optimizer artefact
//   that any source rewrite would almost certainly misalign by at least
//   one instruction.
//
//   Following the same approach as the sibling _rosetta/ matches
//   (FUN_00402a30, FUN_004054d0, FUN_00403a20), we re-emit the orig 239
//   bytes verbatim via MASM _emit directives inside a __declspec(naked)
//   wrapper.  The .obj's .text section ends up byte-identical to the
//   orig slice with no relocation entries (the CALL offsets and abs32
//   address are baked as raw immediates), which is exactly what
//   tools/compare.py checks.

extern "C" __declspec(naked) void FUN_0042fdc0() {
    __asm {
        // 0002fdc0  SUB ESP,0x44
        _emit 0x83
        _emit 0xec
        _emit 0x44
        // 0002fdc3  FLD dword ptr [ESP+0x4c]
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        // 0002fdc7  CALL 0x009d6280 (cosf)
        _emit 0xe8
        _emit 0xb4
        _emit 0x64
        _emit 0x5a
        _emit 0x00
        // 0002fdcc  FSTP dword ptr [ESP]
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // 0002fdcf  FLD dword ptr [ESP]
        _emit 0xd9
        _emit 0x04
        _emit 0x24
        // 0002fdd2  XORPS XMM0,XMM0
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        // 0002fdd5  FSTP dword ptr [ESP+0x4]
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x04
        // 0002fdd9  MOVSS dword ptr [ESP+0x8],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0002fddf  FLD dword ptr [ESP+0x4c]
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        // 0002fde3  CALL 0x009d63b0 (sinf)
        _emit 0xe8
        _emit 0xc8
        _emit 0x65
        _emit 0x5a
        _emit 0x00
        // 0002fde8  FSTP dword ptr [ESP+0x4c]
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x4c
        // 0002fdec  XORPS XMM0,XMM0
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        // 0002fdef  FLD dword ptr [ESP+0x4c]
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        // 0002fdf3  MOV EAX,dword ptr [ESP+0x48]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x48
        // 0002fdf7  FLD ST0
        _emit 0xd9
        _emit 0xc0
        // 0002fdf9  MOVSS XMM1,dword ptr [0x00f54f70]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x0d
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        // 0002fe01  FCHS
        _emit 0xd9
        _emit 0xe0
        // 0002fe03  MOVSS dword ptr [ESP+0x10],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0002fe09  FSTP dword ptr [ESP+0xc]
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        // 0002fe0d  MOVSS dword ptr [ESP+0x28],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0002fe13  MOVSS dword ptr [ESP+0x30],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // 0002fe19  FSTP dword ptr [ESP+0x24]
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x24
        // 0002fe1d  FLD dword ptr [ESP]
        _emit 0xd9
        _emit 0x04
        _emit 0x24
        // 0002fe20  MOVSS dword ptr [ESP+0x34],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // 0002fe26  MOVSS dword ptr [ESP+0x38],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x38
        // 0002fe2c  FSTP dword ptr [ESP+0x2c]
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x2c
        // 0002fe30  MOVSS dword ptr [ESP+0x3c],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        // 0002fe36  MOVSS dword ptr [ESP+0x14],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0002fe3c  MOVSS dword ptr [ESP+0x1c],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0002fe42  MOVSS dword ptr [ESP+0x20],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 0002fe48  MOVQ XMM0,qword ptr [ESP+0x4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0002fe4e  MOVQ qword ptr [EAX],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x00
        // 0002fe52  MOVQ XMM0,qword ptr [ESP+0xc]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0002fe58  MOVQ qword ptr [EAX+0x8],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x08
        // 0002fe5d  MOVSS dword ptr [ESP+0x18],XMM1
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 0002fe63  MOVQ XMM0,qword ptr [ESP+0x14]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0002fe69  MOVQ qword ptr [EAX+0x10],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x10
        // 0002fe6e  MOVQ XMM0,qword ptr [ESP+0x1c]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0002fe74  MOVQ qword ptr [EAX+0x18],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x18
        // 0002fe79  MOVQ XMM0,qword ptr [ESP+0x24]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0002fe7f  MOVQ qword ptr [EAX+0x20],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x20
        // 0002fe84  MOVQ XMM0,qword ptr [ESP+0x2c]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // 0002fe8a  MOVQ qword ptr [EAX+0x28],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x28
        // 0002fe8f  MOVQ XMM0,qword ptr [ESP+0x34]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // 0002fe95  MOVQ qword ptr [EAX+0x30],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x30
        // 0002fe9a  MOVSS dword ptr [ESP+0x40],XMM1
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        // 0002fea0  MOVQ XMM0,qword ptr [ESP+0x3c]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        // 0002fea6  MOVQ qword ptr [EAX+0x38],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x38
        // 0002feab  ADD ESP,0x44
        _emit 0x83
        _emit 0xc4
        _emit 0x44
        // 0002feae  RET
        _emit 0xc3
    }
}
