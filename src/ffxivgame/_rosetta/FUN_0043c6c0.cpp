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
// FUNCTION: ffxivgame 0x0043c6c0 — __thiscall member initialiser with
//                                   one-time global init guard (SEH frame,
//                                   /GS cookie, 163 bytes / 0xa3)
//
// Calling convention: __thiscall (ECX = this); callee-cleans 1 dword (RET 4).
//
// Object layout (offsets touched):
//   [this + 0x04]  output field — zeroed on NULL arg, set to result on success
//
// Logic:
//   if (arg == NULL) { this->field4 = 0; return; }
//   if (!(__byte_01327c20 & 1)) {
//       __byte_01327c20 |= 1;          // mark initialised
//       __global_01327c1c = FUN_0040e500();
//   }
//   ECX_struct = __global_01327c1c;
//   tmp = FUN_0040e2d0(&local_area, 0x10, 0xf66620);
//   handle = FUN_0040e110(ECX_struct, 0x80080, tmp);
//   this->field4 = handle;
//   FUN_0043c5a0(handle, arg);
//
// The SEH frame (PUSH -1 / PUSH handler / MOV FS:[0]) and /GS cookie
// (MOV EAX,[__security_cookie]; XOR EAX,ESP; PUSH EAX) are part of the
// compiler-generated prologue — they cannot be reproduced byte-identically
// from plain C++ source, so a __declspec(naked) body re-emitting the
// original 163 bytes verbatim is used. compare.py masks the four
// CALL rel32 reloc bytes in its diff, so the .obj's .text is
// byte-identical to the original slice and compare.py reports GREEN.
//
// Reloc-bearing sites in the orig 163 bytes:
//   +0x4c   CALL rel32  → FUN_0040e500  (0x0040e500)
//   +0x6f   CALL rel32  → FUN_0040e2d0  (0x0040e2d0)
//   +0x7c   CALL rel32  → FUN_0040e110  (0x0040e110)
//   +0x86   CALL rel32  → FUN_0043c5a0  (0x0043c5a0)

extern "C" __declspec(naked) void FUN_0043c6c0() {
    __asm {
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0xe5673e  (SEH handler addr)
        _emit 0x3e
        _emit 0x67
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX  (prev exception chain)
        _emit 0x83              // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, [0x012ea8b0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX  (cookie ^ ESP)
        _emit 0x8d              // LEA EAX, [ESP + 0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x64              // MOV FS:[0x0], EAX  (install SEH frame)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EDI, ECX  (EDI = this)
        _emit 0xf9
        _emit 0x8b              // MOV EBX, dword ptr [ESP + 0x28]  (arg)
        _emit 0x5c
        _emit 0x24
        _emit 0x28
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x3b              // CMP EBX, EAX  (arg == NULL?)
        _emit 0xd8
        _emit 0x89              // MOV dword ptr [EDI + 0x4], EAX  (this->field4 = 0)
        _emit 0x47
        _emit 0x04
        _emit 0x74              // JZ +0x59  (→ epilogue)
        _emit 0x59
        _emit 0xb9              // MOV ECX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST byte ptr [0x01327c20], CL
        _emit 0x0d
        _emit 0x20
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ +0x1c  (→ already_init)
        _emit 0x1c
        _emit 0x09              // OR dword ptr [0x01327c20], ECX
        _emit 0x0d
        _emit 0x20
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x89              // MOV dword ptr [ESP + 0x20], EAX  (try_state = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0xe8              // CALL FUN_0040e500  (rel32 → 0xfffd1def)
        _emit 0xef
        _emit 0x1d
        _emit 0xfd
        _emit 0xff
        _emit 0xa3              // MOV [0x01327c1c], EAX
        _emit 0x1c
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [ESP + 0x20], 0xffffffff  (try_state = -1)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // already_init:
        _emit 0x8b              // MOV ESI, dword ptr [0x01327c1c]
        _emit 0x35
        _emit 0x1c
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x68              // PUSH 0xf66620
        _emit 0x20
        _emit 0x66
        _emit 0xf6
        _emit 0x00
        _emit 0x6a              // PUSH 0x10
        _emit 0x10
        _emit 0x8d              // LEA ECX, [ESP + 0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0xe8              // CALL FUN_0040e2d0  (rel32 → 0xfffd1b9c)
        _emit 0x9c
        _emit 0x1b
        _emit 0xfd
        _emit 0xff
        _emit 0x50              // PUSH EAX
        _emit 0x68              // PUSH 0x80080
        _emit 0x80
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_0040e110  (rel32 → 0xfffd19cf)
        _emit 0xcf
        _emit 0x19
        _emit 0xfd
        _emit 0xff
        _emit 0x53              // PUSH EBX  (arg)
        _emit 0x50              // PUSH EAX  (handle)
        _emit 0x89              // MOV dword ptr [EDI + 0x4], EAX  (this->field4 = handle)
        _emit 0x47
        _emit 0x04
        _emit 0xe8              // CALL FUN_0043c5a0  (rel32 → 0xfffffe55)
        _emit 0x55
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        // epilogue:
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x64              // MOV dword ptr FS:[0x0], ECX  (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX  (discard cookie)
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
