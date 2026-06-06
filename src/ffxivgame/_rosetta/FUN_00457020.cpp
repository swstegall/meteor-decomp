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
// FUNCTION: ffxivgame 0x00457020 — multi-block stack builder forwarding to
//                                  FUN_00456f70; 110 bytes.
//
// __cdecl void* FUN_00457020(arg1, arg2, ..., arg10)
//
//   Allocates three 12-byte (3-dword) descriptors on the stack, populates
//   them with argument pairs drawn from the caller's frame, then calls
//   FUN_00456f70 with the innermost argument (arg1, in ESI) pushed as a
//   further parameter and returns arg1 unchanged (MOV EAX, ESI before RET).
//
//   Stack layout at entry (esp0):
//     [esp0+00]: return addr
//     [esp0+04]: arg1   ← returned in EAX; also pushed as last arg to callee
//     [esp0+08]: arg2
//     [esp0+0C]: arg3
//     [esp0+10]: arg4
//     [esp0+14]: arg5
//     [esp0+18]: arg6
//     [esp0+1C]: arg7
//     [esp0+20]: arg8
//     [esp0+24]: arg9
//     [esp0+28]: arg10
//
//   Block layout passed to FUN_00456f70 (innermost → outermost):
//     block3: {0, arg3, arg4}
//     block2: {0, arg6, arg7}
//     block1: {0, arg9, arg10}
//     then: arg1 (PUSH ECX from 0x00457038)
//     then: 0    (PUSH EAX from 0x00457037, the zero local)
//
//   Calling convention: __cdecl (caller-cleans, RET with no immediate).
//
//   Reloc-bearing site (the CALL rel32 is baked into the orig binary at
//   the orig VA and emitted verbatim so compare.py sees a zero-reloc
//   passthrough — same technique used by all _rosetta siblings with
//   intra-binary calls):
//     +0x61  CALL rel32  → FUN_00456f70 (RVA 0x00056f70)

extern "C" __declspec(naked) void FUN_00457020() {
    __asm {
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x08]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x28]
        _emit 0x54
        _emit 0x24
        _emit 0x28
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x0C]
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0xc6              // MOV byte ptr [ESP+0x04], 0x00
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x04]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x38]
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0x83              // SUB ESP, 0x0C
        _emit 0xec
        _emit 0x0c
        _emit 0x8b              // MOV EAX, ESP
        _emit 0xc4
        _emit 0x89              // MOV dword ptr [EAX+0x04], EDX
        _emit 0x50
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x34]
        _emit 0x54
        _emit 0x24
        _emit 0x34
        _emit 0x89              // MOV dword ptr [EAX+0x08], ECX
        _emit 0x48
        _emit 0x08
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x38]
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0xc7              // MOV dword ptr [EAX], 0x00000000
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // SUB ESP, 0x0C
        _emit 0xec
        _emit 0x0c
        _emit 0x8b              // MOV EAX, ESP
        _emit 0xc4
        _emit 0x89              // MOV dword ptr [EAX+0x04], EDX
        _emit 0x50
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x34]
        _emit 0x54
        _emit 0x24
        _emit 0x34
        _emit 0x89              // MOV dword ptr [EAX+0x08], ECX
        _emit 0x48
        _emit 0x08
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x38]
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0xc7              // MOV dword ptr [EAX], 0x00000000
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // SUB ESP, 0x0C
        _emit 0xec
        _emit 0x0c
        _emit 0x8b              // MOV EAX, ESP
        _emit 0xc4
        _emit 0x56              // PUSH ESI
        _emit 0xc7              // MOV dword ptr [EAX], 0x00000000
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV dword ptr [EAX+0x04], EDX
        _emit 0x50
        _emit 0x04
        _emit 0x89              // MOV dword ptr [EAX+0x08], ECX
        _emit 0x48
        _emit 0x08
        _emit 0xe8              // CALL FUN_00456f70 (rel32 → 0xfffffe ea)
        _emit 0xea
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x30
        _emit 0xc4
        _emit 0x30
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x59              // POP ECX
        _emit 0xc3              // RET
    }
}
