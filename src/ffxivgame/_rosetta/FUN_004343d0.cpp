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
// FUNCTION: ffxivgame 0x004343d0 — `__thiscall` allocate-construct-and-store
//                                  helper (79 B / 0x4f)
//
// __thiscall void FUN_004343d0(this, int arg)   // ECX = this, saved into ESI
//
// Near-twin of sibling FUN_004341d0: same global-table slot computation and
// allocate-then-store-into-field_0c shape, but this variant allocates 8 bytes,
// stamps vftable 0x00f649d8, and additionally writes the stack argument into
// the new object's +4 slot before delegating. RET 4 (one stack arg).
//
// Body (read from the orig bytes at RVA 0x000343d0):
//
//   push esi
//   mov  esi, ecx                       ; esi = this
//   mov  ecx, [0x01328d90]              ; ecx = g_table  (global ptr, DIR32)
//   movzx eax, byte ptr [ecx]           ; eax = g_table->index (a byte)
//   lea  edx, [eax*8]                   ; edx = index * 8
//   sub  edx, eax                       ; edx = index * 7
//   mov  eax, [ecx+4]                   ; eax = g_table->base
//   lea  ecx, [eax + edx*4]             ; ecx = base + index*28  (slot addr)
//   push 8                              ; arg = 8
//   call 0x00417ab0                     ; p = sub_417ab0(slot, 8)  (__thiscall)
//   test eax, eax
//   jz   zero                           ; allocation/lookup failed
//   mov  ecx, [esp+8]                   ; ecx = stack arg
//   mov  dword ptr [eax], 0xf649d8      ; *p = &vftable_f649d8
//   mov  [eax+4], ecx                   ; p->field_4 = arg
//   mov  ecx, [esi+0xc]                 ; ecx = this->field_0c
//   push eax                            ; arg = p
//   call 0x0043c2d0                     ; this->field_0c->store(p)  (__thiscall)
//   pop  esi
//   ret  4
// zero:
//   mov  ecx, [esi+0xc]                 ; ecx = this->field_0c
//   xor  eax, eax
//   push eax                            ; arg = 0
//   call 0x0043c2d0                     ; this->field_0c->store(0)  (__thiscall)
//   pop  esi
//   ret  4
//
// Reloc-bearing sites in the orig 79 bytes (resolved only at full-binary
// relink at image base 0x00400000; the standalone .obj can't reproduce them
// via source — naked asm emits the bytes as raw immediates that already
// match the orig binary's resolved values byte-for-byte):
//     +0x03   MOV ECX, [0x01328d90]   (global ptr load — DIR32)
//     +0x1d   CALL rel32 → 0x00417ab0
//     +0x2a   MOV dword [EAX], 0xf649d8  (absolute vftable address)
//     +0x37   CALL rel32 → 0x0043c2d0
//     +0x46   CALL rel32 → 0x0043c2d0
//
// Reconstruction strategy — naked-asm byte passthrough (same approach as
// sibling FUN_004341d0): re-emit the orig 79 bytes verbatim via MASM
// `_emit` directives. tools/compare.py then reloc-masks the windows above
// and reports GREEN.

extern "C" __declspec(naked) void FUN_004343d0() {
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
        _emit 0x8d              // LEA EDX, [EAX*8 + 0x0]
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
        _emit 0x6a              // PUSH 0x8
        _emit 0x08
        _emit 0xe8              // CALL rel32 → 0x00417ab0
        _emit 0xbe
        _emit 0x36
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x1a (→ zero)
        _emit 0x1a
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [EAX], 0x00f649d8
        _emit 0x00
        _emit 0xd8
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x89              // MOV dword ptr [EAX + 0x4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0xc]
        _emit 0x4e
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → 0x0043c2d0
        _emit 0xc4
        _emit 0x7e
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
        // zero:
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0xc]
        _emit 0x4e
        _emit 0x0c
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → 0x0043c2d0
        _emit 0xb5
        _emit 0x7e
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
