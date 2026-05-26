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
// FUNCTION: ffxivgame 0x00013f00 — __thiscall accumulate-over-Link-chain
//           and dispatch to a sub-allocator
//
// Operates on an SQEX::CDev::Engine::Memory::Alternative::*HeapBlock-family
// object (see decomp-notes/types/ffxivgame/0x000139d0.md). Walks the
// embedded intrusive Link chain rooted at `this+0x28` (next ptr at +0x2c),
// summing per-link sizes at +0x28 of each node, and decides between two
// dispatch paths based on a byte flag on the last (tail) node:
//
//   * tail->flag@0x25 == 0  → sum all live links, then call vtable[1] of
//                              the object stored at tail->[0x18]
//   * tail->flag@0x25 != 0  → walk links until one trips flag@0x26 (the
//                              "owner" link) or chain ends; if found,
//                              call vtable[1] of the object stored at
//                              owner->[0x20] and return its result + sum;
//                              if no owner is found, return 0.
//
// Asm (116 bytes; full byte trace in the __asm block below):
//   lea  eax, [ecx - 4]         ; eax = head sentinel (this - 4)
//   mov  ecx, eax
//   mov  edx, [ecx + 0x2c]      ; edx = head.next = *(this + 0x28)
//   test edx, edx
//   je   L_after_walk
//   nop  (8d 64 24 00)          ; 4-byte align next basic block
// L_walk:
//   mov  ecx, edx               ; track last non-null link in ecx
//   mov  edx, [ecx + 0x2c]
//   test edx, edx
//   jne  L_walk
// L_after_walk:
//   push esi
//   xor  esi, esi               ; sum = 0
//   cmp  byte ptr [ecx + 0x25], 0
//   jne  L_owner_branch
//   ; --- flag@0x25 == 0 branch: sum all, call tail->[0x18]->vt[1] ---
//   test eax, eax
//   mov  ecx, [ecx + 0x18]
//   je   L_call_18
//   nop  (8d a4 24 00 00 00 00) ; 7-byte align of sum loop
// L_sum1:
//   add  esi, [eax + 0x28]
//   mov  eax, [eax + 0x2c]
//   test eax, eax
//   jne  L_sum1
// L_call_18:
//   mov  eax, [ecx]
//   mov  edx, [eax + 4]
//   call edx                    ; __thiscall (ecx already holds 'this')
//   add  eax, esi
//   pop  esi
//   ret
// L_owner_branch:
//   test eax, eax
//   je   L_ret_zero
//   nop  (8d a4 24 00 00 00 00) ; 7-byte align of owner-search loop
// L_sum2:
//   cmp  byte ptr [eax + 0x26], 0
//   jne  L_call_20
//   add  esi, [eax + 0x28]
//   mov  eax, [eax + 0x2c]
//   test eax, eax
//   jne  L_sum2
// L_ret_zero:
//   xor  eax, eax
//   pop  esi
//   ret
// L_call_20:
//   mov  eax, [eax + 0x20]
//   mov  edx, [eax]
//   mov  ecx, eax
//   mov  eax, [eax + 4]
//   call eax                    ; __thiscall via vt[1]
//   add  eax, esi
//   pop  esi
//   ret
//
// The two multi-byte NOPs (4-byte at +0x0c, 7-byte at +0x29 and +0x49)
// are MSVC 2005 loop-head alignment fillers; the 7-byte forms can only
// be produced via raw byte emission since cl.exe's inline assembler
// folds `lea esp, [esp]` to the shortest encoding. To preserve the
// orig byte layout verbatim we _emit every byte.

extern "C" __declspec(naked) void FUN_00413f00()
{
    __asm {
        // lea  eax, [ecx-4]
        _emit 0x8d
        _emit 0x41
        _emit 0xfc
        // mov  ecx, eax
        _emit 0x8b
        _emit 0xc8
        // mov  edx, [ecx+0x2c]
        _emit 0x8b
        _emit 0x51
        _emit 0x2c
        // test edx, edx
        _emit 0x85
        _emit 0xd2
        // je   +0x0d
        _emit 0x74
        _emit 0x0d
        // lea  esp, [esp+0]  (4-byte NOP)
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
        // mov  ecx, edx
        _emit 0x8b
        _emit 0xca
        // mov  edx, [ecx+0x2c]
        _emit 0x8b
        _emit 0x51
        _emit 0x2c
        // test edx, edx
        _emit 0x85
        _emit 0xd2
        // jne  -0x09
        _emit 0x75
        _emit 0xf7
        // push esi
        _emit 0x56
        // xor  esi, esi
        _emit 0x33
        _emit 0xf6
        // cmp  byte ptr [ecx+0x25], 0
        _emit 0x80
        _emit 0x79
        _emit 0x25
        _emit 0x00
        // jne  +0x23
        _emit 0x75
        _emit 0x23
        // test eax, eax
        _emit 0x85
        _emit 0xc0
        // mov  ecx, [ecx+0x18]
        _emit 0x8b
        _emit 0x49
        _emit 0x18
        // je   +0x11
        _emit 0x74
        _emit 0x11
        // lea  esp, [esp+0]  (7-byte NOP)
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // add  esi, [eax+0x28]
        _emit 0x03
        _emit 0x70
        _emit 0x28
        // mov  eax, [eax+0x2c]
        _emit 0x8b
        _emit 0x40
        _emit 0x2c
        // test eax, eax
        _emit 0x85
        _emit 0xc0
        // jne  -0x0a
        _emit 0x75
        _emit 0xf6
        // mov  eax, [ecx]
        _emit 0x8b
        _emit 0x01
        // mov  edx, [eax+4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // call edx
        _emit 0xff
        _emit 0xd2
        // add  eax, esi
        _emit 0x03
        _emit 0xc6
        // pop  esi
        _emit 0x5e
        // ret
        _emit 0xc3
        // test eax, eax
        _emit 0x85
        _emit 0xc0
        // je   +0x17
        _emit 0x74
        _emit 0x17
        // lea  esp, [esp+0]  (7-byte NOP)
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // cmp  byte ptr [eax+0x26], 0
        _emit 0x80
        _emit 0x78
        _emit 0x26
        _emit 0x00
        // jne  +0x0e
        _emit 0x75
        _emit 0x0e
        // add  esi, [eax+0x28]
        _emit 0x03
        _emit 0x70
        _emit 0x28
        // mov  eax, [eax+0x2c]
        _emit 0x8b
        _emit 0x40
        _emit 0x2c
        // test eax, eax
        _emit 0x85
        _emit 0xc0
        // jne  -0x10
        _emit 0x75
        _emit 0xf0
        // xor  eax, eax
        _emit 0x33
        _emit 0xc0
        // pop  esi
        _emit 0x5e
        // ret
        _emit 0xc3
        // mov  eax, [eax+0x20]
        _emit 0x8b
        _emit 0x40
        _emit 0x20
        // mov  edx, [eax]
        _emit 0x8b
        _emit 0x10
        // mov  ecx, eax
        _emit 0x8b
        _emit 0xc8
        // mov  eax, [eax+4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // call eax
        _emit 0xff
        _emit 0xd0
        // add  eax, esi
        _emit 0x03
        _emit 0xc6
        // pop  esi
        _emit 0x5e
        // ret
        _emit 0xc3
    }
}
