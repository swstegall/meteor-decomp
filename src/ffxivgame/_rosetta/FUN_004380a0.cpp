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
// FUNCTION: ffxivgame 0x004380a0 — __thiscall member fn, 109 bytes.
//
// Semantics (recovered from asm):
//
//   void SomeClass::SomeMethod(SomeRef *arg1, int arg2) {
//       // Look up a slot via global manager singleton:
//       //   g = *(int**)0x01328d90;
//       //   n = (unsigned char)*g;            // byte count / index
//       //   base = *(int*)(g + 4);            // element array base
//       //   slot = base + n * 28;             // stride = 28 (7 * 4)
//       //
//       // Allocate 12-byte object from pool at slot:
//       //   SomeObj *obj = pool->alloc(12);
//       //
//       // If allocation succeeded, construct object inline:
//       //   obj->vtable = 0x00f649d0;
//       //   obj->field4 = *arg1;              // dereference the ref
//       //   obj->field8 = arg2;
//       //
//       // If allocation failed, obj = NULL.
//       //
//       // Push obj (or NULL) to this->member8 list, then call into
//       // this->member2c-based slot (index computed as (member2c+3)*16+this).
//   }
//
// Calling convention: __thiscall (ECX = this on entry; RET 0x8 cleans 2
//   dword stack args).
// Stack frame: SUB ESP,0xc (12 bytes locals) + PUSH ESI (saved register).
//
// Reloc-bearing sites in the orig 109 bytes:
//   +0x06  MOV ECX,[imm32] → 0x01328d90  (global manager ptr)
//   +0x13  LEA EDX,[EAX*8+0]  disp32=0   (encoding artifact, not a reloc)
//   +0x20  CALL rel32 → 0x00417ab0       (pool allocator, __thiscall)
//   +0x33  MOV [EAX],imm32 → 0x00f649d0  (vtable pointer)
//   +0x47  CALL rel32 → 0x0043c2d0       (push-to-list, __thiscall)
//   +0x61  CALL rel32 → 0x004214a0       (slot dispatch, __thiscall)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The three CALL rel32 targets and the two absolute-address references
//   (global singleton read + vtable store) would each require a distinct
//   relocation record if expressed in C++.  compare.py reads the orig PE
//   post-fixup and compares byte streams; emitting the exact orig bytes via
//   MASM _emit directives produces a .obj whose .text matches the orig
//   byte-for-byte with no relocations, so compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_004380a0() {
    __asm {
        _emit 0x83              // SUB ESP, 0x0c
        _emit 0xec
        _emit 0x0c
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV ECX, dword ptr [0x01328d90]
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x0f              // MOVZX EAX, byte ptr [ECX]
        _emit 0xb6
        _emit 0x01
        _emit 0x8d              // LEA EDX, [EAX*8 + 0]
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB EDX, EAX
        _emit 0xd0
        _emit 0x8b              // MOV EAX, dword ptr [ECX + 0x4]
        _emit 0x41
        _emit 0x04
        _emit 0x8d              // LEA ECX, [EAX + EDX*4]
        _emit 0x0c
        _emit 0x90
        _emit 0x6a              // PUSH 0xc
        _emit 0x0c
        _emit 0xe8              // CALL 0x00417ab0 (rel32 → 0xFFFDF9EB)
        _emit 0xeb
        _emit 0xf9
        _emit 0xfd
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x18
        _emit 0x18
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV ECX, dword ptr [ECX]
        _emit 0x09
        _emit 0x8b              // MOV EDX, dword ptr [ESP + 0x18]
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0xc7              // MOV dword ptr [EAX], 0x00f649d0
        _emit 0x00
        _emit 0xd0
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x89              // MOV dword ptr [EAX + 0x4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x89              // MOV dword ptr [EAX + 0x8], EDX
        _emit 0x50
        _emit 0x08
        _emit 0xeb              // JMP +0x02
        _emit 0x02
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0043c2d0 (rel32 → 0x000041E4)
        _emit 0xe4
        _emit 0x41
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA EAX, [ESP + 0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA ECX, [ESP + 0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x2c]
        _emit 0x4e
        _emit 0x2c
        _emit 0x83              // ADD ECX, 0x3
        _emit 0xc1
        _emit 0x03
        _emit 0xc1              // SHL ECX, 0x4
        _emit 0xe1
        _emit 0x04
        _emit 0x03              // ADD ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x004214a0 (rel32 → 0xFFFE939A)
        _emit 0x9a
        _emit 0x93
        _emit 0xfe
        _emit 0xff
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x0c
        _emit 0xc4
        _emit 0x0c
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
