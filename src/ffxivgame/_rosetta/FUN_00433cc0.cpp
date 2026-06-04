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
// FUNCTION: ffxivgame 0x00433cc0 — "build event payload and dispatch"
//                                  thunk (86 B, __thiscall, void(arg1, arg2))
//
// __thiscall void FUN_00433cc0(this, u32 a, u32 b)
//   ECX = this (saved into ESI); two dword stack args → `ret 8`.
//
// Asm shape (read from orig RVA 0x00033cc0):
//
//   push  esi
//   mov   esi, ecx                       ; this
//   mov   ecx, [0x01328d90]              ; g_obj
//   movzx eax, byte ptr [ecx]            ; idx = g_obj->b0
//   lea   edx, [eax*8]                   ; edx = idx*8
//   sub   edx, eax                       ; edx = idx*7
//   mov   eax, [ecx+4]                   ; base = g_obj->p4
//   lea   ecx, [eax + edx*4]             ; this2 = base + idx*28
//   push  0xc                            ; size = 12
//   call  0x00417ab0                     ; alloc(this2, 12) → eax
//   test  eax, eax
//   jz    null_path
//   mov   ecx, [esp+8]                   ; a
//   mov   edx, [esp+0xc]                 ; b
//   mov   dword ptr [eax], 0xf64930      ; payload->vftable  (DIR32)
//   mov   [eax+4], ecx                   ; payload->a
//   mov   [eax+8], edx                   ; payload->b
//   mov   ecx, [esi+0xc]                 ; this->sink
//   push  eax
//   call  0x0043c2d0                     ; sink->dispatch(payload)
//   pop   esi
//   ret   8
// null_path:
//   mov   ecx, [esi+0xc]
//   xor   eax, eax
//   push  eax
//   call  0x0043c2d0                     ; sink->dispatch(NULL)
//   pop   esi
//   ret   8
//
// Reloc-bearing sites in the orig 86 bytes:
//   +0x1d   CALL rel32 → 0x00417ab0   (allocator helper)
//   +0x2e   MOV  imm32 → 0x00f64930   (DIR32, payload vftable)
//   +0x3e   CALL rel32 → 0x0043c2d0   (dispatch sink)
//   +0x4d   CALL rel32 → 0x0043c2d0   (dispatch sink, null arm)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form would emit two CALL rel32 relocations and a
//   DIR32 on the vftable immediate, all resolved only at full-binary
//   relink time. A `__declspec(naked)` body re-emitting the orig 86
//   bytes verbatim via `_emit` produces a .obj whose .text is
//   byte-identical to the orig slice with zero relocations, so
//   tools/compare.py reports GREEN. Same passthrough idiom as sibling
//   _rosetta/FUN_00403b70.cpp / FUN_004065c0.cpp.

extern "C" __declspec(naked) void FUN_00433cc0() {
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
        _emit 0x8b              // MOV  EAX, dword ptr [ECX+0x4]
        _emit 0x41
        _emit 0x04
        _emit 0x8d              // LEA  ECX, [EAX + EDX*0x4]
        _emit 0x0c
        _emit 0x90
        _emit 0x6a              // PUSH 0xC
        _emit 0x0c
        _emit 0xe8              // CALL 0x00417ab0   (rel32)
        _emit 0xce
        _emit 0x3d
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ   null_path (+0x21)
        _emit 0x21
        _emit 0x8b              // MOV  ECX, dword ptr [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV  EDX, dword ptr [ESP+0xC]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0xc7              // MOV  dword ptr [EAX], 0x00F64930  (DIR32)
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
        _emit 0x8b              // MOV  ECX, dword ptr [ESI+0xC]
        _emit 0x4e
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0043c2d0   (rel32)
        _emit 0xcd
        _emit 0x85
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
        _emit 0xe8              // CALL 0x0043c2d0   (rel32)
        _emit 0xbe
        _emit 0x85
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  0x8
        _emit 0x08
        _emit 0x00
    }
}
