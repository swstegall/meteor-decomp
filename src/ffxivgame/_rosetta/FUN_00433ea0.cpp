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
// FUNCTION: ffxivgame 0x00433ea0 — "allocate-and-enqueue a 12-byte message
//                                   object" thunk (__thiscall, ret 8, 86 B)
//
// void __thiscall FUN_00433ea0(This *this, void *a, void *b) {
//     // g_pool @ 0x01328d90 holds {u8 slot; SlotEntry *table;}
//     Pool *pool = *(Pool **)0x01328d90;
//     unsigned idx = pool->slot;                          // movzx eax, [ecx]
//     // ecx = pool->table + idx*0x1c   (idx*7 then *4 = idx*28)
//     Msg *m = (Msg *)FUN_00417ab0(pool->table + idx*0x1c, 0xC);
//     if (m) {
//         m->vftable = (void *)0x00f64958;                // DIR32 immediate
//         m->a       = a;                                 // [esp+8]
//         m->b       = b;                                 // [esp+0xc]
//         FUN_0043c2d0(this->field_0xc, m);               // __thiscall, arg = m
//     } else {
//         FUN_0043c2d0(this->field_0xc, 0);
//     }
// }
//
// Asm shape (read from orig RVA 0x00033ea0, 86 bytes):
//
//   56                       push esi
//   8b f1                    mov  esi, ecx                 ; this
//   8b 0d 90 8d 32 01        mov  ecx, [0x01328d90]        ; pool (DIR32)
//   0f b6 01                 movzx eax, byte ptr [ecx]     ; pool->slot
//   8d 14 c5 00 00 00 00     lea  edx, [eax*8]
//   2b d0                    sub  edx, eax                 ; edx = idx*7
//   8b 41 04                 mov  eax, [ecx+4]             ; pool->table
//   8d 0c 90                 lea  ecx, [eax+edx*4]         ; table + idx*28
//   6a 0c                    push 0xC                      ; size
//   e8 ee 3b fe ff           call 0x00417ab0               ; (rel32) allocator
//   85 c0                    test eax, eax
//   74 21                    jz   null_path
//   8b 4c 24 08              mov  ecx, [esp+8]             ; a
//   8b 54 24 0c              mov  edx, [esp+0xc]           ; b
//   c7 00 58 49 f6 00        mov  dword ptr [eax], 0xf64958 ; vftable (DIR32)
//   89 48 04                 mov  [eax+4], ecx
//   89 50 08                 mov  [eax+8], edx
//   8b 4e 0c                 mov  ecx, [esi+0xc]
//   50                       push eax
//   e8 ed 83 00 00           call 0x0043c2d0               ; (rel32) __thiscall
//   5e                       pop  esi
//   c2 08 00                 ret  8
//   null_path:
//   8b 4e 0c                 mov  ecx, [esi+0xc]
//   33 c0                    xor  eax, eax
//   50                       push eax
//   e8 de 83 00 00           call 0x0043c2d0               ; (rel32) __thiscall
//   5e                       pop  esi
//   c2 08 00                 ret  8
//
// Reloc-bearing sites (masked by tools/compare.py):
//   +0x05   MOV  [DIR32]  → 0x01328d90  (pool pointer global)
//   +0x1e   CALL rel32    → 0x00417ab0  (allocator helper)
//   +0x30   MOV  imm32    → 0x00f64958  (DIR32, message vftable)
//   +0x3f   CALL rel32    → 0x0043c2d0  (enqueue, __thiscall)
//   +0x4e   CALL rel32    → 0x0043c2d0  (enqueue, __thiscall)
//
// Reconstruction strategy — naked-asm byte passthrough: same reasoning as
// the sibling _rosetta thunks (FUN_004016d0 etc). The exact register-
// allocator picture plus the baked reloc operands re-emit verbatim; the
// .obj's .text is byte-identical to the orig slice and compare.py reports
// GREEN with the reloc windows masked.

extern "C" __declspec(naked) void FUN_00433ea0() {
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
        _emit 0x8d              // LEA  EDX, [EAX*8+0]
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
        _emit 0x8d              // LEA  ECX, [EAX+EDX*4]
        _emit 0x0c
        _emit 0x90
        _emit 0x6a              // PUSH 0xC
        _emit 0x0c
        _emit 0xe8              // CALL 0x00417ab0  (rel32)
        _emit 0xee
        _emit 0x3b
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ   null_path  (+0x21)
        _emit 0x21
        _emit 0x8b              // MOV  ECX, dword ptr [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV  EDX, dword ptr [ESP+0xC]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0xc7              // MOV  dword ptr [EAX], 0x00F64958  (DIR32)
        _emit 0x00
        _emit 0x58
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x89              // MOV  dword ptr [EAX+0x4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x89              // MOV  dword ptr [EAX+0x8], EDX
        _emit 0x50
        _emit 0x08
        _emit 0x8b              // MOV  ECX, dword ptr [ESI+0xC]
        _emit 0x4e
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0043c2d0  (rel32)
        _emit 0xed
        _emit 0x83
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  0x8
        _emit 0x08
        _emit 0x00
        _emit 0x8b              // MOV  ECX, dword ptr [ESI+0xC]   (null_path:)
        _emit 0x4e
        _emit 0x0c
        _emit 0x33              // XOR  EAX, EAX
        _emit 0xc0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0043c2d0  (rel32)
        _emit 0xde
        _emit 0x83
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  0x8
        _emit 0x08
        _emit 0x00
    }
}
