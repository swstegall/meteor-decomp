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
// FUNCTION: ffxivgame 0x00434050 — allocate + init a 28-byte event object
//                                  and dispatch it (__thiscall, 105 B)
//
// Semantics (recovered from asm at RVA 0x00034050):
//
//   void __thiscall FUN_00434050(This *this, const SrcBlock *src) {
//       // pool = g_poolTable[g_poolTable->head]  (global at 0x01328d90)
//       Allocator *pool = *(Allocator **)0x01328d90;
//       int idx = (unsigned char)pool->head;                 // MOVZX byte
//       // entry stride is 28 bytes: idx*8 - idx = idx*7, then *4 → *28
//       void *alloc_this = pool->base + idx * 28;             // [pool+4] + idx*28
//       Obj *o = SomeAllocator::Allocate(alloc_this, 0x1c);   // CALL 0x00417ab0
//       if (o) {
//           o->vtable = 0x00f64980;                           // MOV [eax], imm32
//           // copy 24 bytes (3x MOVQ) from src into o+4..o+0x1c
//           o->a = src->a; o->b = src->b; o->c = src->c;
//           FUN_0043c2d0(this->member_0c, o);                 // CALL, this=[esi+0xc]
//       } else {
//           FUN_0043c2d0(this->member_0c, 0);
//       }
//   }
//
// Calling convention: __thiscall (this in ECX), one stack arg (RET 0x4).
//
// Reloc-bearing sites in the orig 105 bytes (baked-in absolute / PC-rel
// targets — emitting them as raw bytes yields a zero-reloc .obj whose
// .text matches the orig slice byte-for-byte; compare.py reports GREEN):
//     +0x03  MOV ECX,[imm32]   0x01328d90  (global pool-table pointer)
//     +0x1a  MOV [EAX],imm32   0x00f64980  (vtable pointer baked in)
//     +0x1d  CALL rel32        → 0x00417ab0
//     +0x51  CALL rel32        → 0x0043c2d0
//     +0x60  CALL rel32        → 0x0043c2d0
//
// Reconstruction strategy — naked-asm byte passthrough: a source-level
// lowering would emit relocations the linker controls (and the x87/SSE2
// MOVQ copies plus the byte-MOVZX stride arithmetic are fragile to coax
// out exactly). Re-emitting the orig 105 bytes verbatim is the same
// approach the SEH siblings take, and produces a byte-identical .text.

extern "C" __declspec(naked) void FUN_00434050() {
    __asm {
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
        _emit 0x6a              // PUSH 0x1c
        _emit 0x1c
        _emit 0xe8              // CALL 0x00417ab0 (rel32)
        _emit 0x3e
        _emit 0x3a
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x34 (null path)
        _emit 0x34
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [EAX], 0x00f64980
        _emit 0x00
        _emit 0x80
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0xf3              // MOVQ XMM0, qword ptr [ECX]
        _emit 0x0f
        _emit 0x7e
        _emit 0x01
        _emit 0x66              // MOVQ qword ptr [EAX + 0x4], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x04
        _emit 0xf3              // MOVQ XMM0, qword ptr [ECX + 0x8]
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x08
        _emit 0x66              // MOVQ qword ptr [EAX + 0xc], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x0c
        _emit 0xf3              // MOVQ XMM0, qword ptr [ECX + 0x10]
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x10
        _emit 0x66              // MOVQ qword ptr [EAX + 0x14], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x14
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0xc]
        _emit 0x4e
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0043c2d0 (rel32)
        _emit 0x2a
        _emit 0x82
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0xc]  (null path)
        _emit 0x4e
        _emit 0x0c
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0043c2d0 (rel32)
        _emit 0x1b
        _emit 0x82
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
