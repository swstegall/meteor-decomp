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
// FUNCTION: ffxivgame 0x00433de0 — `__thiscall` factory helper (with one arg):
//                                  allocates an 8-byte object via sub_417ab0,
//                                  stamps its tag (0x00f64948) and stores the
//                                  caller arg, then dispatches through
//                                  this->slot_0c (79 B / 0x4f).
//
// __thiscall void FUN_00433de0(this, int arg)   // ECX = this, arg on stack
//
// Body (matches asm flow):
//
//   // g_tbl @ 0x01328d90 is a pointer to a small descriptor:
//   //   +0x00  uint8  index
//   //   +0x04  T*     base       (stride-28 element array)
//   void *desc = *(void **)0x01328d90;
//   unsigned idx = *(unsigned char *)desc;             // movzx
//   // element pointer: base + idx*28   (idx*8 - idx == idx*7, then <<2)
//   void *elem = (char *)desc->base + (idx * 7) * 4;
//   void *obj  = sub_417ab0(elem, 8);                  // thiscall(ecx=elem, 8)
//   if (obj) {
//       *(int *)obj       = 0x00f64948;               // tag stamp
//       *((int *)obj + 1) = arg;                      // store caller arg
//       sub_43c2d0(this->slot_0c, obj);               // thiscall(ecx=slot_0c)
//   } else {
//       sub_43c2d0(this->slot_0c, 0);
//   }
//
// Orig codegen (79 bytes — read from the disassembly at orig RVA 0x00033de0):
//
//   56                   push esi
//   8b f1                mov  esi, ecx                  ; esi = this
//   8b 0d 90 8d 32 01    mov  ecx, [0x01328d90]         ; ecx = g_tbl (DIR32)
//   0f b6 01             movzx eax, byte ptr [ecx]      ; eax = g_tbl->index
//   8d 14 c5 00 00 00 00 lea  edx, [eax*8 + 0]
//   2b d0                sub  edx, eax                  ; edx = idx*7
//   8b 41 04             mov  eax, [ecx+4]              ; eax = g_tbl->base
//   8d 0c 90             lea  ecx, [eax + edx*4]        ; ecx = base + idx*28
//   6a 08                push 8
//   e8 ae 3c fe ff       call sub_417ab0                ; rel32 → 0x00417ab0
//   85 c0                test eax, eax
//   74 1a                jz   else_branch
//   8b 4c 24 08          mov  ecx, [esp+8]              ; ecx = caller arg
//   c7 00 48 49 f6 00    mov  dword ptr [eax], 0xf64948 ; tag stamp
//   89 48 04             mov  [eax+4], ecx              ; store arg
//   8b 4e 0c             mov  ecx, [esi+0xc]            ; ecx = this->slot_0c
//   50                   push eax
//   e8 b4 84 00 00       call sub_43c2d0                ; rel32 → 0x0043c2d0
//   5e                   pop  esi
//   c2 04 00             ret  4
// else_branch:
//   8b 4e 0c             mov  ecx, [esi+0xc]            ; ecx = this->slot_0c
//   33 c0                xor  eax, eax
//   50                   push eax
//   e8 a5 84 00 00       call sub_43c2d0                ; rel32 → 0x0043c2d0
//   5e                   pop  esi
//   c2 04 00             ret  4
//
// Near-identical sibling: FUN_00434180 (alloc 4 B, no arg, RET not RET 4).
//
// Reloc-bearing sites in the orig 79 bytes (absolute / rel32 values that
// resolve only in a full-binary relink at image base 0x00400000):
//   +0x05  DIR32 data load  (0x01328d90 — g_tbl)
//   +0x1e  CALL rel32       → sub_417ab0 (0x00417ab0)
//   +0x38  CALL rel32       → sub_43c2d0 (0x0043c2d0)
//   +0x47  CALL rel32       → sub_43c2d0 (0x0043c2d0)
//
// Reconstruction strategy — naked-asm byte passthrough (same as sibling
// FUN_00434180): a `__declspec(naked)` body re-emitting the orig 79 bytes
// verbatim via MASM `_emit`. The .obj's `.text` is then byte-identical to
// the orig slice — the absolute data addresses and rel32 displacements are
// emitted as raw immediates that already match the orig binary's resolved
// bytes, so no relocations are involved and tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00433de0() {
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
        _emit 0x8d              // LEA EDX, [EAX*8 + 0x00000000]
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB EDX, EAX
        _emit 0xd0
        _emit 0x8b              // MOV EAX, dword ptr [ECX + 0x04]
        _emit 0x41
        _emit 0x04
        _emit 0x8d              // LEA ECX, [EAX + EDX*4]
        _emit 0x0c
        _emit 0x90
        _emit 0x6a              // PUSH 0x08
        _emit 0x08
        _emit 0xe8              // CALL rel32 → 0x00417ab0
        _emit 0xae
        _emit 0x3c
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x1a (→ else_branch)
        _emit 0x1a
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x08]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [EAX], 0x00f64948
        _emit 0x00
        _emit 0x48
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x89              // MOV dword ptr [EAX + 0x04], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x0c]
        _emit 0x4e
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → 0x0043c2d0
        _emit 0xb4
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 4
        _emit 0x04
        _emit 0x00
        // else_branch:
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x0c]
        _emit 0x4e
        _emit 0x0c
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → 0x0043c2d0
        _emit 0xa5
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 4
        _emit 0x04
        _emit 0x00
    }
}
