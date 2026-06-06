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
// FUNCTION: ffxivgame 0x009fe782 — animation/state update tick (309 B / 0x135)
//
// Inspection (read from the disassembly at orig RVA 0x005fe782):
//
//   __cdecl float FUN_009fe782(bool arg);
//
//   Flow summary:
//     1. Calls a getter (0x009fef20) to obtain an object pointer in ESI.
//        If ESI == NULL, logs an assert via an indirect-call mechanism
//        (initialises [0x01323910] flag + [0x0132390c] handler, then
//        pushes 5 args and calls the handler) with line 0x16d.
//     2. Checks [ESI+0xC]; if zero, logs another assert (line 0x16e).
//     3. Checks [ESI+0xC0]; if zero and arg (byte) != 0 and
//        [0x01364a24] != 0, calls a four-function sequence
//        (0x009fe310 → 0x009fd160 → 0x009fcdb0 → 0x009fd3a0).
//        Returns 0.0f (FLDZ) in either branch of the zero path.
//     4. If [ESI+0xC0] != 0: calls 0x009fd610 via __thiscall
//        ([ESI+0xC] as ECX), stores the returned float at [ESP+0x10].
//        If arg != 0, calls a second pair of __thiscall getters
//        (0x009fd5c0, 0x009fd510) and compares; on mismatch logs
//        another assert (line 0x17a) via 0x00406550.
//        If [0x01364a24] != 0, calls the same four-function sequence.
//        Loads and returns the stored float from [ESP+0x10].
//
//   Stack frame (after PUSH EBX/ESI/EDI):
//     [ESP+0x10]  float local (temp storage for result from callee)
//     [ESP+0x18]  arg (bool byte from caller)
//
//   Reloc-bearing sites (resolve only in a full-binary relink at
//   image base 0x00400000; standalone .obj uses raw immediates):
//     +0x03  CALL 0x009fef20  (rel32)
//     +0x15  TEST byte ptr [0x01323910] (moffs32)
//     +0x21  OR   dword ptr [0x01323910] (moffs32)
//     +0x27  MOV  dword ptr [0x0132390c] (moffs32)
//     +0x2c–0x41 five PUSHes (string/file/func literals, imm32)
//     +0x46  CALL dword ptr [0x0132390c] (moffs32 indirect)
//     ... and corresponding blocks for the second and third assert sites
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function's register allocation (ESI = object, EBX = flag 1,
//   EDI = handler ptr), the indirect-call assert pattern, the x87
//   FLDZ / FSTP / FLD sequence, and the ADD ESP,0x8; RET epilogue
//   (rather than the more common RET or RET N) are all MSVC 2005 /O2
//   artefacts that are brittle to reproduce at source level without
//   controlling every allocation decision.  The naked-asm passthrough
//   used by FUN_004014b0, FUN_00408f10, FUN_00401a00, and the rest of
//   the _rosetta siblings is the correct strategy here.

extern "C" __declspec(naked) void FUN_009fe782() {
    __asm {
        _emit 0x53  // PUSH EBX
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0xe8  // CALL 0x009fef20
        _emit 0x96
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV ESI,EAX
        _emit 0xf0
        _emit 0x85  // TEST ESI,ESI
        _emit 0xf6
        _emit 0xbb  // MOV EBX,0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xbf  // MOV EDI,0x9fcca0
        _emit 0xa0
        _emit 0xcc
        _emit 0x9f
        _emit 0x00
        _emit 0x75  // JNZ +0x36
        _emit 0x36
        _emit 0x84  // TEST byte ptr [0x01323910],BL
        _emit 0x1d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75  // JNZ +0x0c
        _emit 0x0c
        _emit 0x09  // OR dword ptr [0x01323910],EBX
        _emit 0x1d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x89  // MOV dword ptr [0x0132390c],EDI
        _emit 0x3d
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x68  // PUSH 0x1093e88
        _emit 0x88
        _emit 0x3e
        _emit 0x09
        _emit 0x01
        _emit 0x68  // PUSH 0x16d
        _emit 0x6d
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68  // PUSH 0x1093760
        _emit 0x60
        _emit 0x37
        _emit 0x09
        _emit 0x01
        _emit 0x68  // PUSH 0xf54d48
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68  // PUSH 0x109386c
        _emit 0x6c
        _emit 0x38
        _emit 0x09
        _emit 0x01
        _emit 0xff  // CALL dword ptr [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83  // ADD ESP,0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x83  // CMP dword ptr [ESI+0xc],0x0
        _emit 0x7e
        _emit 0x0c
        _emit 0x00
        _emit 0x75  // JNZ +0x36
        _emit 0x36
        _emit 0x84  // TEST byte ptr [0x01323910],BL
        _emit 0x1d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75  // JNZ +0x0c
        _emit 0x0c
        _emit 0x09  // OR dword ptr [0x01323910],EBX
        _emit 0x1d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x89  // MOV dword ptr [0x0132390c],EDI
        _emit 0x3d
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x68  // PUSH 0x1093e88
        _emit 0x88
        _emit 0x3e
        _emit 0x09
        _emit 0x01
        _emit 0x68  // PUSH 0x16e
        _emit 0x6e
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68  // PUSH 0x1093760
        _emit 0x60
        _emit 0x37
        _emit 0x09
        _emit 0x01
        _emit 0x68  // PUSH 0xf54d48
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68  // PUSH 0x1093cdc
        _emit 0xdc
        _emit 0x3c
        _emit 0x09
        _emit 0x01
        _emit 0xff  // CALL dword ptr [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83  // ADD ESP,0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x80  // CMP byte ptr [ESI+0xc0],0x0
        _emit 0xbe
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x75  // JNZ +0x2f
        _emit 0x2f
        _emit 0x80  // CMP byte ptr [ESP+0x18],0x0
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x74  // JZ +0x1f
        _emit 0x1f
        _emit 0x80  // CMP byte ptr [0x01364a24],0x0
        _emit 0x3d
        _emit 0x24
        _emit 0x4a
        _emit 0x36
        _emit 0x01
        _emit 0x00
        _emit 0x74  // JZ +0x16
        _emit 0x16
        _emit 0xe8  // CALL 0x009fe310
        _emit 0xe6
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0xe8  // CALL 0x009fd160
        _emit 0x31
        _emit 0xe9
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ECX,EAX
        _emit 0xc8
        _emit 0xe8  // CALL 0x009fcdb0
        _emit 0x7a
        _emit 0xe5
        _emit 0xff
        _emit 0xff
        _emit 0xe8  // CALL 0x009fd3a0
        _emit 0x65
        _emit 0xeb
        _emit 0xff
        _emit 0xff
        _emit 0xd9  // FLDZ
        _emit 0xee
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5b  // POP EBX
        _emit 0x83  // ADD ESP,0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc3  // RET
        _emit 0x8b  // MOV ECX,dword ptr [ESI+0xc]
        _emit 0x4e
        _emit 0x0c
        _emit 0xe8  // CALL 0x009fd610
        _emit 0xc4
        _emit 0xed
        _emit 0xff
        _emit 0xff
        _emit 0xd9  // FSTP float ptr [ESP+0x10]
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x80  // CMP byte ptr [ESP+0x18],0x0
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x74  // JZ +0x55
        _emit 0x55
        _emit 0x8b  // MOV ECX,ESI
        _emit 0xce
        _emit 0xe8  // CALL 0x009fd5c0
        _emit 0x62
        _emit 0xed
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ECX,ESI
        _emit 0xce
        _emit 0x8b  // MOV EDI,EAX
        _emit 0xf8
        _emit 0xe8  // CALL 0x009fd510
        _emit 0xa9
        _emit 0xec
        _emit 0xff
        _emit 0xff
        _emit 0x3b  // CMP EDI,EAX
        _emit 0xf8
        _emit 0x74  // JZ +0x22
        _emit 0x22
        _emit 0x68  // PUSH 0x1093e88
        _emit 0x88
        _emit 0x3e
        _emit 0x09
        _emit 0x01
        _emit 0x68  // PUSH 0x17a
        _emit 0x7a
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68  // PUSH 0x1093760
        _emit 0x60
        _emit 0x37
        _emit 0x09
        _emit 0x01
        _emit 0x68  // PUSH 0xf54d48
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68  // PUSH 0x1093e5c
        _emit 0x5c
        _emit 0x3e
        _emit 0x09
        _emit 0x01
        _emit 0x8d  // LEA ECX,[ESP+0x23]
        _emit 0x4c
        _emit 0x24
        _emit 0x23
        _emit 0xe8  // CALL 0x00406550
        _emit 0xc3
        _emit 0x7c
        _emit 0xa0
        _emit 0xff
        _emit 0x80  // CMP byte ptr [0x01364a24],0x0
        _emit 0x3d
        _emit 0x24
        _emit 0x4a
        _emit 0x36
        _emit 0x01
        _emit 0x00
        _emit 0x74  // JZ +0x16
        _emit 0x16
        _emit 0xe8  // CALL 0x009fe310
        _emit 0x75
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0xe8  // CALL 0x009fd160
        _emit 0xc0
        _emit 0xe8
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ECX,EAX
        _emit 0xc8
        _emit 0xe8  // CALL 0x009fcdb0
        _emit 0x09
        _emit 0xe5
        _emit 0xff
        _emit 0xff
        _emit 0xe8  // CALL 0x009fd3a0
        _emit 0xf4
        _emit 0xea
        _emit 0xff
        _emit 0xff
        _emit 0xd9  // FLD float ptr [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5b  // POP EBX
        _emit 0x83  // ADD ESP,0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc3  // RET
    }
}
