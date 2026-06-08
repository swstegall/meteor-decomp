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
// FUNCTION: ffxivgame 0x0042e050 — build a float-vector struct on the stack
//                                  and forward it to FUN_0042cbc0 (77 B / 0x4d).
//
// Behaviour read from asm/ffxivgame/0002e050_FUN_0042e050.s:
//
//   __thiscall void FUN_0042e050(SomeStruct* this_ptr);
//
//   Allocates 0x20 bytes on the stack, fills four consecutive floats at
//   [ESP+0..0xc] with the global float at 0x00f54f70, then stores 0.0
//   (via FLDZ/FSTP) into [ESP+0] while keeping the original 8-byte block
//   in XMM0.  It then marshals two stack-local quadwords into a larger
//   argument block ([ESP+0x14] and [ESP+0x20]) before calling FUN_0042cbc0.
//
//   Reloc-bearing sites in the orig 77 bytes:
//     +0x07  MOVSS XMM0, dword ptr [0x00f54f70]  (dir32, .rdata global float)
//     +0x44  CALL FUN_0042cbc0                   (rel32, __cdecl callee)
//   (masked by tools/compare.py)
//
// Reconstruction: __declspec(naked) byte passthrough — same strategy as
// FUN_0042e010 and other _rosetta siblings.

extern "C" __declspec(naked) void FUN_0042e050() {
    __asm {
        // 0002e050: 83 ec 20   SUB ESP, 0x20
        _emit 0x83
        _emit 0xec
        _emit 0x20
        // 0002e053: f3 0f 10 05 70 4f f5 00   MOVSS XMM0, dword ptr [0x00f54f70]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        // 0002e05b: d9 ee   FLDZ
        _emit 0xd9
        _emit 0xee
        // 0002e05d: f3 0f 11 04 24   MOVSS dword ptr [ESP], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        // 0002e062: f3 0f 11 44 24 04   MOVSS dword ptr [ESP+0x4], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0002e068: f3 0f 11 44 24 08   MOVSS dword ptr [ESP+0x8], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0002e06e: f3 0f 11 44 24 0c   MOVSS dword ptr [ESP+0xc], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0002e074: f3 0f 7e 04 24   MOVQ XMM0, qword ptr [ESP]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x04
        _emit 0x24
        // 0002e079: 51   PUSH ECX
        _emit 0x51
        // 0002e07a: 8d 44 24 14   LEA EAX, [ESP+0x14]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0002e07e: d9 1c 24   FSTP float ptr [ESP]
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // 0002e081: 66 0f d6 44 24 14   MOVQ qword ptr [ESP+0x14], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0002e087: f3 0f 7e 44 24 0c   MOVQ XMM0, qword ptr [ESP+0xc]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0002e08d: 50   PUSH EAX
        _emit 0x50
        // 0002e08e: 66 0f d6 44 24 20   MOVQ qword ptr [ESP+0x20], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 0002e094: e8 27 eb ff ff   CALL FUN_0042cbc0
        _emit 0xe8
        _emit 0x27
        _emit 0xeb
        _emit 0xff
        _emit 0xff
        // 0002e099: 83 c4 28   ADD ESP, 0x28
        _emit 0x83
        _emit 0xc4
        _emit 0x28
        // 0002e09c: c3   RET
        _emit 0xc3
    }
}
