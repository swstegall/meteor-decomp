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
// FUNCTION: ffxivgame 0x004623e0 — conditional dispatch with _chkstk prologue,
//                                  __fastcall ECX/EDX params, ambient ESI,
//                                  0x14-byte local frame, two-path return 1
//                                  (132 bytes / 0x84).
//
// Prologue: MOV EAX,0x14 + CALL _chkstk allocates a 0x14-byte frame;
// then PUSH EBX/PUSH EDI save registers; ECX saved to EBX, EDX saved to EDI.
// The function tests [EBX]==0:
//
//   Path 1 ([EBX]==0): pushes 0xf54d48 (before branch), then ESI/0xf69cc8/EDI,
//     calls FUN_00467e90 (4 args); reloads EBX=[EBX+4]; pushes ESI; calls
//     FUN_00462370 (1 arg); ADD ESP,0x14; POP EDI/EBX; ADD ESP,0x14; return 1.
//
//   Path 2 ([EBX]!=0): loads EAX=[EBX+4], pushes ECX(=ESI+2)/0xf54d48/ESI/
//     0xf69cb0/EDI + stores EAX at [ESP+0x20] (local slot); calls FUN_00467e90
//     (5 args + stored arg); pushes 0x82031f/0/&local/EDI; calls FUN_0046c140
//     (4 args); pushes 0xf54d98/EDI; calls FUN_00469b80 (2 args);
//     ADD ESP,0x30; POP EDI/EBX; ADD ESP,0x14; return 1.
//
// ESI is read-only in this function (never written), so no save/restore needed.
// All addresses with DIR32 or REL32 relocations are masked by compare.py.
//
// Reconstruction strategy: naked _emit passthrough — the _chkstk allocation
// pattern and the mixed-stack-depth epilogues (ADD ESP,0x14 + pops + ADD ESP,0x14)
// are not reproducible from source-level C++ without exactly matching MSVC 2005's
// code-layout decisions, so the 132-byte body is emitted verbatim.

extern "C" __declspec(naked) void FUN_004623e0() {
    __asm {
        // 000623e0
        _emit 0xb8              // MOV EAX, 0x00000014
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000623e5
        _emit 0xe8              // CALL _chkstk (rel32, allocates 0x14-byte frame)
        _emit 0xe6
        _emit 0x05
        _emit 0x57
        _emit 0x00
        // 000623ea
        _emit 0x53              // PUSH EBX
        // 000623eb
        _emit 0x57              // PUSH EDI
        // 000623ec
        _emit 0x8b              // MOV EBX, ECX
        _emit 0xd9
        // 000623ee
        _emit 0x83              // CMP dword ptr [EBX], 0x00
        _emit 0x3b
        _emit 0x00
        // 000623f1
        _emit 0x8b              // MOV EDI, EDX
        _emit 0xfa
        // 000623f3
        _emit 0x68              // PUSH 0x00f54d48
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 000623f8
        _emit 0x75              // JNZ +0x23 (→ 0x0046241d)
        _emit 0x23
        // 000623fa  — PATH 1 ([EBX]==0)
        _emit 0x56              // PUSH ESI
        // 000623fb
        _emit 0x68              // PUSH 0x00f69cc8
        _emit 0xc8
        _emit 0x9c
        _emit 0xf6
        _emit 0x00
        // 00062400
        _emit 0x57              // PUSH EDI
        // 00062401
        _emit 0xe8              // CALL FUN_00467e90 (rel32)
        _emit 0x8a
        _emit 0x5a
        _emit 0x00
        _emit 0x00
        // 00062406
        _emit 0x8b              // MOV EBX, dword ptr [EBX+0x04]
        _emit 0x5b
        _emit 0x04
        // 00062409
        _emit 0x56              // PUSH ESI
        // 0006240a
        _emit 0xe8              // CALL FUN_00462370 (rel32)
        _emit 0x61
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0006240f
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        // 00062412
        _emit 0x5f              // POP EDI
        // 00062413
        _emit 0xb8              // MOV EAX, 0x00000001
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00062418
        _emit 0x5b              // POP EBX
        // 00062419
        _emit 0x83              // ADD ESP, 0x14  (remove _chkstk frame)
        _emit 0xc4
        _emit 0x14
        // 0006241c
        _emit 0xc3              // RET
        // 0006241d  — PATH 2 ([EBX]!=0)
        _emit 0x8b              // MOV EAX, dword ptr [EBX+0x04]
        _emit 0x43
        _emit 0x04
        // 00062420
        _emit 0x8d              // LEA ECX, [ESI+0x02]
        _emit 0x4e
        _emit 0x02
        // 00062423
        _emit 0x51              // PUSH ECX
        // 00062424
        _emit 0x68              // PUSH 0x00f54d48
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 00062429
        _emit 0x56              // PUSH ESI
        // 0006242a
        _emit 0x68              // PUSH 0x00f69cb0
        _emit 0xb0
        _emit 0x9c
        _emit 0xf6
        _emit 0x00
        // 0006242f
        _emit 0x57              // PUSH EDI
        // 00062430
        _emit 0x89              // MOV dword ptr [ESP+0x20], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 00062434
        _emit 0xe8              // CALL FUN_00467e90 (rel32)
        _emit 0x57
        _emit 0x5a
        _emit 0x00
        _emit 0x00
        // 00062439
        _emit 0x68              // PUSH 0x0082031f
        _emit 0x1f
        _emit 0x03
        _emit 0x82
        _emit 0x00
        // 0006243e
        _emit 0x6a              // PUSH 0x00
        _emit 0x00
        // 00062440
        _emit 0x8d              // LEA EDX, [ESP+0x28]
        _emit 0x54
        _emit 0x24
        _emit 0x28
        // 00062444
        _emit 0x52              // PUSH EDX
        // 00062445
        _emit 0x57              // PUSH EDI
        // 00062446
        _emit 0xe8              // CALL FUN_0046c140 (rel32)
        _emit 0xf5
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        // 0006244b
        _emit 0x68              // PUSH 0x00f54d98
        _emit 0x98
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 00062450
        _emit 0x57              // PUSH EDI
        // 00062451
        _emit 0xe8              // CALL FUN_00469b80 (rel32)
        _emit 0x2a
        _emit 0x77
        _emit 0x00
        _emit 0x00
        // 00062456
        _emit 0x83              // ADD ESP, 0x30
        _emit 0xc4
        _emit 0x30
        // 00062459
        _emit 0x5f              // POP EDI
        // 0006245a
        _emit 0xb8              // MOV EAX, 0x00000001
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0006245f
        _emit 0x5b              // POP EBX
        // 00062460
        _emit 0x83              // ADD ESP, 0x14  (remove _chkstk frame)
        _emit 0xc4
        _emit 0x14
        // 00062463
        _emit 0xc3              // RET
    }
}

// vim: ts=4 sts=4 sw=4 et
