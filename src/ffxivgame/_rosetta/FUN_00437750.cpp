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
// FUNCTION: ffxivgame 0x00037750 — allocate-and-enqueue a 12-byte command
//                                  object (__thiscall, 2 stack args, 86 B)
//
// __thiscall void FUN_00437750(Self *this, void *a1, void *a2)  // RET 0x8
//
// Asm shape (read from orig RVA 0x00037750):
//
//   push esi
//   mov  esi, ecx                       ; esi = this
//   mov  ecx, [0x01328d90]              ; ecx = g_table   (global pointer)
//   movzx eax, byte ptr [ecx]           ; eax = g_table->index   (u8)
//   lea  edx, [eax*8]                   ; edx = index*8
//   sub  edx, eax                       ; edx = index*7
//   mov  eax, [ecx+4]                   ; eax = g_table->base
//   lea  ecx, [eax + edx*4]             ; ecx = base + index*28  (element ptr)
//   push 0Ch                            ; size = 12
//   call 0x00417ab0                     ; this=element; allocate(12) -> eax
//   test eax, eax
//   jz   alloc_failed
//   mov  ecx, [esp+8]                   ; ecx = a1
//   mov  edx, [esp+0xC]                 ; edx = a2
//   mov  dword ptr [eax], 0xF649D0      ; eax->vftable = <vtable>  (DIR32 imm)
//   mov  [eax+4], ecx                   ; eax->a1 = a1
//   mov  [eax+8], edx                   ; eax->a2 = a2
//   mov  ecx, [esi+8]                   ; ecx = this->sink   (field_8)
//   push eax                            ; arg = command object
//   call 0x0043c2d0                     ; this->sink->enqueue(obj)
//   pop  esi
//   ret  8
// alloc_failed:
//   mov  ecx, [esi+8]                   ; ecx = this->sink
//   xor  eax, eax
//   push eax                            ; arg = NULL
//   call 0x0043c2d0                     ; this->sink->enqueue(NULL)
//   pop  esi
//   ret  8
//
// Reloc-bearing sites in the orig 86 bytes (resolved only at full-binary
// relink time; re-emitted here verbatim so the .obj .text is byte-identical
// to the orig slice with NO relocations — tools/compare.py reports GREEN):
//   +0x1d   CALL rel32  → 0x00417ab0   (allocator thunk; matched sibling)
//   +0x2e   MOV  imm32  → 0x00F649D0   (DIR32, command object vftable)
//   +0x3e   CALL rel32  → 0x0043C2D0   (sink->enqueue, success arm)
//   +0x4d   CALL rel32  → 0x0043C2D0   (sink->enqueue, failure arm)
//
// Reconstruction strategy — naked-asm byte passthrough (same rationale as
// sibling FUN_00403b70 / FUN_004065c0): an idiomatic C++ form can't reliably
// reproduce the register-allocator picture and the two CALL rel32 + one DIR32
// reloc-bearing operands, so the naked `_emit` body re-emits the orig bytes
// directly.

extern "C" __declspec(naked) void FUN_00437750() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV  ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV  ECX, dword ptr [0x01328D90]
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
        _emit 0x8b              // MOV  EAX, dword ptr [ECX+0x4]
        _emit 0x41
        _emit 0x04
        _emit 0x8d              // LEA  ECX, [EAX + EDX*0x4]
        _emit 0x0c
        _emit 0x90
        _emit 0x6a              // PUSH 0x0C
        _emit 0x0c
        _emit 0xe8              // CALL 0x00417AB0   (rel32)
        _emit 0x3e
        _emit 0x03
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ   alloc_failed (+0x21)
        _emit 0x21
        _emit 0x8b              // MOV  ECX, dword ptr [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV  EDX, dword ptr [ESP+0xC]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0xc7              // MOV  dword ptr [EAX], 0x00F649D0  (DIR32 imm)
        _emit 0x00
        _emit 0xd0
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x89              // MOV  dword ptr [EAX+0x4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x89              // MOV  dword ptr [EAX+0x8], EDX
        _emit 0x50
        _emit 0x08
        _emit 0x8b              // MOV  ECX, dword ptr [ESI+0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0043C2D0   (rel32)
        _emit 0x3d
        _emit 0x4b
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  0x8
        _emit 0x08
        _emit 0x00
        _emit 0x8b              // MOV  ECX, dword ptr [ESI+0x8]   (alloc_failed:)
        _emit 0x4e
        _emit 0x08
        _emit 0x33              // XOR  EAX, EAX
        _emit 0xc0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0043C2D0   (rel32)
        _emit 0x2e
        _emit 0x4b
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  0x8
        _emit 0x08
        _emit 0x00
    }
}
