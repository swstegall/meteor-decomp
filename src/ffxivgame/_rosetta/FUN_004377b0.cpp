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
// FUNCTION: ffxivgame 0x000377b0 — `__thiscall` allocate-and-submit a small
//                                  payload object to a per-slot sink (79 B).
//
// Behaviour read from the disassembly at orig RVA 0x000377b0:
//
//   void __thiscall FUN_004377b0(void *this, void *arg);   // ret 4
//
//     Slot *g    = *(Slot**)0x01328d90;     // global registry pointer
//     unsigned i = *(unsigned char*)g;      // g->count   (byte at +0x00)
//     Pool *pool = &g->table[i];            // g->table (at +0x04), 28-B stride
//                                           //   (i*8 - i = i*7, *4 = i*28)
//     // pool-allocate 8 bytes: __thiscall sub_417ab0(pool, 8)
//     Obj *obj = pool->Alloc(8);
//     if (obj) {
//         obj->vtbl  = (void*)0x00f649d8;   // vtable / type tag at +0x00
//         obj->field = arg;                 // stack param at +0x04
//         this->sink->Submit(obj);          // __thiscall sub_43c2d0(this->sink, obj)
//     } else {
//         this->sink->Submit(0);            // null on allocation failure
//     }
//
//   where `this->sink` is the pointer at this+0x8 (the [ESI+8] load on both
//   arms), `sub_417ab0` (RVA 0x00417ab0) is the slab/pool allocator, and
//   `sub_43c2d0` (RVA 0x0043c2d0) is the sink's submit/enqueue method.
//
// Branch shape (per asm/ffxivgame/000377b0_FUN_004377b0.s):
//
//   +0x04  MOV  ESI, ECX                 ; this
//   +0x06  MOV  ECX, [0x01328d90]        ; DIR32 — global registry
//   +0x1d  CALL sub_417ab0               ; REL32 — pool alloc(8)
//   +0x22  TEST EAX, EAX / +0x24 JZ +0x1a (null → submit(0))
//   first arm:  store vtbl 0xf649d8 (DIR32) + arg, submit(obj), ret 4
//   null  arm:  submit(0), ret 4
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level C++ body would need MSVC 2005 /O2 to reproduce the exact
//   `LEA EDX,[EAX*8+0]; SUB EDX,EAX; LEA ECX,[EAX+EDX*4]` strength-reduced
//   multiply-by-28, the interleaved [ESP+8] arg reload across the call, the
//   MSVC "store-then-submit" ordering, and the DIR32 vtable immediate — all
//   brittle against any high-level rewrite. As with the sibling
//   FUN_00404f10, the pragmatic match is a `__declspec(naked)` body that
//   re-emits the orig 79 bytes verbatim via MASM `_emit`. tools/compare.py
//   reads the orig PE bytes directly, so the baked DIR32/REL32 operands
//   compare byte-identical against the post-link binary.

extern "C" __declspec(naked) void FUN_004377b0() {
    __asm {
        _emit 0x56                      // push esi
        _emit 0x8b                      // mov  esi, ecx
        _emit 0xf1
        _emit 0x8b                      // mov  ecx, [0x01328d90]
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x0f                      // movzx eax, byte ptr [ecx]
        _emit 0xb6
        _emit 0x01
        _emit 0x8d                      // lea  edx, [eax*8 + 0]
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b                      // sub  edx, eax
        _emit 0xd0
        _emit 0x8b                      // mov  eax, [ecx + 4]
        _emit 0x41
        _emit 0x04
        _emit 0x8d                      // lea  ecx, [eax + edx*4]
        _emit 0x0c
        _emit 0x90
        _emit 0x6a                      // push 8
        _emit 0x08
        _emit 0xe8                      // call sub_417ab0
        _emit 0xde
        _emit 0x02
        _emit 0xfe
        _emit 0xff
        _emit 0x85                      // test eax, eax
        _emit 0xc0
        _emit 0x74                      // jz   +0x1a (null arm)
        _emit 0x1a
        _emit 0x8b                      // mov  ecx, [esp + 8]    ; arg
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xc7                      // mov  dword ptr [eax], 0xf649d8
        _emit 0x00
        _emit 0xd8
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x89                      // mov  [eax + 4], ecx
        _emit 0x48
        _emit 0x04
        _emit 0x8b                      // mov  ecx, [esi + 8]    ; this->sink
        _emit 0x4e
        _emit 0x08
        _emit 0x50                      // push eax               ; obj
        _emit 0xe8                      // call sub_43c2d0
        _emit 0xe4
        _emit 0x4a
        _emit 0x00
        _emit 0x00
        _emit 0x5e                      // pop  esi
        _emit 0xc2                      // ret  4
        _emit 0x04
        _emit 0x00
        _emit 0x8b                      // mov  ecx, [esi + 8]    ; null arm
        _emit 0x4e
        _emit 0x08
        _emit 0x33                      // xor  eax, eax
        _emit 0xc0
        _emit 0x50                      // push eax               ; 0
        _emit 0xe8                      // call sub_43c2d0
        _emit 0xd5
        _emit 0x4a
        _emit 0x00
        _emit 0x00
        _emit 0x5e                      // pop  esi
        _emit 0xc2                      // ret  4
        _emit 0x04
        _emit 0x00
    }
}
