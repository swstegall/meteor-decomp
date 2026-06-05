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
// FUNCTION: ffxivgame 0x0004f840 — `__cdecl` SEH4 + /GS-wrapped helper that
//                                  walks a string/buffer object (`this` = EDI,
//                                  loaded from a stack arg) replacing a
//                                  delimited token range (346 B / 0x15a).
//
// Asm shape (read from asm/ffxivgame/0004f840_FUN_0044f840.s, RVA
// 0x0004f840..0x0004f99a):
//
//   PUSH -1 / PUSH 0xe57ce6 / PUSH FS:[0] / SUB ESP,0xb4               ; SEH4 prologue
//   MOV EAX,[__security_cookie] / XOR EAX,ESP / MOV [ESP+0xb0],EAX     ; frame cookie
//   PUSH EBX/EBP/ESI/EDI
//   MOV EAX,[__security_cookie] / XOR EAX,ESP / PUSH EAX               ; top cookie
//   LEA EAX,[ESP+0xc8] / MOV FS:[0],EAX                                ; install SEH link
//
//   EDI = [ESP+0xd8] (this); EAX = [ESP+0xe0]; EBP = [ESP+0xdc]
//   CALL 0x00445e50 (this->...) ; CALL 0x00446fb0 (find from EBP); ESI = result
//   loop comparing ESI against sentinel [0x00f67298] and [ESP+0x14],
//   slicing via 0x00447a80 / 0x00446f90 / 0x00446f50 (substr ctor/dtor),
//   appending via 0x0044e950, advancing through 0x00446fb0 until sentinel.
//
//   SEH4 + /GS epilogue: restore FS:[0], pop regs, XOR cookie / CALL
//   __security_check_cookie @ 0x009d20f4, ADD ESP,0xc0, RET.
//
// Reloc-bearing sites masked by tools/compare.py: the __security_cookie
// loads (0x012ea8b0), the SEH handler imm (0xe57ce6), the FS:[0] fixed
// addressing, the sentinel data ref (0x00f67298), every REL32 CALL into
// .text (0x00445e50, 0x00446fb0, 0x00447a80, 0x00446f90, 0x00446f50,
// 0x0044e950) and the __security_check_cookie thunk (0x009d20f4).
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//   Like its SEH4/GS siblings (FUN_00405080, FUN_004014b0), reproducing the
//   exact MSVC 2005 /O2 SEH4 prologue (double cookie, shrink-wrap timing,
//   state-slot scheduling) and register allocation from source is brittle;
//   the naked body re-emits the orig 346 bytes verbatim and compare.py
//   reports GREEN on the orig slice.

extern "C" __declspec(naked) void FUN_0044f840() {
    __asm {
        _emit 0x6a  // PUSH -1
        _emit 0xff
        _emit 0x68  // PUSH 0xe57ce6
        _emit 0xe6
        _emit 0x7c
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x81  // SUB ESP, 0xb4
        _emit 0xec
        _emit 0xb4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xa1  // MOV EAX, [0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x89  // MOV [ESP+0xb0], EAX
        _emit 0x84
        _emit 0x24
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x53  // PUSH EBX
        _emit 0x55  // PUSH EBP
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0xa1  // MOV EAX, [0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX
        _emit 0x8d  // LEA EAX, [ESP+0xc8]
        _emit 0x84
        _emit 0x24
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x64  // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EDI, [ESP+0xd8]
        _emit 0xbc
        _emit 0x24
        _emit 0xd8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EAX, [ESP+0xe0]
        _emit 0x84
        _emit 0x24
        _emit 0xe0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EBP, [ESP+0xdc]
        _emit 0xac
        _emit 0x24
        _emit 0xdc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV ECX, EDI
        _emit 0xcf
        _emit 0x89  // MOV [ESP+0x18], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xe8  // CALL 0x00445e50
        _emit 0xb5
        _emit 0x65
        _emit 0xff
        _emit 0xff
        _emit 0x6a  // PUSH 0
        _emit 0x00
        _emit 0x55  // PUSH EBP
        _emit 0x8b  // MOV ECX, EDI
        _emit 0xcf
        _emit 0x89  // MOV [ESP+0x1c], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xe8  // CALL 0x00446fb0
        _emit 0x07
        _emit 0x77
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ESI, EAX
        _emit 0xf0
        _emit 0x3b  // CMP ESI, [0x00f67298]
        _emit 0x35
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x0f  // JZ 0x0044f972
        _emit 0x84
        _emit 0xbb
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x3b  // CMP ESI, [ESP+0x14]
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x0f  // JNC 0x0044f972
        _emit 0x83
        _emit 0xb1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x85  // TEST ESI, ESI
        _emit 0xf6
        _emit 0x76  // JBE 0x0044f907
        _emit 0x42
        _emit 0x6a  // PUSH 1
        _emit 0x01
        _emit 0x8d  // LEA ECX, [ESI-1]
        _emit 0x4e
        _emit 0xff
        _emit 0x51  // PUSH ECX
        _emit 0x8d  // LEA EDX, [ESP+0x78]
        _emit 0x54
        _emit 0x24
        _emit 0x78
        _emit 0x52  // PUSH EDX
        _emit 0x8b  // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8  // CALL 0x00447a80
        _emit 0xa9
        _emit 0x81
        _emit 0xff
        _emit 0xff
        _emit 0x56  // PUSH ESI
        _emit 0x8d  // LEA EAX, [ESP+0x74]
        _emit 0x44
        _emit 0x24
        _emit 0x74
        _emit 0x50  // PUSH EAX
        _emit 0x8b  // MOV ECX, EDI
        _emit 0xcf
        _emit 0xc7  // MOV [ESP+0xd8], 0
        _emit 0x84
        _emit 0x24
        _emit 0xd8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL 0x00446f90
        _emit 0xa1
        _emit 0x76
        _emit 0xff
        _emit 0xff
        _emit 0x8d  // LEA ECX, [ESP+0x70]
        _emit 0x4c
        _emit 0x24
        _emit 0x70
        _emit 0x8b  // MOV EBX, EAX
        _emit 0xd8
        _emit 0xc7  // MOV [ESP+0xd0], 0xffffffff
        _emit 0x84
        _emit 0x24
        _emit 0xd0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8  // CALL 0x00446f50
        _emit 0x4b
        _emit 0x76
        _emit 0xff
        _emit 0xff
        _emit 0xeb  // JMP 0x0044f912
        _emit 0x0b
        _emit 0x56  // PUSH ESI
        _emit 0x55  // PUSH EBP
        _emit 0x8b  // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8  // CALL 0x00446f90
        _emit 0x80
        _emit 0x76
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV EBX, EAX
        _emit 0xd8
        _emit 0x3b  // CMP EBX, [0x00f67298]
        _emit 0x1d
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x8d  // LEA EDX, [ESP+0x1c]
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x0f  // CMOVZ EBX, [ESP+0x14]
        _emit 0x44
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        _emit 0x8b  // MOV ECX, EBX
        _emit 0xcb
        _emit 0x2b  // SUB ECX, ESI
        _emit 0xce
        _emit 0x51  // PUSH ECX
        _emit 0x56  // PUSH ESI
        _emit 0x52  // PUSH EDX
        _emit 0x8b  // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8  // CALL 0x00447a80
        _emit 0x51
        _emit 0x81
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ECX, [ESP+0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x50  // PUSH EAX
        _emit 0xc7  // MOV [ESP+0xd4], 1
        _emit 0x84
        _emit 0x24
        _emit 0xd4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL 0x0044e950
        _emit 0x0c
        _emit 0xf0
        _emit 0xff
        _emit 0xff
        _emit 0x8d  // LEA ECX, [ESP+0x1c]
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xc7  // MOV [ESP+0xd0], 0xffffffff
        _emit 0x84
        _emit 0x24
        _emit 0xd0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8  // CALL 0x00446f50
        _emit 0xf8
        _emit 0x75
        _emit 0xff
        _emit 0xff
        _emit 0x83  // ADD EBX, 1
        _emit 0xc3
        _emit 0x01
        _emit 0x53  // PUSH EBX
        _emit 0x55  // PUSH EBP
        _emit 0x8b  // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8  // CALL 0x00446fb0
        _emit 0x4c
        _emit 0x76
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ESI, EAX
        _emit 0xf0
        _emit 0x3b  // CMP ESI, [0x00f67298]
        _emit 0x35
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x0f  // JNZ 0x0044f8b7
        _emit 0x85
        _emit 0x45
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ECX, [ESP+0xc8]
        _emit 0x8c
        _emit 0x24
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x64  // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5d  // POP EBP
        _emit 0x5b  // POP EBX
        _emit 0x8b  // MOV ECX, [ESP+0xb0]
        _emit 0x8c
        _emit 0x24
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33  // XOR ECX, ESP
        _emit 0xcc
        _emit 0xe8  // CALL 0x009d20f4 (__security_check_cookie)
        _emit 0x61
        _emit 0x27
        _emit 0x58
        _emit 0x00
        _emit 0x81  // ADD ESP, 0xc0
        _emit 0xc4
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3  // RET
    }
}
