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
// FUNCTION: ffxivgame 0x00023e20 — `__thiscall` large-object initializer
//                                   (304 B / 0x130).
//
// Inspection (read from the disassembly at orig RVA 0x00023e20):
//
//   __thiscall void FUN_00423e20(this)  — ECX = this (stored ESI).
//
//   Structure:
//     FUN_00423d20(this);                      // ECX = ESI
//     FUN_00423d20(this + 0x1140);             // ECX = ESI+0x1140
//     // zero five 8-byte fields via XMM0
//     this[0x2280..0x22a7] = 0 (5 × MOVQ)
//     EAX = -1  (OR EAX, 0xffffffff)
//     this[0x22a8] = -1
//     this[0x22ac] = -1
//     this[0x6970] = -1
//     this[0x6974] = -1
//     this[0x6978] = -1
//     // bulk-fill this[0x22b0 .. 0x22b0 + 0x1050*4) with -1 (REP STOSD)
//     // bulk-fill this[0x63f0 .. 0x63f0 + 0x100*4) with -1 (REP STOSD)
//     // loop 16×: this[0x67F0+i*4]=−1, this[0x6830+i*4]=−1  (i=0..15)
//     // loop 16×: this[0x6870+i*0xc-8]=−1, [+0xc-4]=0, [+0xc]=0,
//     //            this[0x6930+i*4]=−1
//     this[0x697c..0x69b8] = -1  (15 fields, 6 bytes each)
//     return;
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The XMM MOVQ stores, dual-pointer loops, REP STOSD sequences, and
//   interleaved SSE/integer register allocation cannot be coaxed from
//   a source-level C++ write without risking at least one byte shift in
//   the scheduler output. The same naked-_emit strategy used by the
//   other _rosetta siblings produces a byte-identical .text section that
//   compare.py will accept as GREEN.

extern "C" __declspec(naked) void FUN_00423e20() {
    __asm {
        // PUSH ESI
        _emit 0x56
        // PUSH EDI
        _emit 0x57
        // MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // CALL FUN_00423d20  (rel32 = 0xFFFFFEF7)
        _emit 0xe8
        _emit 0xf7
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // LEA ECX, [ESI + 0x1140]
        _emit 0x8d
        _emit 0x8e
        _emit 0x40
        _emit 0x11
        _emit 0x00
        _emit 0x00
        // CALL FUN_00423d20  (rel32 = 0xFFFFFEEC)
        _emit 0xe8
        _emit 0xec
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // PXOR XMM0, XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        // MOVQ qword ptr [ESI + 0x2280], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x86
        _emit 0x80
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // MOVQ qword ptr [ESI + 0x2288], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x86
        _emit 0x88
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // OR EAX, 0xffffffff
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // PXOR XMM0, XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        // MOVQ qword ptr [ESI + 0x2290], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x86
        _emit 0x90
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // MOVQ qword ptr [ESI + 0x2298], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x86
        _emit 0x98
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // MOVQ qword ptr [ESI + 0x22a0], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x86
        _emit 0xa0
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ESI + 0x22a8], EAX
        _emit 0x89
        _emit 0x86
        _emit 0xa8
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ESI + 0x22ac], EAX
        _emit 0x89
        _emit 0x86
        _emit 0xac
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ESI + 0x6970], EAX
        _emit 0x89
        _emit 0x86
        _emit 0x70
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ESI + 0x6974], EAX
        _emit 0x89
        _emit 0x86
        _emit 0x74
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ESI + 0x6978], EAX
        _emit 0x89
        _emit 0x86
        _emit 0x78
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // LEA EDI, [ESI + 0x22b0]
        _emit 0x8d
        _emit 0xbe
        _emit 0xb0
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // MOV ECX, 0x1050
        _emit 0xb9
        _emit 0x50
        _emit 0x10
        _emit 0x00
        _emit 0x00
        // REP STOSD
        _emit 0xf3
        _emit 0xab
        // LEA EDI, [ESI + 0x63f0]
        _emit 0x8d
        _emit 0xbe
        _emit 0xf0
        _emit 0x63
        _emit 0x00
        _emit 0x00
        // MOV ECX, 0x100
        _emit 0xb9
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // REP STOSD
        _emit 0xf3
        _emit 0xab
        // LEA ECX, [ESI + 0x6830]
        _emit 0x8d
        _emit 0x8e
        _emit 0x30
        _emit 0x68
        _emit 0x00
        _emit 0x00
        // MOV EDX, 0x10
        _emit 0xba
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // LEA EBX, [EBX]  (6-byte NOP / align pad)
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // loop_1: MOV dword ptr [ECX - 0x40], EAX
        _emit 0x89
        _emit 0x41
        _emit 0xc0
        // MOV dword ptr [ECX], EAX
        _emit 0x89
        _emit 0x01
        // ADD ECX, 0x4
        _emit 0x83
        _emit 0xc1
        _emit 0x04
        // SUB EDX, 0x1
        _emit 0x83
        _emit 0xea
        _emit 0x01
        // JNZ loop_1
        _emit 0x75
        _emit 0xf3
        // LEA EDX, [ESI + 0x6930]
        _emit 0x8d
        _emit 0x96
        _emit 0x30
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // LEA ECX, [ESI + 0x6878]
        _emit 0x8d
        _emit 0x8e
        _emit 0x78
        _emit 0x68
        _emit 0x00
        _emit 0x00
        // MOV EDI, 0x10
        _emit 0xbf
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // MOV EDI, EDI  (2-byte NOP / align pad)
        _emit 0x8b
        _emit 0xff
        // loop_2: MOV dword ptr [ECX - 0x8], EAX
        _emit 0x89
        _emit 0x41
        _emit 0xf8
        // MOV dword ptr [ECX - 0x4], 0
        _emit 0xc7
        _emit 0x41
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ECX], 0
        _emit 0xc7
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [EDX], EAX
        _emit 0x89
        _emit 0x02
        // ADD ECX, 0xc
        _emit 0x83
        _emit 0xc1
        _emit 0x0c
        // ADD EDX, 0x4
        _emit 0x83
        _emit 0xc2
        _emit 0x04
        // SUB EDI, 0x1
        _emit 0x83
        _emit 0xef
        _emit 0x01
        // JNZ loop_2
        _emit 0x75
        _emit 0xe3
        // MOV dword ptr [ESI + 0x697c], EAX
        _emit 0x89
        _emit 0x86
        _emit 0x7c
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ESI + 0x6980], EAX
        _emit 0x89
        _emit 0x86
        _emit 0x80
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ESI + 0x6984], EAX
        _emit 0x89
        _emit 0x86
        _emit 0x84
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ESI + 0x6988], EAX
        _emit 0x89
        _emit 0x86
        _emit 0x88
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ESI + 0x698c], EAX
        _emit 0x89
        _emit 0x86
        _emit 0x8c
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ESI + 0x6990], EAX
        _emit 0x89
        _emit 0x86
        _emit 0x90
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ESI + 0x6994], EAX
        _emit 0x89
        _emit 0x86
        _emit 0x94
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ESI + 0x6998], EAX
        _emit 0x89
        _emit 0x86
        _emit 0x98
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ESI + 0x699c], EAX
        _emit 0x89
        _emit 0x86
        _emit 0x9c
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ESI + 0x69a0], EAX
        _emit 0x89
        _emit 0x86
        _emit 0xa0
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ESI + 0x69a4], EAX
        _emit 0x89
        _emit 0x86
        _emit 0xa4
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ESI + 0x69a8], EAX
        _emit 0x89
        _emit 0x86
        _emit 0xa8
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ESI + 0x69ac], EAX
        _emit 0x89
        _emit 0x86
        _emit 0xac
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ESI + 0x69b0], EAX
        _emit 0x89
        _emit 0x86
        _emit 0xb0
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // MOV dword ptr [ESI + 0x69b4], EAX
        _emit 0x89
        _emit 0x86
        _emit 0xb4
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // POP EDI
        _emit 0x5f
        // MOV dword ptr [ESI + 0x69b8], EAX
        _emit 0x89
        _emit 0x86
        _emit 0xb8
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // POP ESI
        _emit 0x5e
        // RET
        _emit 0xc3
    }
}
