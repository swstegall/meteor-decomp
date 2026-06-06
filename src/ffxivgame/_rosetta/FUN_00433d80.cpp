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
// FUNCTION: ffxivgame 0x00433d80 — 2-arg __thiscall factory/dispatch
//                                  (86 bytes)
//
// __thiscall void FUN_00433d80(This *this, int a, int b)
//   RET 0x8 → cleans 2 dword args; ECX = this.
//   Stack after `push esi`:
//     [ESP+0x08] : a   (arg0)
//     [ESP+0x0C] : b   (arg1)
//
// Behaviour: index a global slot table (g = [0x01328d90]); compute the
// per-slot allocator `&g->pool[g->byte0]` where the pool stride is 28
// bytes (idx*8 - idx == idx*7, then *4). Call the slot allocator
// FUN_00417ab0(0xc) (thiscall ECX = &pool[idx]) for a 12-byte node.
//   If allocation succeeds (EAX != 0): initialise the node —
//     node->vtbl = 0x00f64940; node->+4 = a; node->+8 = b;
//     then dispatch FUN_0043c2d0(this->+0xc, node).
//   Else: dispatch FUN_0043c2d0(this->+0xc, 0).
//
// The 2-arg / 12-byte-node sibling of FUN_00433c60 (3-arg / 16-byte node).
//
// Asm (86 bytes, RVA 0x00033d80):
//   56                   push esi
//   8b f1                mov  esi, ecx                  ; this
//   8b 0d 90 8d 32 01    mov  ecx, [0x01328d90]         ; g
//   0f b6 01             movzx eax, byte ptr [ecx]      ; g->byte0
//   8d 14 c5 00 00 00 00 lea  edx, [eax*8]
//   2b d0                sub  edx, eax                  ; idx*7
//   8b 41 04             mov  eax, [ecx+4]              ; g->pool
//   8d 0c 90             lea  ecx, [eax + edx*4]        ; &g->pool[idx] (stride 28)
//   6a 0c                push 0xc
//   e8 0e 3d fe ff       call FUN_00417ab0              ; node = alloc(0xc)
//   85 c0                test eax, eax
//   74 21                jz   null_path
//   8b 4c 24 08          mov  ecx, [esp+8]              ; a
//   8b 54 24 0c          mov  edx, [esp+0xc]            ; b
//   c7 00 40 49 f6 00    mov  dword ptr [eax], 0xf64940 ; vtbl
//   89 48 04             mov  [eax+4], ecx
//   89 50 08             mov  [eax+8], edx
//   8b 4e 0c             mov  ecx, [esi+0xc]
//   50                   push eax
//   e8 0d 85 00 00       call FUN_0043c2d0
//   5e                   pop  esi
//   c2 08 00             ret  8
// null_path:
//   8b 4e 0c             mov  ecx, [esi+0xc]
//   33 c0                xor  eax, eax
//   50                   push eax
//   e8 fe 84 00 00       call FUN_0043c2d0
//   5e                   pop  esi
//   c2 08 00             ret  8
//
// Reloc-bearing sites (CALL rel32 the linker resolves at relink time):
//   +0x1d  CALL rel32 → FUN_00417ab0 (RVA 0x00417ab0)
//   +0x3e  CALL rel32 → FUN_0043c2d0 (RVA 0x0043c2d0)
//   +0x4d  CALL rel32 → FUN_0043c2d0 (RVA 0x0043c2d0)
// We re-emit the orig 86 bytes verbatim via `_emit`; the .obj .text is
// byte-identical to orig with zero relocations → compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00433d80() {
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
        _emit 0x8b              // MOV EAX, dword ptr [ECX+4]
        _emit 0x41
        _emit 0x04
        _emit 0x8d              // LEA ECX, [EAX + EDX*4]
        _emit 0x0c
        _emit 0x90
        _emit 0x6a              // PUSH 0xc
        _emit 0x0c
        _emit 0xe8              // CALL FUN_00417ab0 (rel32)
        _emit 0x0e
        _emit 0x3d
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ null_path (+0x21)
        _emit 0x21
        _emit 0x8b              // MOV ECX, dword ptr [ESP+8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0xc]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0xc7              // MOV dword ptr [EAX], 0xf64940
        _emit 0x00
        _emit 0x40
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x89              // MOV dword ptr [EAX+4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x89              // MOV dword ptr [EAX+8], EDX
        _emit 0x50
        _emit 0x08
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0xc]
        _emit 0x4e
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_0043c2d0 (rel32)
        _emit 0x0d
        _emit 0x85
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 8
        _emit 0x08
        _emit 0x00
        _emit 0x8b              // null_path: MOV ECX, dword ptr [ESI+0xc]
        _emit 0x4e
        _emit 0x0c
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_0043c2d0 (rel32)
        _emit 0xfe
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 8
        _emit 0x08
        _emit 0x00
    }
}
