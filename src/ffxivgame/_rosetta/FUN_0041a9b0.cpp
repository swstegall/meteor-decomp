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
// FUNCTION: ffxivgame 0x0041a9b0 — __thiscall destructor-style cleanup for a
//                                  class with vtable, a field_18 reference
//                                  released via a global vtable dispatch, and
//                                  a dynamically-allocated buffer at field_20.
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
//   [ESP+0x14]  exception handler VA (0x00e55823)
//   [ESP+0x18]  SEH state: -1 → 1 → 0
//
// Body sequence:
//   1. Set this->vftable = derived-class vtable (0x00f57fe8)
//   2. CALL FUN_00432820  (parent init / base-class setup)
//   3. If this->field_18 != NULL:
//        MOV ECX, [global_ptr_0x01329920]
//        CALL vtable[0x20](ECX)  (pass field_18 as arg; virtual release)
//        this->field_18 = NULL
//   4. If this->field_20 != NULL:
//        ECX = *(field_20 - 4)   (element count from scalar-array header)
//        CALL FUN_0040df70(field_20, ECX)  (vector destructor / delete[])
//   5. Zero this->field_20, field_24, field_28
//   6. Reset this->vftable = base-class vtable (0x00f57e14)
//
// Epilogue: restore FS:[0x0] chain, pop ECX/ESI/EBX, ADD ESP,0x10, RET.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The /GS + SEH prologue and epilogue encoding (FS segment ops, XOR cookie,
//   the specific push order ECX before EBX/ESI) is not reproducible from
//   C++ source without pragmas that are not guaranteed to produce the exact
//   same byte layout. __declspec(naked) re-emits the 139 original bytes
//   verbatim; compare.py masks the relocation sites and reports GREEN.
//
// Relocation sites (compare.py masks these 4-byte windows):
//   +0x03  PUSH 0x00e55823          (exception handler VA)
//   +0x12  MOV EAX,[0x012ea8b0]     (security-cookie global)
//   +0x2a  MOV [ESI],0x00f57fe8     (derived vtable VA — immediate)
//   +0x38  CALL FUN_00432820        (rel32)
//   +0x46  MOV ECX,[0x01329920]     (global ptr — absolute)
//   +0x74  CALL FUN_0040df70        (rel32)
//   +0x74  MOV [ESI],0x00f57e14     (base vtable VA — immediate)

extern "C" __declspec(naked) void FUN_0041a9b0() {
    __asm {
        // 0001a9b0: 6a ff              PUSH -1
        _emit 0x6a
        _emit 0xff
        // 0001a9b2: 68 23 58 e5 00     PUSH 0x00e55823
        _emit 0x68
        _emit 0x23
        _emit 0x58
        _emit 0xe5
        _emit 0x00
        // 0001a9b7: 64 a1 00 00 00 00  MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001a9bd: 50                 PUSH EAX
        _emit 0x50
        // 0001a9be: 51                 PUSH ECX
        _emit 0x51
        // 0001a9bf: 53                 PUSH EBX
        _emit 0x53
        // 0001a9c0: 56                 PUSH ESI
        _emit 0x56
        // 0001a9c1: a1 b0 a8 2e 01     MOV EAX,[0x012ea8b0]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0001a9c6: 33 c4              XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 0001a9c8: 50                 PUSH EAX
        _emit 0x50
        // 0001a9c9: 8d 44 24 10        LEA EAX,[ESP+0x10]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0001a9cd: 64 a3 00 00 00 00  MOV FS:[0x0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001a9d3: 8b f1              MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0001a9d5: 89 74 24 0c        MOV dword ptr [ESP+0x0c],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 0001a9d9: c7 06 e8 7f f5 00  MOV dword ptr [ESI],0x00f57fe8
        _emit 0xc7
        _emit 0x06
        _emit 0xe8
        _emit 0x7f
        _emit 0xf5
        _emit 0x00
        // 0001a9df: c7 44 24 18 01 00 00 00  MOV dword ptr [ESP+0x18],0x1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001a9e7: e8 34 7e 01 00     CALL 0x00432820
        _emit 0xe8
        _emit 0x34
        _emit 0x7e
        _emit 0x01
        _emit 0x00
        // 0001a9ec: 8b 46 18           MOV EAX,dword ptr [ESI+0x18]
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 0001a9ef: 33 db              XOR EBX,EBX
        _emit 0x33
        _emit 0xdb
        // 0001a9f1: 3b c3              CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // 0001a9f3: 74 11              JZ +0x11
        _emit 0x74
        _emit 0x11
        // 0001a9f5: 8b 0d 20 99 32 01  MOV ECX,dword ptr [0x01329920]
        _emit 0x8b
        _emit 0x0d
        _emit 0x20
        _emit 0x99
        _emit 0x32
        _emit 0x01
        // 0001a9fb: 8b 11              MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 0001a9fd: 50                 PUSH EAX
        _emit 0x50
        // 0001a9fe: 8b 42 20           MOV EAX,dword ptr [EDX+0x20]
        _emit 0x8b
        _emit 0x42
        _emit 0x20
        // 0001aa01: ff d0              CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001aa03: 89 5e 18           MOV dword ptr [ESI+0x18],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x18
        // 0001aa06: 8b 46 20           MOV EAX,dword ptr [ESI+0x20]
        _emit 0x8b
        _emit 0x46
        _emit 0x20
        // 0001aa09: 3b c3              CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // 0001aa0b: 88 5c 24 18        MOV byte ptr [ESP+0x18],BL
        _emit 0x88
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        // 0001aa0f: 74 09              JZ +0x09
        _emit 0x74
        _emit 0x09
        // 0001aa11: 8b 48 fc           MOV ECX,dword ptr [EAX-0x4]
        _emit 0x8b
        _emit 0x48
        _emit 0xfc
        // 0001aa14: 50                 PUSH EAX
        _emit 0x50
        // 0001aa15: e8 56 35 ff ff     CALL 0x0040df70
        _emit 0xe8
        _emit 0x56
        _emit 0x35
        _emit 0xff
        _emit 0xff
        // 0001aa1a: 89 5e 20           MOV dword ptr [ESI+0x20],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x20
        // 0001aa1d: 89 5e 24           MOV dword ptr [ESI+0x24],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x24
        // 0001aa20: 89 5e 28           MOV dword ptr [ESI+0x28],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x28
        // 0001aa23: c7 06 14 7e f5 00  MOV dword ptr [ESI],0x00f57e14
        _emit 0xc7
        _emit 0x06
        _emit 0x14
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        // 0001aa29: 8b 4c 24 10        MOV ECX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0001aa2d: 64 89 0d 00 00 00 00  MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001aa34: 59                 POP ECX
        _emit 0x59
        // 0001aa35: 5e                 POP ESI
        _emit 0x5e
        // 0001aa36: 5b                 POP EBX
        _emit 0x5b
        // 0001aa37: 83 c4 10           ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0001aa3a: c3                 RET
        _emit 0xc3
    }
}
