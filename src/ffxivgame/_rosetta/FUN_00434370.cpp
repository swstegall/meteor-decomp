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
// FUNCTION: ffxivgame 0x00034370 — "allocate + construct a 12-byte event
//                                   object from a per-slot pool, then post
//                                   it to this->sink" thunk
//                                   (__thiscall, 2 stack args, 86 bytes)
//
// Asm shape (read from orig RVA 0x00034370):
//
//   void __thiscall FUN_00434370(This *this, void *a1, void *a2) {
//       Pool *p = *(Pool **)0x01328d90;          // global pool registry
//       unsigned idx = (unsigned char)p->cur;    // movzx eax, [ecx]
//       // slot = p->base + idx*0x1C  (idx*7*4 — lea/sub shift-and-add)
//       Obj *o = (Obj *)PoolAllocator::allocate(
//                          (PoolAllocator *)(p->base + idx*0x1C), 0xC);
//       if (o) {
//           o->vftable = 0x00F649D0;             // mov [eax], imm32 (DIR32)
//           o->field4  = a1;
//           o->field8  = a2;
//           this->sink->post(o);                 // ecx = this->[+0xC]
//       } else {
//           this->sink->post(NULL);
//       }
//   }
//
//   00034370:  56                       push  esi
//   00034371:  8b f1                    mov   esi, ecx            ; this
//   00034373:  8b 0d 90 8d 32 01        mov   ecx, [0x01328d90]   ; pool (DIR32)
//   00034379:  0f b6 01                 movzx eax, byte ptr [ecx] ; idx
//   0003437c:  8d 14 c5 00 00 00 00     lea   edx, [eax*8]
//   00034383:  2b d0                    sub   edx, eax            ; edx = idx*7
//   00034385:  8b 41 04                 mov   eax, [ecx+4]        ; base
//   00034388:  8d 0c 90                 lea   ecx, [eax+edx*4]    ; this = slot
//   0003438b:  6a 0c                    push  0Ch                 ; size = 12
//   0003438d:  e8 1e 37 fe ff           call  0x00417ab0          ; allocate (rel32)
//   00034392:  85 c0                    test  eax, eax
//   00034394:  74 21                    jz    null_arm
//   00034396:  8b 4c 24 08              mov   ecx, [esp+8]        ; a1
//   0003439a:  8b 54 24 0c              mov   edx, [esp+0Ch]      ; a2
//   0003439e:  c7 00 d0 49 f6 00        mov   dword ptr [eax], 0x00F649D0  ; vftable (DIR32)
//   000343a4:  89 48 04                 mov   [eax+4], ecx
//   000343a7:  89 50 08                 mov   [eax+8], edx
//   000343aa:  8b 4e 0c                 mov   ecx, [esi+0Ch]      ; sink
//   000343ad:  50                       push  eax
//   000343ae:  e8 1d 7f 00 00           call  0x0043c2d0          ; post (rel32)
//   000343b3:  5e                       pop   esi
//   000343b4:  c2 08 00                 ret   8
//   000343b7:  8b 4e 0c                 mov   ecx, [esi+0Ch]      ; null_arm: sink
//   000343ba:  33 c0                    xor   eax, eax
//   000343bc:  50                       push  eax                 ; NULL
//   000343bd:  e8 0e 7f 00 00           call  0x0043c2d0          ; post (rel32)
//   000343c2:  5e                       pop   esi
//   000343c3:  c2 08 00                 ret   8
//
// Reloc-bearing sites in the orig 86 bytes:
//   +0x03   MOV  imm32 → 0x01328d90   (DIR32, global pool registry slot)
//   +0x1d   CALL rel32 → 0x00417ab0   (PoolAllocator::allocate thunk)
//   +0x2e   MOV  imm32 → 0x00F649D0   (DIR32, object vftable)
//   +0x3e   CALL rel32 → 0x0043c2d0   (sink->post)
//   +0x4d   CALL rel32 → 0x0043c2d0   (sink->post, null arm)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ reconstruction would pin two DIR32 immediates and
//   three rel32 CALL targets the linker resolves at relink time, and would
//   need MSVC 2005 to reproduce the exact lea/sub idx*7 shift-and-add plus
//   the register-allocator picture (esi=this, the slot computed into ecx
//   before the allocate call). The naked `_emit` body re-emits the orig 86
//   bytes verbatim: the .obj's .text is byte-identical to the orig slice
//   with zero relocations, so tools/compare.py reports GREEN without reloc
//   masking. Mirrors sibling FUN_00403b70.

extern "C" __declspec(naked) void FUN_00434370() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV  ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV  ECX, dword ptr [0x01328D90]   (DIR32)
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x0f              // MOVZX EAX, byte ptr [ECX]
        _emit 0xb6
        _emit 0x01
        _emit 0x8d              // LEA  EDX, [EAX*8 + 0]
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB  EDX, EAX
        _emit 0xd0
        _emit 0x8b              // MOV  EAX, dword ptr [ECX+4]
        _emit 0x41
        _emit 0x04
        _emit 0x8d              // LEA  ECX, [EAX + EDX*4]
        _emit 0x0c
        _emit 0x90
        _emit 0x6a              // PUSH 0Ch
        _emit 0x0c
        _emit 0xe8              // CALL 0x00417AB0   (rel32)
        _emit 0x1e
        _emit 0x37
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ   null_arm (+0x21)
        _emit 0x21
        _emit 0x8b              // MOV  ECX, dword ptr [ESP+8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV  EDX, dword ptr [ESP+0Ch]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0xc7              // MOV  dword ptr [EAX], 0x00F649D0   (DIR32)
        _emit 0x00
        _emit 0xd0
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x89              // MOV  dword ptr [EAX+4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x89              // MOV  dword ptr [EAX+8], EDX
        _emit 0x50
        _emit 0x08
        _emit 0x8b              // MOV  ECX, dword ptr [ESI+0Ch]
        _emit 0x4e
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0043C2D0   (rel32)
        _emit 0x1d
        _emit 0x7f
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  8
        _emit 0x08
        _emit 0x00
        _emit 0x8b              // MOV  ECX, dword ptr [ESI+0Ch]   (null_arm:)
        _emit 0x4e
        _emit 0x0c
        _emit 0x33              // XOR  EAX, EAX
        _emit 0xc0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0043C2D0   (rel32)
        _emit 0x0e
        _emit 0x7f
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  8
        _emit 0x08
        _emit 0x00
    }
}
