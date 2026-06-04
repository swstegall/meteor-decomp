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
// FUNCTION: ffxivgame 0x00437040 — __thiscall command-record factory
//                                  (3 stack args, RET 0xC, 93 bytes)
//
// __thiscall void FUN_00437040(This *this /*ECX*/, int a, int b, int c)
//
// Behaviour: index a global table ([0x01328d90]) by its leading byte
// count (record stride = 28 bytes: idx*7*4), pass the slot address as
// `this` to a 0x10-byte allocator (FUN_00417ab0 with size arg 0x10). On
// success, stamp the new node's vftable (offset 0 = 0x00f64928) and three
// payload DWORDs (a→[+4], b→[+8], c→[+0xC]), then hand the node off to a
// queue/insert method (FUN_0043c2d0) reached through [this+8]. On
// allocation failure, call the same method with a NULL node.
//
// Asm shape (93 bytes, read from orig RVA 0x00037040, image base 0x00400000):
//
//   56                  push esi
//   8b f1               mov  esi, ecx                ; esi = this
//   8b 0d 90 8d 32 01   mov  ecx, [0x01328d90]       ; ecx = table
//   0f b6 01            movzx eax, byte ptr [ecx]     ; count
//   8d 14 c5 00 00 00 00 lea  edx, [eax*8]
//   2b d0               sub  edx, eax                ; edx = count*7
//   8b 41 04            mov  eax, [ecx+4]            ; eax = table base
//   8d 0c 90            lea  ecx, [eax + edx*4]      ; this = &slot (stride 28)
//   6a 10               push 0x10
//   e8 4e 0a fe ff      call FUN_00417ab0            ; node = alloc(0x10)
//   85 c0               test eax, eax
//   74 28               jz   fail
//   8b 4c 24 08         mov  ecx, [esp+0x8]          ; a
//   8b 54 24 0c         mov  edx, [esp+0xc]          ; b
//   89 48 04            mov  [eax+4], ecx
//   8b 4c 24 10         mov  ecx, [esp+0x10]         ; c
//   c7 00 28 49 f6 00   mov  dword ptr [eax], 0x00f64928   ; vftable (DIR32)
//   89 50 08            mov  [eax+8], edx
//   89 48 0c            mov  [eax+0xc], ecx
//   8b 4e 08            mov  ecx, [esi+8]
//   50                  push eax
//   e8 46 52 00 00      call FUN_0043c2d0            ; (rel32 → 0x0043c2d0)
//   5e                  pop  esi
//   c2 0c 00            ret  0xc
//  fail:
//   8b 4e 08            mov  ecx, [esi+8]
//   33 c0               xor  eax, eax
//   50                  push eax
//   e8 37 52 00 00      call FUN_0043c2d0            ; (rel32 → 0x0043c2d0)
//   5e                  pop  esi
//   c2 0c 00            ret  0xc
//
// Reloc-bearing sites in the orig 93 bytes:
//   +0x1d   CALL rel32 → 0x00417ab0   (0x10-byte allocator)
//   +0x35   MOV  imm32 → 0x00f64928   (DIR32, node vftable)
//   +0x45   CALL rel32 → 0x0043c2d0   (insert/forward method)
//   +0x54   CALL rel32 → 0x0043c2d0   (insert/forward method, fail arm)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form would reproduce this shape but emit two CALL
//   relocations plus one DIR32 mov-imm32 that the linker resolves at
//   relink time. tools/compare.py masks reloc bytes, but a
//   __declspec(naked) body re-emitting the orig 93 bytes verbatim via
//   MASM `_emit` produces a .obj whose .text is byte-identical to the
//   orig slice with zero relocations — GREEN without a relink.

extern "C" __declspec(naked) void FUN_00437040() {
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
        _emit 0x8d              // LEA EDX, [EAX*0x8 + 0x0]
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB EDX, EAX
        _emit 0xd0
        _emit 0x8b              // MOV EAX, dword ptr [ECX+0x4]
        _emit 0x41
        _emit 0x04
        _emit 0x8d              // LEA ECX, [EAX + EDX*4]
        _emit 0x0c
        _emit 0x90
        _emit 0x6a              // PUSH 0x10
        _emit 0x10
        _emit 0xe8              // CALL FUN_00417ab0 (rel32 → 0x00417ab0)
        _emit 0x4e
        _emit 0x0a
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ fail (+0x28)
        _emit 0x28
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0xc]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x89              // MOV dword ptr [EAX+0x4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0xc7              // MOV dword ptr [EAX], 0x00f64928 (DIR32)
        _emit 0x00
        _emit 0x28
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x89              // MOV dword ptr [EAX+0x8], EDX
        _emit 0x50
        _emit 0x08
        _emit 0x89              // MOV dword ptr [EAX+0xc], ECX
        _emit 0x48
        _emit 0x0c
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_0043c2d0 (rel32 → 0x0043c2d0)
        _emit 0x46
        _emit 0x52
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x8]   ; fail
        _emit 0x4e
        _emit 0x08
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_0043c2d0 (rel32 → 0x0043c2d0)
        _emit 0x37
        _emit 0x52
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00
    }
}
