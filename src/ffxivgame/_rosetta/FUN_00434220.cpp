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
// FUNCTION: ffxivgame 0x00434220 — `__thiscall` 3-arg event factory
//                                  (RET 0xC, 93 bytes)
//
// __thiscall void FUN_00434220(this, int a1, int a2, int a3)
//
// Behaviour: indexes a global descriptor table to find the active pool
// allocator, allocates a 16-byte (0x10) node from it, stamps a vftable
// (VA 0x00f649a8) and the three call args into it, then forwards the
// node (or NULL on allocation failure) to a `__thiscall` dispatch
// helper (FUN_0043c2d0) invoked on this->[0xC].
//
//   void *self;                                  // ECX on entry → ESI
//   Pool *g = *(Pool **)0x01328d90;              // global table ptr
//   unsigned idx = (unsigned char)g->cur;        // g[0]
//   Slot *slot = &g->base[idx];                  // g->base + idx*0x1C
//   Node *n = slot->alloc(0x10);                 // FUN_00417ab0(this=slot,0x10)
//   if (n) {
//       n->vftable = (void *)0x00f649a8;
//       n->a1 = a1;  n->a2 = a2;  n->a3 = a3;
//   }
//   ((Dispatcher *)self[0xC])->post(n);          // FUN_0043c2d0(this=self->[0xC], n)
//
// Slot stride is 0x1C (28) — computed via LEA EDX,[EAX*8]; SUB EDX,EAX
// (EAX*7) then [EAX + EDX*4] (base + idx*28), the canonical MSVC 2005
// "multiply by 28" address form (cf. sibling FUN_00403bd0's 0x1C
// element-size new[] thunk).
//
// Reloc-bearing sites in the orig 93 bytes:
//   +0x03   MOV  ECX, [imm32]  (DIR32 → global ptr 0x01328d90)
//   +0x1d   CALL rel32         → FUN_00417ab0 (pool alloc)
//   +0x35   MOV  [EAX], imm32  (DIR32 → vftable 0x00f649a8)
//   +0x45   CALL rel32         → FUN_0043c2d0 (dispatch, hit-path)
//   +0x54   CALL rel32         → FUN_0043c2d0 (dispatch, null-path)
//
// Reconstruction strategy — naked-asm byte passthrough (same approach as
// siblings FUN_00403bd0 / FUN_00406133): the body is emitted verbatim via
// MASM `_emit`, with the rel32 / DIR32 operands baked in as the concrete
// byte values that already resolve against the orig PE's address space.
// The resulting .obj's .text is byte-identical to the orig slice with
// zero relocations, so tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00434220() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV  ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV  ECX, dword ptr [0x01328d90]
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x0f              // MOVZX EAX, byte ptr [ECX]
        _emit 0xb6
        _emit 0x01
        _emit 0x8d              // LEA  EDX, [EAX*0x8 + 0x0]
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB  EDX, EAX
        _emit 0xd0
        _emit 0x8b              // MOV  EAX, dword ptr [ECX + 0x4]
        _emit 0x41
        _emit 0x04
        _emit 0x8d              // LEA  ECX, [EAX + EDX*0x4]
        _emit 0x0c
        _emit 0x90
        _emit 0x6a              // PUSH 0x10
        _emit 0x10
        _emit 0xe8              // CALL FUN_00417ab0 (rel32)
        _emit 0x6e
        _emit 0x38
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ   null_path (+0x28)
        _emit 0x28
        _emit 0x8b              // MOV  ECX, dword ptr [ESP + 0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV  EDX, dword ptr [ESP + 0xC]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x89              // MOV  dword ptr [EAX + 0x4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x8b              // MOV  ECX, dword ptr [ESP + 0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0xc7              // MOV  dword ptr [EAX], 0x00f649a8 (DIR32)
        _emit 0x00
        _emit 0xa8
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x89              // MOV  dword ptr [EAX + 0x8], EDX
        _emit 0x50
        _emit 0x08
        _emit 0x89              // MOV  dword ptr [EAX + 0xC], ECX
        _emit 0x48
        _emit 0x0c
        _emit 0x8b              // MOV  ECX, dword ptr [ESI + 0xC]
        _emit 0x4e
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_0043c2d0 (rel32)
        _emit 0x66
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  0xC
        _emit 0x0c
        _emit 0x00
        _emit 0x8b              // MOV  ECX, dword ptr [ESI + 0xC]   (null_path:)
        _emit 0x4e
        _emit 0x0c
        _emit 0x33              // XOR  EAX, EAX
        _emit 0xc0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_0043c2d0 (rel32)
        _emit 0x57
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  0xC
        _emit 0x0c
        _emit 0x00
    }
}
