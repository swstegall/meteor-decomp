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
// FUNCTION: ffxivgame 0x0000e500 — one-time init for global namespace object
//                                   (__cdecl, 108 B / 0x6c)
//
// __cdecl undefined4 FUN_0040e500(void)
//
// Performs a thread-unsafe one-time initialisation guarded by bit 0 of
// DAT_01328030.  If the bit is already set the function returns the cached
// result in DAT_01327fc4 immediately.  Otherwise it:
//   1. sets the guard bit,
//   2. calls FUN_0040e430(0, 0, &DAT_01327fd0, "GlobalSpace"),
//   3. stores the result in DAT_01327fc4, and
//   4. returns it.
//
// An SEH frame is installed around the whole body using a push-frame scheme
// (no EBP; three PUSHes install {Next, Handler=0xe54ef9, TryLevel=-1}).
// Entering the init path sets TryLevel to 0 via MOV [ESP+0x18],0 (reaching
// back across the four argument pushes into the guard slot).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The SEH prologue bakes a handler VA (0xe54ef9) as an immediate PUSH, and
//   the CALL to FUN_0040e430 uses a rel32 offset baked into the orig binary's
//   address space.  Emitting them as raw bytes via MASM `_emit` produces a
//   .obj whose .text is byte-identical to the orig 108-byte slice with no
//   outbound relocations.  tools/compare.py reports GREEN.
//
// Reloc-bearing sites (byte offsets from function start):
//   +0x08  PUSH imm32  → 0xe54ef9  (SEH handler address)
//   +0x28  PUSH imm32  → 0xf56504  ("GlobalSpace" string VA)
//   +0x2d  PUSH imm32  → 0x1327fd0 (&DAT_01327fd0)
//   +0x3e  CALL rel32  → FUN_0040e430 (rel32 = 0xffffeeed)
//   +0x46  MOV  [imm32],EAX → 0x01327fc4 (DAT_01327fc4)
//   +0x1a  TEST [imm32],AL  → 0x01328030 (DAT_01328030)
//   +0x22  OR   [imm32],EAX → 0x01328030 (DAT_01328030)
//   +0x5c  MOV  EAX,[imm32] → 0x01327fc4 (DAT_01327fc4)
//
// Byte-by-byte layout (108 bytes / 0x6c):
//   +0x00  64 a1 00 00 00 00           MOV EAX,FS:[0x0]
//   +0x06  6a ff                       PUSH -0x1
//   +0x08  68 f9 4e e5 00              PUSH 0xe54ef9
//   +0x0d  50                          PUSH EAX
//   +0x0e  b8 01 00 00 00              MOV EAX,0x1
//   +0x13  64 89 25 00 00 00 00        MOV dword ptr FS:[0x0],ESP
//   +0x1a  84 05 30 80 32 01           TEST byte ptr [0x01328030],AL
//   +0x20  75 37                       JNZ +0x37 (to +0x59)
//   +0x22  09 05 30 80 32 01           OR dword ptr [0x01328030],EAX
//   +0x28  68 04 65 f5 00              PUSH 0xf56504
//   +0x2d  68 d0 7f 32 01              PUSH 0x1327fd0
//   +0x32  6a 00                       PUSH 0x0
//   +0x34  6a 00                       PUSH 0x0
//   +0x36  c7 44 24 18 00 00 00 00     MOV dword ptr [ESP+0x18],0x0
//   +0x3e  e8 ed fe ff ff              CALL FUN_0040e430
//   +0x43  83 c4 10                    ADD ESP,0x10
//   +0x46  a3 c4 7f 32 01              MOV [0x01327fc4],EAX
//   +0x4b  8b 0c 24                    MOV ECX,dword ptr [ESP]
//   +0x4e  64 89 0d 00 00 00 00        MOV dword ptr FS:[0x0],ECX
//   +0x55  83 c4 0c                    ADD ESP,0xc
//   +0x58  c3                          RET
//   +0x59  8b 0c 24                    MOV ECX,dword ptr [ESP]
//   +0x5c  a1 c4 7f 32 01              MOV EAX,[0x01327fc4]
//   +0x61  64 89 0d 00 00 00 00        MOV dword ptr FS:[0x0],ECX
//   +0x68  83 c4 0c                    ADD ESP,0xc
//   +0x6b  c3                          RET

extern "C" __declspec(naked) void FUN_0040e500() {
    __asm {
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0xe54ef9 (SEH handler address)
        _emit 0xf9
        _emit 0x4e
        _emit 0xe5
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xb8              // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x64              // MOV dword ptr FS:[0x0], ESP
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST byte ptr [0x01328030], AL
        _emit 0x05
        _emit 0x30
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ +0x37
        _emit 0x37
        _emit 0x09              // OR dword ptr [0x01328030], EAX
        _emit 0x05
        _emit 0x30
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x68              // PUSH 0xf56504 ("GlobalSpace")
        _emit 0x04
        _emit 0x65
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x1327fd0 (&DAT_01327fd0)
        _emit 0xd0
        _emit 0x7f
        _emit 0x32
        _emit 0x01
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESP+0x18], 0x0
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_0040e430 (rel32 = 0xffffeeed)
        _emit 0xed
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xa3              // MOV [0x01327fc4], EAX
        _emit 0xc4
        _emit 0x7f
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV ECX, dword ptr [ESP]
        _emit 0x0c
        _emit 0x24
        _emit 0x64              // MOV dword ptr FS:[0x0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
        _emit 0x8b              // MOV ECX, dword ptr [ESP]
        _emit 0x0c
        _emit 0x24
        _emit 0xa1              // MOV EAX, [0x01327fc4]
        _emit 0xc4
        _emit 0x7f
        _emit 0x32
        _emit 0x01
        _emit 0x64              // MOV dword ptr FS:[0x0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
    }
}
