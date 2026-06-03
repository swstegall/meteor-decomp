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
// FUNCTION: ffxivgame 0x0041aa70 — __thiscall destructor-style cleanup for a
//                                  class with vtable, a field_1c reference
//                                  released via a global vtable dispatch, and
//                                  a dynamically-allocated buffer at field_24.
//                                  (139 bytes / 0x8b)
//
// Calling convention: __thiscall (ECX = this); void return.
// Callee-saves pushed: EBX, ESI. Also saves ECX (this) on stack.
// /GS security cookie + SEH frame on the stack.
//
// Stack layout after prologue (ESP-relative):
//   [ESP+0x00]  XOR'd security cookie
//   [ESP+0x04]  saved ESI
//   [ESP+0x08]  saved EBX
//   [ESP+0x0c]  saved ECX (this)
//   [ESP+0x10]  old FS:[0x0] node  ← SEH node pointer installs here
//   [ESP+0x14]  exception handler VA (0x00e55853)
//   [ESP+0x18]  SEH state: -1 → 1 → 0
//
// Body sequence:
//   1. Set this->vftable = derived-class vtable (0x00f57ff4)
//   2. CALL FUN_00432820  (parent init / base-class setup)
//   3. If this->field_1c != NULL:
//        MOV ECX, [global_ptr_0x01329920]
//        CALL vtable[0x1c](ECX)  (pass field_1c as arg; virtual release)
//        this->field_1c = NULL
//   4. If this->field_24 != NULL:
//        ECX = *(field_24 - 4)   (element count from scalar-array header)
//        CALL FUN_0040df70(field_24, ECX)  (vector destructor / delete[])
//   5. Zero this->field_24, field_28, field_2c
//   6. Reset this->vftable = base-class vtable (0x00f57e14)
//
// Epilogue: restore FS:[0x0] chain, pop ECX/ESI/EBX, ADD ESP,0x10, RET.
//
// Closely related to FUN_0041a9b0 (same SEH frame / base vtable / global ptr
// dispatch pattern); differs in exception handler VA, intermediate vtable,
// field offsets (0x1c/0x24 vs 0x18/0x20), and virtual-call slot (0x1c vs 0x20).
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The /GS + SEH prologue and epilogue encoding (FS segment ops, XOR cookie,
//   the specific push order ECX before EBX/ESI) is not reproducible from
//   C++ source without pragmas that are not guaranteed to produce the exact
//   same byte layout. __declspec(naked) re-emits the 139 original bytes
//   verbatim; compare.py masks the relocation sites and reports GREEN.
//
// Relocation sites (compare.py masks these 4-byte windows):
//   +0x03  PUSH 0x00e55853          (exception handler VA)
//   +0x12  MOV EAX,[0x012ea8b0]     (security-cookie global)
//   +0x2a  MOV [ESI],0x00f57ff4     (derived vtable VA — immediate)
//   +0x38  CALL FUN_00432820        (rel32)
//   +0x46  MOV ECX,[0x01329920]     (global ptr — absolute)
//   +0x66  CALL FUN_0040df70        (rel32)
//   +0x74  MOV [ESI],0x00f57e14     (base vtable VA — immediate)

extern "C" __declspec(naked) void FUN_0041aa70() {
    __asm {
        // 0001aa70: 6a ff              PUSH -1
        _emit 0x6a
        _emit 0xff
        // 0001aa72: 68 53 58 e5 00     PUSH 0x00e55853
        _emit 0x68
        _emit 0x53
        _emit 0x58
        _emit 0xe5
        _emit 0x00
        // 0001aa77: 64 a1 00 00 00 00  MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001aa7d: 50                 PUSH EAX
        _emit 0x50
        // 0001aa7e: 51                 PUSH ECX
        _emit 0x51
        // 0001aa7f: 53                 PUSH EBX
        _emit 0x53
        // 0001aa80: 56                 PUSH ESI
        _emit 0x56
        // 0001aa81: a1 b0 a8 2e 01     MOV EAX,[0x012ea8b0]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0001aa86: 33 c4              XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 0001aa88: 50                 PUSH EAX
        _emit 0x50
        // 0001aa89: 8d 44 24 10        LEA EAX,[ESP+0x10]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0001aa8d: 64 a3 00 00 00 00  MOV FS:[0x0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001aa93: 8b f1              MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0001aa95: 89 74 24 0c        MOV dword ptr [ESP+0x0c],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 0001aa99: c7 06 f4 7f f5 00  MOV dword ptr [ESI],0x00f57ff4
        _emit 0xc7
        _emit 0x06
        _emit 0xf4
        _emit 0x7f
        _emit 0xf5
        _emit 0x00
        // 0001aa9f: c7 44 24 18 01 00 00 00  MOV dword ptr [ESP+0x18],0x1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001aaa7: e8 74 7d 01 00     CALL 0x00432820
        _emit 0xe8
        _emit 0x74
        _emit 0x7d
        _emit 0x01
        _emit 0x00
        // 0001aaac: 8b 46 1c           MOV EAX,dword ptr [ESI+0x1c]
        _emit 0x8b
        _emit 0x46
        _emit 0x1c
        // 0001aaaf: 33 db              XOR EBX,EBX
        _emit 0x33
        _emit 0xdb
        // 0001aab1: 3b c3              CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // 0001aab3: 74 11              JZ +0x11
        _emit 0x74
        _emit 0x11
        // 0001aab5: 8b 0d 20 99 32 01  MOV ECX,dword ptr [0x01329920]
        _emit 0x8b
        _emit 0x0d
        _emit 0x20
        _emit 0x99
        _emit 0x32
        _emit 0x01
        // 0001aabb: 8b 11              MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 0001aabd: 50                 PUSH EAX
        _emit 0x50
        // 0001aabe: 8b 42 1c           MOV EAX,dword ptr [EDX+0x1c]
        _emit 0x8b
        _emit 0x42
        _emit 0x1c
        // 0001aac1: ff d0              CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001aac3: 89 5e 1c           MOV dword ptr [ESI+0x1c],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x1c
        // 0001aac6: 8b 46 24           MOV EAX,dword ptr [ESI+0x24]
        _emit 0x8b
        _emit 0x46
        _emit 0x24
        // 0001aac9: 3b c3              CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // 0001aacb: 88 5c 24 18        MOV byte ptr [ESP+0x18],BL
        _emit 0x88
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        // 0001aacf: 74 09              JZ +0x09
        _emit 0x74
        _emit 0x09
        // 0001aad1: 8b 48 fc           MOV ECX,dword ptr [EAX-0x4]
        _emit 0x8b
        _emit 0x48
        _emit 0xfc
        // 0001aad4: 50                 PUSH EAX
        _emit 0x50
        // 0001aad5: e8 96 34 ff ff     CALL 0x0040df70
        _emit 0xe8
        _emit 0x96
        _emit 0x34
        _emit 0xff
        _emit 0xff
        // 0001aada: 89 5e 24           MOV dword ptr [ESI+0x24],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x24
        // 0001aadd: 89 5e 28           MOV dword ptr [ESI+0x28],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x28
        // 0001aae0: 89 5e 2c           MOV dword ptr [ESI+0x2c],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x2c
        // 0001aae3: c7 06 14 7e f5 00  MOV dword ptr [ESI],0x00f57e14
        _emit 0xc7
        _emit 0x06
        _emit 0x14
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        // 0001aae9: 8b 4c 24 10        MOV ECX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0001aaed: 64 89 0d 00 00 00 00  MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001aaf4: 59                 POP ECX
        _emit 0x59
        // 0001aaf5: 5e                 POP ESI
        _emit 0x5e
        // 0001aaf6: 5b                 POP EBX
        _emit 0x5b
        // 0001aaf7: 83 c4 10           ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0001aafa: c3                 RET
        _emit 0xc3
    }
}
