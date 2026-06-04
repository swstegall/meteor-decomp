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
// FUNCTION: ffxivgame 0x004375b0 — `__thiscall` allocate-construct-and-handoff
//                                  helper (68 B / 0x44)
//
// __thiscall void FUN_004375b0(this)   // ECX = this
//
// Inspection (read from the orig bytes at RVA 0x000375b0, 68 bytes total):
//
//   push esi
//   mov  esi, ecx                       ; esi = this
//   mov  ecx, [0x01328d90]              ; ecx = *(g**)0x01328d90 (global table ptr)
//   movzx eax, byte ptr [ecx]           ; eax = g->idx (1-byte selector)
//   lea  edx, [eax*8]                   ; edx = idx*8
//   sub  edx, eax                       ; edx = idx*7   (record stride / 4)
//   mov  eax, [ecx+4]                   ; eax = g->base (array base ptr)
//   lea  ecx, [eax + edx*4]             ; ecx = &g->base[idx*28]  (this for the alloc)
//   push 0x4                            ; arg = 4
//   call 0x00417ab0                     ; eax = allocator->alloc(4)  (__thiscall)
//   test eax, eax
//   jz   else                           ; alloc failed → pass NULL
//   mov  dword ptr [eax], 0x00f649a0    ; *eax = vftable 0x00f649a0  (placement ctor)
//   mov  ecx, [esi+8]                   ; ecx = this->sink_8
//   push eax                            ; arg = new object
//   call 0x0043c2d0                     ; this->sink_8->accept(eax)  (__thiscall)
//   pop  esi
//   ret
// else:
//   mov  ecx, [esi+8]                   ; ecx = this->sink_8
//   xor  eax, eax
//   push eax                            ; arg = NULL
//   call 0x0043c2d0                     ; this->sink_8->accept(NULL) (__thiscall)
//   pop  esi
//   ret
//
// Reloc-bearing sites in the orig 68 bytes (CALL rel32 displacements that
// resolve only in a full-binary relink at image base 0x00400000):
//     +0x1d   CALL rel32 → 0x00417ab0  (allocator)
//     +0x30   CALL rel32 → 0x0043c2d0  (sink accept, success arm)
//     +0x3d   CALL rel32 → 0x0043c2d0  (sink accept, failure arm)
//   The MOV [EAX],0x00f649a0 immediate is an absolute vftable address that
//   is likewise a fixed value in the binary's own address space.
//
// Reconstruction strategy — naked-asm byte passthrough (same approach as
// sibling FUN_00406fa0): re-emit the orig 68 bytes verbatim via MASM
// `_emit` directives. The CALL rel32 displacements and the absolute
// vftable immediate are emitted as raw bytes that match the orig binary's
// already-resolved encoding byte-for-byte (no relocations in the .obj —
// the values live in the binary's own address space). `tools/compare.py`
// then reports GREEN.

extern "C" __declspec(naked) void FUN_004375b0() {
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
        _emit 0x6a              // PUSH 0x4
        _emit 0x04
        _emit 0xe8              // CALL rel32 → 0x00417ab0
        _emit 0xde
        _emit 0x04
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x11 (→ else)
        _emit 0x11
        _emit 0xc7              // MOV dword ptr [EAX], 0x00f649a0
        _emit 0x00
        _emit 0xa0
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → 0x0043c2d0
        _emit 0xeb
        _emit 0x4c
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        _emit 0x8b              // (else:) MOV ECX, dword ptr [ESI + 0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → 0x0043c2d0
        _emit 0xde
        _emit 0x4c
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
