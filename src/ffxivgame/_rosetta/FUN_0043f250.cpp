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
// FUNCTION: ffxivgame 0x0043f250 — GlobalSpace lazy-init + alloc wrapper
//                                   (__cdecl, 50 B / 0x32)
//
// void* FUN_0043f250(unsigned int size)
//   [ESP+0x04] : size — allocation size in bytes (passed through to FUN_0040e110)
//
// Behaviour:
//   1. Load global allocator object from DAT_01327fc0.
//      If NULL, call FUN_0040e500 to obtain it (lazy init / GlobalSpace getter).
//   2. Construct an 8-byte Space descriptor on the stack.
//      Call FUN_0040e2b0 (__thiscall, ECX=&local) to set:
//        local.field_0 = 0x10  (space tag / type id)
//        local.field_4 = 0
//      FUN_0040e2b0 returns EAX = ECX = &local.
//   3. Call FUN_0040e110 (__thiscall, ECX=global_alloc_obj):
//        alloc(size=param1, space=&local)  → allocated memory pointer.
//   4. Return the result.
//
// Calling convention: __cdecl (caller cleans; plain RET).
// Frame: SUB ESP,0x8 (8-byte local Space struct); PUSH ESI (callee-save).
//   Unusual prologue order — MSVC FPO alloc before callee-save push.
//
// Reloc-bearing sites (byte offsets from function start):
//   +0x05  MOV  ESI,[imm32]   → 0x01327fc0 (g_alloc_obj ptr)
//   +0x0f  CALL rel32         → FUN_0040e500
//   +0x1c  CALL rel32         → FUN_0040e2b0
//   +0x29  CALL rel32         → FUN_0040e110
//
// Reconstruction strategy — naked-asm byte passthrough.
//   The unusual SUB-before-PUSH prologue order and the thiscall dispatch
//   via ECX cannot be reliably reproduced from clean C++ with MSVC 2005 /O2.
//   The __declspec(naked) body emits all 50 original bytes verbatim.

extern "C" __declspec(naked) void FUN_0043f250() {
    __asm {
        // 0003f250: 83 ec 08   SUB ESP,0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 0003f253: 56   PUSH ESI
        _emit 0x56
        // 0003f254: 8b 35 c0 7f 32 01   MOV ESI,dword ptr [0x01327fc0]  [reloc +0x05: abs32]
        _emit 0x8b
        _emit 0x35
        _emit 0xc0
        _emit 0x7f
        _emit 0x32
        _emit 0x01
        // 0003f25a: 85 f6   TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 0003f25c: 75 07   JNZ +7 (to 0x0043f265)
        _emit 0x75
        _emit 0x07
        // 0003f25e: e8 9d f2 fc ff   CALL 0x0040e500  [reloc +0x0f: rel32]
        _emit 0xe8
        _emit 0x9d
        _emit 0xf2
        _emit 0xfc
        _emit 0xff
        // 0003f263: 8b f0   MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // 0003f265: 6a 10   PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 0003f267: 8d 4c 24 08   LEA ECX,[ESP+0x8]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 0003f26b: e8 40 f0 fc ff   CALL 0x0040e2b0  [reloc +0x1c: rel32]
        _emit 0xe8
        _emit 0x40
        _emit 0xf0
        _emit 0xfc
        _emit 0xff
        // 0003f270: 50   PUSH EAX
        _emit 0x50
        // 0003f271: 8b 44 24 14   MOV EAX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0003f275: 50   PUSH EAX
        _emit 0x50
        // 0003f276: 8b ce   MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0003f278: e8 93 ee fc ff   CALL 0x0040e110  [reloc +0x29: rel32]
        _emit 0xe8
        _emit 0x93
        _emit 0xee
        _emit 0xfc
        _emit 0xff
        // 0003f27d: 5e   POP ESI
        _emit 0x5e
        // 0003f27e: 83 c4 08   ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0003f281: c3   RET
        _emit 0xc3
    }
}
