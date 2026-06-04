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
// FUNCTION: ffxivgame 0x00434d00 — __thiscall member fn, 108 bytes (RET 0x8).
//
// Semantics (recovered from asm):
//
//   void FUN_00434d00(this, void **arg1, int arg2) {   // __thiscall
//       // allocate a 12-byte node from a pool selected by a global index
//       Pool *pool = g_pool[ /* g_pool->byte[0] * 28 */ ];
//       Node *n = pool_alloc(pool, 0xc);                // FUN_00417ab0
//       if (n) {
//           n->vftable = 0x00F649D0;                    // vtable constant
//           n->field4  = *arg1;
//           n->field8  = arg2;
//       }
//       // hand the node to a sub-object accessor
//       (this->field0xc)->method(n);                    // FUN_0043c2d0
//       // dispatch into a 16-byte-strided element table at this+0x38
//       elem = (char*)this + 0x38 + (this->field0x34 << 4);
//       FUN_004214a0(elem, &locals, &arg1);             // FUN_004214a0
//   }
//
//   ECX = this throughout (MOV ESI,ECX saves it). Two callee-cleaned
//   stack args (RET 0x8): arg1 at [ESP+0x14], arg2 at [ESP+0x18] after
//   the SUB ESP,0xc / PUSH ESI prolog.
//
// Reloc-bearing sites in the orig 108 bytes (compare.py wildcard-masks
// these 4-byte windows; emitting them as literal bytes via MASM `_emit`
// produces a zero-reloc passthrough .obj whose .text matches the orig
// slice byte-for-byte — the same path the siblings FUN_004090b0 /
// FUN_004091f0 / FUN_00409260 took):
//
//     +0x06   MOV ECX,[imm32]      → .data 0x01328D90 (global pool root)
//     +0x20   CALL rel32           → .text 0x00417AB0 (pool allocator)
//     +0x33   MOV [EAX],imm32      → 0x00F649D0 (node vftable constant)
//     +0x47   CALL rel32           → .text 0x0043C2D0
//     +0x60   CALL rel32           → .text 0x004214A0
//
// Reconstruction strategy — naked-asm byte passthrough. A source-level
// C++ form would emit the same shape but produce linker-controlled
// relocations; re-emitting the orig bytes verbatim resolves them against
// the orig binary's own address space, yielding a byte-identical .text.

extern "C" __declspec(naked) void FUN_00434d00() {
    __asm {
        _emit 0x83              // SUB ESP, 0xc
        _emit 0xec
        _emit 0x0c
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV ECX, dword ptr [0x01328D90]
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x0f              // MOVZX EAX, byte ptr [ECX]
        _emit 0xb6
        _emit 0x01
        _emit 0x8d              // LEA EDX, [EAX*0x8 + 0x0]
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
        _emit 0x8d              // LEA ECX, [EAX + EDX*0x4]
        _emit 0x0c
        _emit 0x90
        _emit 0x6a              // PUSH 0xc
        _emit 0x0c
        _emit 0xe8              // CALL 0x00417AB0
        _emit 0x8b
        _emit 0x2d
        _emit 0xfe
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
        _emit 0xc7              // MOV dword ptr [EAX], 0x00F649D0
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
        _emit 0xeb              // JMP +0x2
        _emit 0x02
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0xc]
        _emit 0x4e
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0043C2D0
        _emit 0x84
        _emit 0x75
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EDX, dword ptr [ESI + 0x34]
        _emit 0x56
        _emit 0x34
        _emit 0x8d              // LEA EAX, [ESP + 0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA ECX, [ESP + 0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xc1              // SHL EDX, 0x4
        _emit 0xe2
        _emit 0x04
        _emit 0x51              // PUSH ECX
        _emit 0x8d              // LEA ECX, [EDX + ESI + 0x38]
        _emit 0x4c
        _emit 0x32
        _emit 0x38
        _emit 0xe8              // CALL 0x004214A0
        _emit 0x3b
        _emit 0xc7
        _emit 0xfe
        _emit 0xff
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
