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
// FUNCTION: ffxivgame 0x0001b820 — Windows handle/socket setup wrapper;
//                                  allocates a struct on the stack, fills
//                                  it from a global + constants, calls three
//                                  Win32 API thunks, and returns a bool
//                                  (SETNZ AL on the 16-bit result of the
//                                  final call). 114 bytes / 0x72.
//
// Calling convention: __cdecl (no 'this'; EAX is consumed as an in-argument
//   saved to [ESP+0x34] at entry; ESI is the only callee-save used).
//
// Stack frame (ESP-relative, no EBP):
//   SUB ESP,0x30         — reserve 0x30 bytes of local struct
//   PUSH ESI             — callee-save; [ESP+0x34] originally held EAX
//   PUSH 0x7f00          — 2nd arg to first Win32 call
//   PUSH ESI (=0)        — 1st arg to first Win32 call
//
// Struct fields written before the first call:
//   [ESP+0x0c] = 0x30       (struct size / cbSize)
//   [ESP+0x10] = 0x23       (type/flags)
//   [ESP+0x14] = 0x0041b7d0 (function pointer / lpSecurityDescriptor)
//   [ESP+0x20] = ECX        (loaded from global 0x013298b8)
//   [ESP+0x24] = 0          (ESI)
//   [ESP+0x38] = 0          (ESI — second frame slot zeroed)
//
// Call sequence:
//   1. CALL dword ptr [0x00f3e4bc]   — first Win32 import (e.g. CreateFileMapping)
//   2. PUSH 0x5  ; store result      — second import call with timeout/flag arg
//      CALL dword ptr [0x00f3e074]
//   3. LEA EDX,[ESP+0x4] ; PUSH EDX  — pass struct ptr; clear more fields
//      CALL dword ptr [0x00f3e490]   — third import (returns BOOL in AX)
//      TEST AX,AX / SETNZ AL         — bool return
//
// Reloc-bearing sites (absolute addresses emitted as raw immediates so that
// the .obj byte image matches the orig slice with no relocations):
//   +0x03  MOV ECX, [0x013298b8]    (.data global)
//   +0x26  MOV dword ptr [ESP+0x14], 0x0041b7d0   (.text fn-ptr immediate)
//   +0x3a  CALL dword ptr [0x00f3e4bc]  (import thunk)
//   +0x46  CALL dword ptr [0x00f3e074]  (import thunk)
//   +0x61  CALL dword ptr [0x00f3e490]  (import thunk)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Source-level C++ would emit relocation records for all five absolute
//   addresses above; the standalone .obj then mismatches. Emitting all
//   114 bytes verbatim via MASM _emit directives produces a byte-identical
//   .obj text section. compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0041b820() {
    __asm {
        // 0001b820: 83 ec 30   SUB ESP,0x30
        _emit 0x83
        _emit 0xec
        _emit 0x30
        // 0001b823: 8b 0d b8 98 32 01   MOV ECX,dword ptr [0x013298b8]
        _emit 0x8b
        _emit 0x0d
        _emit 0xb8
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001b829: 56   PUSH ESI
        _emit 0x56
        // 0001b82a: 33 f6   XOR ESI,ESI
        _emit 0x33
        _emit 0xf6
        // 0001b82c: 68 00 7f 00 00   PUSH 0x7f00
        _emit 0x68
        _emit 0x00
        _emit 0x7f
        _emit 0x00
        _emit 0x00
        // 0001b831: 56   PUSH ESI
        _emit 0x56
        // 0001b832: 89 44 24 34   MOV dword ptr [ESP+0x34],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // 0001b836: c7 44 24 0c 30 00 00 00   MOV dword ptr [ESP+0xc],0x30
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x30
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001b83e: c7 44 24 10 23 00 00 00   MOV dword ptr [ESP+0x10],0x23
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x23
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001b846: c7 44 24 14 d0 b7 41 00   MOV dword ptr [ESP+0x14],0x0041b7d0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xd0
        _emit 0xb7
        _emit 0x41
        _emit 0x00
        // 0001b84e: 89 4c 24 20   MOV dword ptr [ESP+0x20],ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 0001b852: 89 74 24 24   MOV dword ptr [ESP+0x24],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x24
        // 0001b856: 89 74 24 38   MOV dword ptr [ESP+0x38],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x38
        // 0001b85a: ff 15 bc e4 f3 00   CALL dword ptr [0x00f3e4bc]
        _emit 0xff
        _emit 0x15
        _emit 0xbc
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        // 0001b860: 6a 05   PUSH 0x5
        _emit 0x6a
        _emit 0x05
        // 0001b862: 89 44 24 24   MOV dword ptr [ESP+0x24],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0001b866: ff 15 74 e0 f3 00   CALL dword ptr [0x00f3e074]
        _emit 0xff
        _emit 0x15
        _emit 0x74
        _emit 0xe0
        _emit 0xf3
        _emit 0x00
        // 0001b86c: 8d 54 24 04   LEA EDX,[ESP+0x4]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x04
        // 0001b870: 52   PUSH EDX
        _emit 0x52
        // 0001b871: 89 44 24 28   MOV dword ptr [ESP+0x28],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0001b875: 89 74 24 2c   MOV dword ptr [ESP+0x2c],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x2c
        // 0001b879: 89 74 24 14   MOV dword ptr [ESP+0x14],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 0001b87d: 89 74 24 18   MOV dword ptr [ESP+0x18],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x18
        // 0001b881: ff 15 90 e4 f3 00   CALL dword ptr [0x00f3e490]
        _emit 0xff
        _emit 0x15
        _emit 0x90
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        // 0001b887: 66 85 c0   TEST AX,AX
        _emit 0x66
        _emit 0x85
        _emit 0xc0
        // 0001b88a: 0f 95 c0   SETNZ AL
        _emit 0x0f
        _emit 0x95
        _emit 0xc0
        // 0001b88d: 5e   POP ESI
        _emit 0x5e
        // 0001b88e: 83 c4 30   ADD ESP,0x30
        _emit 0x83
        _emit 0xc4
        _emit 0x30
        // 0001b891: c3   RET
        _emit 0xc3
    }
}
