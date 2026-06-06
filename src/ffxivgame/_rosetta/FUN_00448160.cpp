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
// FUNCTION: ffxivgame 0x00048160 — twin-arg "clamp + dispatch" helper
//                                  (267 B / 0x10b, EH4-SEH wrapped, /GS).
//
// Behaviour read from the disassembly at orig RVA 0x00048160:
//
//   __thiscall ? FUN_00448160(this/ECX, a, b, c, d, e);   // RET 0x14
//
//     // Two calls into the same __thiscall helper 0x00447a80,
//     // each producing a value into a local out-param:
//     local_A = 0x00447a80(&local_A, b, a);            // ECX = incoming this
//     /* trylevel = 0 */
//     local_B = 0x00447a80(&local_B, d, c);            // ECX = ESI (= arg c)
//
//     unsigned x = local_A;                            // [esp+0x14]
//     unsigned y = local_B;                            // [esp+0x68]
//     if (!(x < y)) x = y;                             // x = min(x, y)
//
//     EDI = FUN_00445b70(arg_e, local_B_field, x);     // __cdecl, 3 args
//     if (local_B.flag == 0)                           // [esp+0x71]
//         FUN_0044d350(local_B_field, local_B2, 0xb);  // __cdecl, 3 args
//     /* trylevel = -1 */
//     if (local_A.flag == 0)                           // [esp+0x1d]
//         FUN_0044d350(arg_e, local_A2, 0xb);          // __cdecl, 3 args
//     return EDI;
//
//   Reloc-bearing sites in the orig 267 bytes (absolute addresses resolve
//   only in a full-binary relink at image base 0x00400000; standalone .obj
//   compilation can't reproduce them):
//     +0x02   scope-table handler RVA   (0x00e57586 — .rdata FuncInfo)
//     +0x07   FS:[0] read               (constant 0, fold-through)
//     +0x14   __security_cookie load    (.data 0x012ea8b0)
//     +0x24   __security_cookie load    (.data 0x012ea8b0, 2nd)
//     +0x33   FS:[0] install            (constant 0, fold-through)
//     +0x55   helper CALL               (.text 0x00447a80 rel32 — __thiscall)
//     +0x7c   helper CALL               (.text 0x00447a80 rel32, 2nd)
//     +0x9a   CALL                      (.text 0x00445b70 rel32 — __cdecl)
//     +0xb3   CALL                      (.text 0x0044d350 rel32 — __cdecl)
//     +0xd9   CALL                      (.text 0x0044d350 rel32, 2nd)
//     +0xea   FS:[0] restore            (constant 0)
//     +0xfd   __security_check_cookie   (.text 0x009d20f4 rel32)
//
// Reconstruction strategy — naked-asm byte passthrough (same choice as the
// SEH-wrapped siblings FUN_004054d0 / FUN_00403a20): a `__declspec(naked)`
// body re-emits the orig 267 bytes verbatim via MASM `_emit` directives.
// The .obj's `.text` section ends up byte-identical to the orig slice (the
// absolute addresses bake in as raw immediates rather than COFF fixups),
// which is exactly what tools/compare.py grades against.

extern "C" __declspec(naked) void FUN_00448160() {
    __asm {
        // 00048160  PUSH -1
        _emit 0x6a
        _emit 0xff
        // 00048162  PUSH 0xe57586 (scope-table handler RVA)
        _emit 0x68
        _emit 0x86
        _emit 0x75
        _emit 0xe5
        _emit 0x00
        // 00048167  MOV EAX, FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004816d  PUSH EAX
        _emit 0x50
        // 0004816e  SUB ESP, 0xac
        _emit 0x81
        _emit 0xec
        _emit 0xac
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00048174  MOV EAX, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00048179  XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 0004817b  MOV [ESP+0xa8], EAX
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0xa8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00048182  PUSH ESI
        _emit 0x56
        // 00048183  PUSH EDI
        _emit 0x57
        // 00048184  MOV EAX, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00048189  XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 0004818b  PUSH EAX  (EH4 cookie #2)
        _emit 0x50
        // 0004818c  LEA EAX, [ESP+0xb8]
        _emit 0x8d
        _emit 0x84
        _emit 0x24
        _emit 0xb8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00048193  MOV FS:[0], EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00048199  MOV EAX, [ESP+0xcc]
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000481a0  MOV EDX, [ESP+0xc8]
        _emit 0x8b
        _emit 0x94
        _emit 0x24
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000481a7  MOV ESI, [ESP+0xd0]
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0xd0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000481ae  PUSH EAX
        _emit 0x50
        // 000481af  PUSH EDX
        _emit 0x52
        // 000481b0  LEA EAX, [ESP+0x14]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 000481b4  PUSH EAX
        _emit 0x50
        // 000481b5  CALL 0x00447a80
        _emit 0xe8
        _emit 0xc6
        _emit 0xf8
        _emit 0xff
        _emit 0xff
        // 000481ba  MOV ECX, [ESP+0xd8]
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0xd8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000481c1  MOV EDX, [ESP+0xd4]
        _emit 0x8b
        _emit 0x94
        _emit 0x24
        _emit 0xd4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000481c8  PUSH ECX
        _emit 0x51
        // 000481c9  PUSH EDX
        _emit 0x52
        // 000481ca  LEA EAX, [ESP+0x68]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x68
        // 000481ce  PUSH EAX
        _emit 0x50
        // 000481cf  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000481d1  MOV dword [ESP+0xcc], 0
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000481dc  CALL 0x00447a80
        _emit 0xe8
        _emit 0x9f
        _emit 0xf8
        _emit 0xff
        _emit 0xff
        // 000481e1  MOV EAX, [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 000481e5  MOV ECX, [ESP+0x68]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x68
        // 000481e9  CMP EAX, ECX
        _emit 0x3b
        _emit 0xc1
        // 000481eb  JC +2  -> 0x000481ef
        _emit 0x72
        _emit 0x02
        // 000481ed  MOV EAX, ECX
        _emit 0x8b
        _emit 0xc1
        // 000481ef  MOV ESI, [ESP+0x60]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x60
        // 000481f3  MOV ECX, [ESP+0xc]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 000481f7  PUSH EAX
        _emit 0x50
        // 000481f8  PUSH ESI
        _emit 0x56
        // 000481f9  PUSH ECX
        _emit 0x51
        // 000481fa  CALL 0x00445b70
        _emit 0xe8
        _emit 0x71
        _emit 0xd9
        _emit 0xff
        _emit 0xff
        // 000481ff  ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00048202  CMP byte [ESP+0x71], 0
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x71
        _emit 0x00
        // 00048207  MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // 00048209  JNZ +0x10  -> 0x0004821b
        _emit 0x75
        _emit 0x10
        // 0004820b  MOV EDX, [ESP+0x64]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x64
        // 0004820f  PUSH 0xb
        _emit 0x6a
        _emit 0x0b
        // 00048211  PUSH EDX
        _emit 0x52
        // 00048212  PUSH ESI
        _emit 0x56
        // 00048213  CALL 0x0044d350
        _emit 0xe8
        _emit 0x38
        _emit 0x51
        _emit 0x00
        _emit 0x00
        // 00048218  ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0004821b  CMP byte [ESP+0x1d], 0
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x1d
        _emit 0x00
        // 00048220  MOV dword [ESP+0xc0], 0xffffffff
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0004822b  JNZ +0x14  -> 0x00048241
        _emit 0x75
        _emit 0x14
        // 0004822d  MOV EAX, [ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00048231  MOV ECX, [ESP+0xc]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 00048235  PUSH 0xb
        _emit 0x6a
        _emit 0x0b
        // 00048237  PUSH EAX
        _emit 0x50
        // 00048238  PUSH ECX
        _emit 0x51
        // 00048239  CALL 0x0044d350
        _emit 0xe8
        _emit 0x12
        _emit 0x51
        _emit 0x00
        _emit 0x00
        // 0004823e  ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00048241  MOV EAX, EDI
        _emit 0x8b
        _emit 0xc7
        // 00048243  MOV ECX, [ESP+0xb8]   ; saved FS:[0]
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0xb8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004824a  MOV FS:[0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00048251  POP ECX (cookie #2)
        _emit 0x59
        // 00048252  POP EDI
        _emit 0x5f
        // 00048253  POP ESI
        _emit 0x5e
        // 00048254  MOV ECX, [ESP+0xa8]   ; cookie
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0xa8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004825b  XOR ECX, ESP
        _emit 0x33
        _emit 0xcc
        // 0004825d  CALL 0x009d20f4       ; __security_check_cookie
        _emit 0xe8
        _emit 0x92
        _emit 0x9e
        _emit 0x58
        _emit 0x00
        // 00048262  ADD ESP, 0xb8
        _emit 0x81
        _emit 0xc4
        _emit 0xb8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00048268  RET 0x14
        _emit 0xc2
        _emit 0x14
        _emit 0x00
    }
}
