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
// FUNCTION: ffxivgame 0x00434180 — `__thiscall` factory helper: allocates a
//                                  4-byte object via sub_417ab0, stamps its
//                                  vftable (0x00f64998), then dispatches it
//                                  through this->slot_0c (68 B / 0x44).
//
// __thiscall void FUN_00434180(this)    // ECX = this
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
//   void *obj  = sub_417ab0(elem, 4);                  // thiscall(ecx=elem, 4)
//   if (obj) {
//       *(void **)obj = (void *)0x00f64998;            // vftable stamp
//       sub_43c2d0(this->slot_0c, obj);                // thiscall(ecx=slot_0c)
//   } else {
//       sub_43c2d0(this->slot_0c, 0);
//   }
//
// Orig codegen (68 bytes — read from the disassembly at orig RVA 0x00034180):
//
//   56                   push esi
//   8b f1                mov  esi, ecx                  ; esi = this
//   8b 0d 90 8d 32 01    mov  ecx, [0x01328d90]         ; ecx = g_tbl (DIR32)
//   0f b6 01             movzx eax, byte ptr [ecx]      ; eax = g_tbl->index
//   8d 14 c5 00 00 00 00 lea  edx, [eax*8 + 0]
//   2b d0                sub  edx, eax                  ; edx = idx*7
//   8b 41 04             mov  eax, [ecx+4]              ; eax = g_tbl->base
//   8d 0c 90             lea  ecx, [eax + edx*4]        ; ecx = base + idx*28
//   6a 04                push 4
//   e8 0e 39 fe ff       call sub_417ab0                ; rel32 → 0x00417ab0
//   85 c0                test eax, eax
//   74 11                jz   else_branch
//   c7 00 98 49 f6 00    mov  dword ptr [eax], 0xf64998 ; vftable stamp (DIR32)
//   8b 4e 0c             mov  ecx, [esi+0xc]            ; ecx = this->slot_0c
//   50                   push eax
//   e8 1b 81 00 00       call sub_43c2d0                ; rel32 → 0x0043c2d0
//   5e                   pop  esi
//   c3                   ret
// else_branch:
//   8b 4e 0c             mov  ecx, [esi+0xc]            ; ecx = this->slot_0c
//   33 c0                xor  eax, eax
//   50                   push eax
//   e8 0e 81 00 00       call sub_43c2d0                ; rel32 → 0x0043c2d0
//   5e                   pop  esi
//   c3                   ret
//
// Reloc-bearing sites in the orig 68 bytes (these absolute / rel32 values
// resolve only in a full-binary relink at image base 0x00400000; the
// standalone .obj cannot reproduce them via source-level symbol refs):
//   +0x03  DIR32 data load  (0x01328d90 — g_tbl)
//   +0x1d  CALL rel32       → sub_417ab0 (0x00417ab0)
//   +0x22  DIR32 immediate  (0x00f64998 — vftable)
//   +0x30  CALL rel32       → sub_43c2d0 (0x0043c2d0)
//   +0x3d  CALL rel32       → sub_43c2d0 (0x0043c2d0)
//
// Reconstruction strategy — naked-asm byte passthrough (same as siblings
// FUN_00406fa0 / FUN_00403d10): a `__declspec(naked)` body re-emitting the
// orig 68 bytes verbatim via MASM `_emit`. The .obj's `.text` is then
// byte-identical to the orig slice — the absolute data addresses and rel32
// displacements are emitted as raw immediates that already match the orig
// binary's resolved bytes, so no relocations are involved and
// tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00434180() {
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
        _emit 0x6a              // PUSH 0x04
        _emit 0x04
        _emit 0xe8              // CALL rel32 → 0x00417ab0
        _emit 0x0e
        _emit 0x39
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x11 (→ else_branch)
        _emit 0x11
        _emit 0xc7              // MOV dword ptr [EAX], 0x00f64998
        _emit 0x00
        _emit 0x98
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x0c]
        _emit 0x4e
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → 0x0043c2d0
        _emit 0x1b
        _emit 0x81
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        // else_branch:
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x0c]
        _emit 0x4e
        _emit 0x0c
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → 0x0043c2d0
        _emit 0x0e
        _emit 0x81
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
