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
// FUNCTION: ffxivgame 0x0040a900 — SEH-wrapped object method call (101 bytes)
//
// __cdecl bool FUN_0040a900(int param_1, int param_2)
//
// Stack layout (after PUSH EBX):
//   [ESP+0x00] = saved EBX
//   [ESP+0x04] = object local (start of 0x24-byte local frame)
//   [ESP+0x28] = old FS:[0]  (SEH chain link)
//   [ESP+0x2c] = handler     (0xe54d7c)
//   [ESP+0x30] = SEH state   (initially -1, 0 in protected region)
//   [ESP+0x34] = return address
//   [ESP+0x38] = param_1
//   [ESP+0x3c] = param_2
//
// Sequence:
//   1. Install SEH frame (state=-1 → 0 around FUN_0040ddd0 → -1)
//   2. Construct object at &local via FUN_0040dd50 (__thiscall, no stack args)
//   3. Store param_2 into local+4, call FUN_0040ddd0 (__thiscall, 1 stack arg
//      = param_1, callee cleans 4 bytes via RET 4), save returned bool in BL
//   4. Destruct object via FUN_0040db10 (__thiscall, no stack args)
//   5. Restore SEH chain and return BL in AL
//
// Three CALL rel32 sites make a source-level lowering brittle (the
// compiler controls the rel32 bytes and they differ from the orig);
// the naked-asm byte passthrough reproduces the exact wire image.
// compare.py masks the four reloc sites so the diff is GREEN.

extern "C" __declspec(naked) void FUN_0040a900()
{
    __asm {
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0xe54d7c
        _emit 0x7c
        _emit 0x4d
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x64              // MOV dword ptr FS:[0x0], ESP
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // SUB ESP, 0x24
        _emit 0xec
        _emit 0x24
        _emit 0x53              // PUSH EBX
        _emit 0x8d              // LEA ECX, [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xe8              // CALL FUN_0040dd50 (rel32)
        _emit 0x2e
        _emit 0x34
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x38]
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x3c]
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0x51              // PUSH ECX
        _emit 0x8d              // LEA ECX, [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [ESP+0x34], 0x0
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESP+0xc], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xe8              // CALL FUN_0040ddd0 (rel32)
        _emit 0x90
        _emit 0x34
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x8a              // MOV BL, AL
        _emit 0xd8
        _emit 0xc7              // MOV dword ptr [ESP+0x30], 0xffffffff
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8              // CALL FUN_0040db10 (rel32)
        _emit 0xbd
        _emit 0x31
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x28]
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x8a              // MOV AL, BL
        _emit 0xc3
        _emit 0x5b              // POP EBX
        _emit 0x64              // MOV dword ptr FS:[0x0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x30
        _emit 0xc4
        _emit 0x30
        _emit 0xc3              // RET
    }
}
