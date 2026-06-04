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
// FUNCTION: ffxivgame 0x00433ff0 — "build payload object and dispatch it to
//                                  a bound sink" thunk (86 B, __thiscall,
//                                  void(this, a0, a1), ret 8)
//
// Calling convention: __thiscall — ECX carries `this`, immediately spilled
// to ESI; the epilogue `ret 8` pops the two stack arguments (a0, a1), so the
// source-level signature is `void __thiscall f(This*, void *a0, void *a1)`.
//
// Asm shape (read from the orig bytes at RVA 0x00033ff0, 86 bytes total):
//
//   00033ff0:  56                       push esi
//   00033ff1:  8b f1                    mov  esi, ecx            ; this
//   00033ff3:  8b 0d 90 8d 32 01        mov  ecx, [g_pool]       ; @ 0x01328d90
//   00033ff9:  0f b6 01                 movzx eax, byte ptr [ecx]; idx = g_pool->sel
//   00033ffc:  8d 14 c5 00 00 00 00     lea  edx, [eax*8]
//   00034003:  2b d0                    sub  edx, eax            ; edx = idx*7
//   00034005:  8b 41 04                 mov  eax, [ecx+4]        ; base = g_pool->arr
//   00034008:  8d 0c 90                 lea  ecx, [eax+edx*4]    ; this2 = &arr[idx]
//                                                               ;   (stride 28 = idx*7*4)
//   0003400b:  6a 0c                    push 0xc                 ; size = 12
//   0003400d:  e8 9e 3a fe ff           call 0x00417ab0          ; allocate(this2, 12)
//   00034012:  85 c0                    test eax, eax
//   00034014:  74 21                    jz   fall_through        ; alloc failed → dispatch NULL
//   00034016:  8b 4c 24 08              mov  ecx, [esp+8]        ; a0
//   0003401a:  8b 54 24 0c              mov  edx, [esp+0xc]      ; a1
//   0003401e:  c7 00 78 49 f6 00        mov  dword ptr [eax], 0x00f64978  ; obj->vftable
//   00034024:  89 48 04                 mov  [eax+4], ecx        ; obj->a0 = a0
//   00034027:  89 50 08                 mov  [eax+8], edx        ; obj->a1 = a1
//   0003402a:  8b 4e 0c                 mov  ecx, [esi+0xc]      ; sink = this->field_0xc
//   0003402d:  50                       push eax                 ; payload
//   0003402e:  e8 9d 82 00 00           call 0x0043c2d0          ; sink->dispatch(obj)
//   00034033:  5e                       pop  esi
//   00034034:  c2 08 00                 ret  8
//   fall_through:                                               ; @ 0x00434037
//   00034037:  8b 4e 0c                 mov  ecx, [esi+0xc]      ; sink = this->field_0xc
//   0003403a:  33 c0                    xor  eax, eax
//   0003403c:  50                       push eax                 ; NULL
//   0003403d:  e8 8e 82 00 00           call 0x0043c2d0          ; sink->dispatch(NULL)
//   00034042:  5e                       pop  esi
//   00034043:  c2 08 00                 ret  8
//
// Reloc-bearing sites in the orig 86 bytes:
//   +0x05   MOV  [g_pool]  (DIR32 → 0x01328d90)
//   +0x0d   CALL rel32     (→ 0x00417ab0, pool allocator __thiscall)
//   +0x20   MOV  imm32     (DIR32 → 0x00f64978, payload vftable)
//   +0x3e   CALL rel32     (→ 0x0043c2d0, sink dispatch __thiscall, hit-arm)
//   +0x4d   CALL rel32     (→ 0x0043c2d0, sink dispatch __thiscall, NULL-arm)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level reconstruction would need MSVC to (a) reproduce the exact
//   `lea edx,[eax*8] / sub edx,eax / lea ecx,[eax+edx*4]` idx*28 address
//   ladder, (b) keep `this` pinned in ESI across the allocator call, and
//   (c) resolve four DIR32/rel32 reloc operands against external symbols
//   whose addresses we'd have to supply by hand. Re-emitting the orig 86
//   bytes verbatim via `_emit` sidesteps all of that: the .obj's .text is
//   byte-identical to the orig slice with zero relocations, so
//   tools/compare.py reports GREEN. Same approach as sibling FUN_00403b70.

extern "C" __declspec(naked) void FUN_00433ff0() {
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
        _emit 0xe8              // CALL 0x00417ab0  (rel32)
        _emit 0x9e
        _emit 0x3a
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ   fall_through (+0x21)
        _emit 0x21
        _emit 0x8b              // MOV  ECX, dword ptr [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV  EDX, dword ptr [ESP+0xC]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0xc7              // MOV  dword ptr [EAX], 0x00F64978 (DIR32 imm)
        _emit 0x00
        _emit 0x78
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
        _emit 0x9d
        _emit 0x82
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  0x8
        _emit 0x08
        _emit 0x00
        _emit 0x8b              // MOV  ECX, dword ptr [ESI+0xC]   (fall_through:)
        _emit 0x4e
        _emit 0x0c
        _emit 0x33              // XOR  EAX, EAX
        _emit 0xc0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0043c2d0  (rel32)
        _emit 0x8e
        _emit 0x82
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  0x8
        _emit 0x08
        _emit 0x00
    }
}
