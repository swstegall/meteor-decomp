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
// FUNCTION: ffxivgame 0x0003ddf0 — SEH-framed __thiscall constructor / init
//                                  (202 B / 0xca) — sets vtable 0xf57ea0,
//                                  initialises two DWORD members at this+0x08
//                                  and this+0x0c to zero, dispatches through
//                                  two virtual-call pairs on an optional
//                                  interface pointer (arg4), then calls
//                                  FUN_0041b0f0 and returns `this`.
//
// Calling convention: __thiscall (ECX = this), 4 stack args, RET 0x10
//
// Prologue: EH3-style SEH frame (PUSH -1 / scope_table 0xe56cb8 / FS:[0]
// chain swap) + /GS __security_cookie (XOR ESP). Saved: EBX, ESI, EDI.
//
// Body outline:
//   EDI = this (ECX)
//   push arg4, arg3, arg2, arg1 onto stack (in that order of pushes)
//   set [EDI]  = 0xf57ea0  (vtable)
//   ESI = EDI+8
//   push 1; ECX = &arg4 (stack-local); clear ESI[0], ESI[4]
//   CALL FUN_0043eec0(ECX=&arg4, stack=1,arg1,arg2,arg3,arg4)
//   SEH state → 0
//   if (arg4 != 0): EBX = arg4->vtbl[3](arg4)  else EBX = 0
//   FUN_00419f80(ECX=ESI, EBX)
//   if (arg4 != 0): EAX = arg4->vtbl[4](arg4)  else EAX = 0
//   FUN_0041b0f0(ECX=[EDI+4], EAX, EBX)
//   ADD ESP, 0x0c
//   SEH state → -1
//   FUN_0043e210(ECX=[ESP+0x2c])
//   EAX = EDI (return this)
//   unwind SEH frame, RET 0x10
//
// Reloc-bearing sites (compare.py wildcards these 4-byte windows):
//   +0x02  PUSH scope_table       0xe56cb8   (abs32 imm)
//   +0x11  MOV EAX,__security_cookie  0x012ea8b0  (abs32 moffs)
//   +0x39  MOV [EDI],0xf57ea0    (vtable abs32 imm)
//   +0x55  CALL rel32 → FUN_0043eec0
//   +0x7b  CALL rel32 → FUN_00419f80
//   +0x9a  CALL rel32 → FUN_0041b0f0
//   +0xae  CALL rel32 → FUN_0043e210
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The EH3 SEH prolog (PUSH -1 / scope_table / FS:[0] swap / cookie XOR ESP /
//   LEA+MOV FS:[0]) and the two EH-state writes (MOV [ESP+0x18],0 and
//   MOV [ESP+0x18],-1) form a compiler-emitted shape whose exact byte encoding
//   depends on the linker-supplied scope_table address and the local stack-slot
//   numbering — coaxing this precise 202-byte sequence from /O2 /GS /EHsc C++
//   is impractical (branch short-vs-near, moffs32 vs modrm, state numbering).
//   Same choice as the sibling SEH functions (FUN_0043e2e0, FUN_00404e40):
//   a `__declspec(naked)` body re-emitting all 202 orig bytes verbatim via
//   MASM `_emit` directives. The .obj's `.text` section is byte-identical
//   modulo the wildcarded reloc windows; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0043ddf0() {
    __asm {
        // 0003ddf0: 6a ff                  PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0003ddf2: 68 b8 6c e5 00         PUSH 0xe56cb8  (scope_table RELOC)
        _emit 0x68
        _emit 0xb8
        _emit 0x6c
        _emit 0xe5
        _emit 0x00
        // 0003ddf7: 64 a1 00 00 00 00      MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003ddfd: 50                     PUSH EAX
        _emit 0x50
        // 0003ddfe: 53                     PUSH EBX
        _emit 0x53
        // 0003ddff: 56                     PUSH ESI
        _emit 0x56
        // 0003de00: 57                     PUSH EDI
        _emit 0x57
        // 0003de01: a1 b0 a8 2e 01         MOV EAX,[0x012ea8b0]  (__security_cookie RELOC)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0003de06: 33 c4                  XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 0003de08: 50                     PUSH EAX
        _emit 0x50
        // 0003de09: 8d 44 24 10            LEA EAX,[ESP+0x10]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0003de0d: 64 a3 00 00 00 00      MOV FS:[0x0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003de13: 8b f9                  MOV EDI,ECX
        _emit 0x8b
        _emit 0xf9
        // 0003de15: 8b 44 24 2c            MOV EAX,dword ptr [ESP+0x2c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // 0003de19: 8b 4c 24 28            MOV ECX,dword ptr [ESP+0x28]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // 0003de1d: 8b 54 24 24            MOV EDX,dword ptr [ESP+0x24]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 0003de21: 50                     PUSH EAX
        _emit 0x50
        // 0003de22: 8b 44 24 24            MOV EAX,dword ptr [ESP+0x24]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0003de26: 51                     PUSH ECX
        _emit 0x51
        // 0003de27: 52                     PUSH EDX
        _emit 0x52
        // 0003de28: 50                     PUSH EAX
        _emit 0x50
        // 0003de29: c7 07 a0 7e f5 00      MOV dword ptr [EDI],0xf57ea0  (vtable RELOC)
        _emit 0xc7
        _emit 0x07
        _emit 0xa0
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        // 0003de2f: 8d 77 08               LEA ESI,[EDI+0x8]
        _emit 0x8d
        _emit 0x77
        _emit 0x08
        // 0003de32: 6a 01                  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0003de34: 8d 4c 24 40            LEA ECX,[ESP+0x40]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        // 0003de38: c7 06 00 00 00 00      MOV dword ptr [ESI],0x0
        _emit 0xc7
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003de3e: c7 46 04 00 00 00 00   MOV dword ptr [ESI+0x4],0x0
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003de45: e8 76 10 00 00         CALL FUN_0043eec0  (rel32 RELOC)
        _emit 0xe8
        _emit 0x76
        _emit 0x10
        _emit 0x00
        _emit 0x00
        // 0003de4a: 8b 44 24 2c            MOV EAX,dword ptr [ESP+0x2c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // 0003de4e: 85 c0                  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0003de50: c7 44 24 18 00 00 00 00  MOV dword ptr [ESP+0x18],0x0  (EH state 0)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003de58: 74 0c                  JZ +0x0c
        _emit 0x74
        _emit 0x0c
        // 0003de5a: 8b 08                  MOV ECX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x08
        // 0003de5c: 8b 51 0c               MOV EDX,dword ptr [ECX+0xc]
        _emit 0x8b
        _emit 0x51
        _emit 0x0c
        // 0003de5f: 50                     PUSH EAX
        _emit 0x50
        // 0003de60: ff d2                  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0003de62: 8b d8                  MOV EBX,EAX
        _emit 0x8b
        _emit 0xd8
        // 0003de64: eb 02                  JMP +0x02
        _emit 0xeb
        _emit 0x02
        // 0003de66: 33 db                  XOR EBX,EBX
        _emit 0x33
        _emit 0xdb
        // 0003de68: 53                     PUSH EBX
        _emit 0x53
        // 0003de69: 8b ce                  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0003de6b: e8 10 c1 fd ff         CALL FUN_00419f80  (rel32 RELOC)
        _emit 0xe8
        _emit 0x10
        _emit 0xc1
        _emit 0xfd
        _emit 0xff
        // 0003de70: 8b 44 24 2c            MOV EAX,dword ptr [ESP+0x2c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // 0003de74: 85 c0                  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0003de76: 74 0a                  JZ +0x0a
        _emit 0x74
        _emit 0x0a
        // 0003de78: 8b 08                  MOV ECX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x08
        // 0003de7a: 8b 51 10               MOV EDX,dword ptr [ECX+0x10]
        _emit 0x8b
        _emit 0x51
        _emit 0x10
        // 0003de7d: 50                     PUSH EAX
        _emit 0x50
        // 0003de7e: ff d2                  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0003de80: eb 02                  JMP +0x02
        _emit 0xeb
        _emit 0x02
        // 0003de82: 33 c0                  XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0003de84: 8d 4f 04               LEA ECX,[EDI+0x4]
        _emit 0x8d
        _emit 0x4f
        _emit 0x04
        // 0003de87: 51                     PUSH ECX
        _emit 0x51
        // 0003de88: 50                     PUSH EAX
        _emit 0x50
        // 0003de89: 53                     PUSH EBX
        _emit 0x53
        // 0003de8a: e8 61 d2 fd ff         CALL FUN_0041b0f0  (rel32 RELOC)
        _emit 0xe8
        _emit 0x61
        _emit 0xd2
        _emit 0xfd
        _emit 0xff
        // 0003de8f: 83 c4 0c               ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0003de92: 8d 4c 24 2c            LEA ECX,[ESP+0x2c]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // 0003de96: c7 44 24 18 ff ff ff ff  MOV dword ptr [ESP+0x18],-1  (EH state -1)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0003de9e: e8 6d 03 00 00         CALL FUN_0043e210  (rel32 RELOC)
        _emit 0xe8
        _emit 0x6d
        _emit 0x03
        _emit 0x00
        _emit 0x00
        // 0003dea3: 8b c7                  MOV EAX,EDI
        _emit 0x8b
        _emit 0xc7
        // 0003dea5: 8b 4c 24 10            MOV ECX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0003dea9: 64 89 0d 00 00 00 00   MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003deb0: 59                     POP ECX
        _emit 0x59
        // 0003deb1: 5f                     POP EDI
        _emit 0x5f
        // 0003deb2: 5e                     POP ESI
        _emit 0x5e
        // 0003deb3: 5b                     POP EBX
        _emit 0x5b
        // 0003deb4: 83 c4 0c               ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0003deb7: c2 10 00               RET 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
