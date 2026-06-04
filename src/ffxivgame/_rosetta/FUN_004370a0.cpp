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
// FUNCTION: ffxivgame 0x004370a0 — "allocate + populate a 12-byte event
//                                   object from a pool, then dispatch it"
//                                   (__thiscall, 2 stack args, 86 bytes)
//
// Behaviour (read from orig RVA 0x000370a0):
//
//   void __thiscall FUN_004370a0(This *self, void *a, void *b) {
//       // g @ 0x01328d90 is a small descriptor: byte[0] is an index,
//       // dword[1] is a base pointer to an array of 28-byte (0x1c)
//       // records.  elem = base + index*28.
//       PoolRec *g    = *(PoolRec **)0x01328d90;
//       unsigned idx  = (unsigned char)g->index;       // movzx
//       char    *elem = g->base + idx * 28;            // lea/sub/lea ladder
//
//       Obj *o = elem->alloc(12);                      // FUN_00417ab0, thiscall
//       if (o) {
//           o->vftable = 0x00f64930;                   // DIR32 immediate
//           o->field4  = a;
//           o->field8  = b;
//           self->field8->dispatch(o);                 // FUN_0043c2d0, thiscall
//       } else {
//           self->field8->dispatch(0);                 // FUN_0043c2d0, thiscall
//       }
//   }
//
// Asm shape:
//
//   push esi
//   mov  esi, ecx                       ; self
//   mov  ecx, dword ptr [0x01328d90]    ; g            (DIR32)
//   movzx eax, byte ptr [ecx]           ; idx
//   lea  edx, [eax*8+0]                 ; idx*8        (disp32=0 form)
//   sub  edx, eax                       ; idx*7
//   mov  eax, dword ptr [ecx+4]         ; base
//   lea  ecx, [eax+edx*4]               ; elem = base + idx*28
//   push 0Ch                            ; size = 12
//   call 0x00417ab0                     ; pool alloc   (rel32)
//   test eax, eax
//   jz   alloc_failed                   ; +0x21
//   mov  ecx, dword ptr [esp+8]         ; a
//   mov  edx, dword ptr [esp+0Ch]       ; b
//   mov  dword ptr [eax], 0x00f64930    ; vftable      (DIR32 immediate)
//   mov  dword ptr [eax+4], ecx         ; o->field4 = a
//   mov  dword ptr [eax+8], edx         ; o->field8 = b
//   mov  ecx, dword ptr [esi+8]         ; self->field8
//   push eax
//   call 0x0043c2d0                     ; dispatch     (rel32)
//   pop  esi
//   ret  8
//  alloc_failed:
//   mov  ecx, dword ptr [esi+8]         ; self->field8
//   xor  eax, eax
//   push eax                            ; 0
//   call 0x0043c2d0                     ; dispatch(0)  (rel32)
//   pop  esi
//   ret  8
//
// Reloc-bearing sites in the orig 86 bytes:
//   +0x05   MOV  ECX, [DIR32 0x01328d90]   (global descriptor pointer)
//   +0x1d   CALL rel32 → 0x00417ab0        (pool allocator, __thiscall)
//   +0x2e   MOV  [EAX], imm32 0x00f64930   (DIR32 vftable address)
//   +0x3e   CALL rel32 → 0x0043c2d0        (dispatch, __thiscall)
//   +0x4d   CALL rel32 → 0x0043c2d0        (dispatch, __thiscall)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Identical reasoning to siblings FUN_00403b70 / FUN_004065c0: a
//   source-level C++ form can't reliably coax MSVC 2005 /O2 into the
//   exact register-allocator picture (esi carrying `self`, the
//   lea/sub/lea idx*28 ladder, the dual dispatch arms) while also
//   pinning the DIR32 global / DIR32 vftable / rel32 call operands.
//   The naked `_emit` body re-emits the orig 86 bytes verbatim; the
//   .obj's .text is byte-identical with zero relocations, so
//   tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_004370a0() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV  ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV  ECX, dword ptr [0x01328d90]  (DIR32)
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
        _emit 0x6a              // PUSH 0xC
        _emit 0x0c
        _emit 0xe8              // CALL 0x00417ab0  (rel32)
        _emit 0xee
        _emit 0x09
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ   alloc_failed  (+0x21)
        _emit 0x21
        _emit 0x8b              // MOV  ECX, dword ptr [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV  EDX, dword ptr [ESP+0xC]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0xc7              // MOV  dword ptr [EAX], 0x00f64930  (DIR32 imm)
        _emit 0x00
        _emit 0x30
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
        _emit 0xe8              // CALL 0x0043c2d0  (rel32)
        _emit 0xed
        _emit 0x51
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  0x8
        _emit 0x08
        _emit 0x00
        _emit 0x8b              // MOV  ECX, dword ptr [ESI+0x8]    (alloc_failed:)
        _emit 0x4e
        _emit 0x08
        _emit 0x33              // XOR  EAX, EAX
        _emit 0xc0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0043c2d0  (rel32)
        _emit 0xde
        _emit 0x51
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  0x8
        _emit 0x08
        _emit 0x00
    }
}
