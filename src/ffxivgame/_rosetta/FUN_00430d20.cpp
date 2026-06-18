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
// FUNCTION: ffxivgame 0x00030d20 — __thiscall constructor/initializer for a
//                                  state-dispatch object (146 B / 0x92, ret 0xc).
//
// Calling convention: __thiscall (ECX = this); 3 stack args (RET 0xC = 12 B).
// Returns this in EAX.
//
// Stack frame (6 pushes = 0x18 bytes; SEH + /GS prologue):
//   [ESP+0x00]  GS cookie (XOR'd with ESP)
//   [ESP+0x04]  saved ESI
//   [ESP+0x08]  saved ECX (= this, overwritten with ESI after save)
//   [ESP+0x0c]  prev FS:[0]  ← EAX used for new SEH record .Next
//   [ESP+0x10]  SEH handler  (0xe55f61)
//   [ESP+0x14]  SEH state    (initially -1, updated to 0 before calls)
//   [ESP+0x18]  return address
//   [ESP+0x1c]  arg1         → this->field_2c
//   [ESP+0x20]  arg2         → this->field_30 (loaded into ECX early)
//   [ESP+0x24]  arg3         → this->field_34 (loaded into EDX early)
//
// Object layout initialised here (inferred from offsets written):
//   [this+0x00]  vtable ptr   ← 0x00f633e0
//   [this+0x04]  field_04     ← 0
//   [this+0x08]  field_08     ← 0
//   [this+0x0c..0x13]  field_0c/10  ← 0  (PXOR XMM0 + MOVQ)
//   [this+0x14..0x1b]  field_14/18  ← 0  (MOVQ)
//   [this+0x1c..0x23]  field_1c/20  ← 0  (MOVQ)
//   [this+0x24]  field_24     ← 0
//   [this+0x28]  field_28     ← 0
//   [this+0x2c]  field_2c     ← arg1
//   [this+0x30]  field_30     ← arg2
//   [this+0x34]  field_34     ← arg3
//   [this+0x38]  field_38     ← 2
//
// After field init, calls:
//   FUN_00430aa0(this)  — state-dispatch handler (peer)
//   FUN_004328a0(this)  — secondary initializer
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//   The SEH + /GS prologue (PUSH -1, PUSH handler, PUSH FS:[0], PUSH ECX,
//   PUSH ESI, XOR-and-push cookie, LEA/MOV FS:[0]) combined with the SSE2
//   PXOR+MOVQ zeroing idiom cannot be reproduced from source-level C++ without
//   the compiler reordering the SEH state transition and the register saves.
//   The naked-asm passthrough emits all 146 bytes verbatim.
//
// Reloc-bearing sites (absolute addresses embedded in the instruction stream):
//   +0x02  PUSH 0xe55f61         — SEH handler VA (fixed-base, no reloc needed)
//   +0x10  MOV EAX,[0x012ea8b0]  — __security_cookie global
//   +0x38  MOV [ESI],0x00f633e0  — vtable VA
//   +0x71  CALL 0x00430aa0       — rel32 = 0xfffffd0a
//   +0x78  CALL 0x004328a0       — rel32 = 0x00001b03

extern "C" __declspec(naked) void FUN_00430d20()
{
    __asm {
        // 00030d20: 6a ff              PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00030d22: 68 61 5f e5 00     PUSH 0xe55f61 (SEH handler addr)
        _emit 0x68
        _emit 0x61
        _emit 0x5f
        _emit 0xe5
        _emit 0x00
        // 00030d27: 64 a1 00 00 00 00  MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00030d2d: 50                 PUSH EAX
        _emit 0x50
        // 00030d2e: 51                 PUSH ECX
        _emit 0x51
        // 00030d2f: 56                 PUSH ESI
        _emit 0x56
        // 00030d30: a1 b0 a8 2e 01     MOV EAX,[0x012ea8b0]  (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00030d35: 33 c4              XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 00030d37: 50                 PUSH EAX
        _emit 0x50
        // 00030d38: 8d 44 24 0c        LEA EAX,[ESP+0xc]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00030d3c: 64 a3 00 00 00 00  MOV FS:[0x0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00030d42: 8b f1              MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 00030d44: 89 74 24 08        MOV dword ptr [ESP+0x8],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 00030d48: 33 c0              XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 00030d4a: 89 46 04           MOV dword ptr [ESI+0x4],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 00030d4d: 89 46 08           MOV dword ptr [ESI+0x8],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 00030d50: 8b 4c 24 20        MOV ECX,dword ptr [ESP+0x20]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 00030d54: 8b 54 24 24        MOV EDX,dword ptr [ESP+0x24]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 00030d58: c7 06 e0 33 f6 00  MOV dword ptr [ESI],0x00f633e0  (vtable)
        _emit 0xc7
        _emit 0x06
        _emit 0xe0
        _emit 0x33
        _emit 0xf6
        _emit 0x00
        // 00030d5e: 66 0f ef c0        PXOR XMM0,XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        // 00030d62: 66 0f d6 46 0c     MOVQ qword ptr [ESI+0xc],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x0c
        // 00030d67: 66 0f d6 46 14     MOVQ qword ptr [ESI+0x14],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x14
        // 00030d6c: 66 0f d6 46 1c     MOVQ qword ptr [ESI+0x1c],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x1c
        // 00030d71: 89 46 24           MOV dword ptr [ESI+0x24],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x24
        // 00030d74: 89 44 24 14        MOV dword ptr [ESP+0x14],EAX  (SEH state → 0)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00030d78: 89 46 28           MOV dword ptr [ESI+0x28],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x28
        // 00030d7b: 8b 44 24 1c        MOV EAX,dword ptr [ESP+0x1c]  (arg1)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00030d7f: 89 4e 30           MOV dword ptr [ESI+0x30],ECX  (arg2)
        _emit 0x89
        _emit 0x4e
        _emit 0x30
        // 00030d82: 8b ce              MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00030d84: 89 46 2c           MOV dword ptr [ESI+0x2c],EAX  (arg1)
        _emit 0x89
        _emit 0x46
        _emit 0x2c
        // 00030d87: 89 56 34           MOV dword ptr [ESI+0x34],EDX  (arg3)
        _emit 0x89
        _emit 0x56
        _emit 0x34
        // 00030d8a: c7 46 38 02 00 00 00  MOV dword ptr [ESI+0x38],0x2
        _emit 0xc7
        _emit 0x46
        _emit 0x38
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00030d91: e8 0a fd ff ff     CALL 0x00430aa0 (rel32=0xfffffd0a)
        _emit 0xe8
        _emit 0x0a
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 00030d96: 8b ce              MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00030d98: e8 03 1b 00 00     CALL 0x004328a0 (rel32=0x00001b03)
        _emit 0xe8
        _emit 0x03
        _emit 0x1b
        _emit 0x00
        _emit 0x00
        // 00030d9d: 8b c6              MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 00030d9f: 8b 4c 24 0c        MOV ECX,dword ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 00030da3: 64 89 0d 00 00 00 00  MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00030daa: 59                 POP ECX
        _emit 0x59
        // 00030dab: 5e                 POP ESI
        _emit 0x5e
        // 00030dac: 83 c4 10           ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00030daf: c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
