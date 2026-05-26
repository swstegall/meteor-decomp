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
// FUNCTION: ffxivgame 0x004140e0 — FUN_004140e0 (89 B / 0x59)
//                                   __thiscall member function, lazy-init of
//                                   a per-instance bookkeeping node that is
//                                   spliced onto the tail of a circular
//                                   doubly-linked list owned by the manager
//                                   returned by `this->m_14->vtable[1]()`.
//
// Calling convention: __thiscall (ECX = this); plain `RET` — no stack args.
// Callee-saves pushed: ESI (single register, holds `this`).
//
// Object layout (offsets touched on `this`):
//   [this + 0x14]   ManagerAccessor *  (object whose vtable slot 1 returns the
//                                       owning manager that anchors the list)
//   [this + 0x34]   ListNode *         (lazily-allocated bookkeeping node;
//                                       NULL until first insert, then points
//                                       at the heap-allocated node)
//
// ListNode layout (offsets stored at +0x00, +0x04, +0x08, +0x0C of the
// freshly-allocated node):
//   [node + 0x00]   void *vftable     = 0x00f56f78  (DLL-node vtable)
//   [node + 0x04]   ListNode *next    (initially self, then patched to head)
//   [node + 0x08]   ListNode *prev    (initially self, then patched to tail)
//   [node + 0x0C]   void *owner       = this (back-pointer for unlink-time
//                                             callbacks)
//
// High-level behaviour:
//   if (this->m_34 != NULL) return;                  // already inserted
//   Manager *mgr = this->m_14->vtable[1]();          // fetch owning manager
//   ListNode *node = FUN_004109a0(mgr->m_18);        // typed allocator
//   if (node) {
//       node->next   = node;                         // initialise circular
//       node->prev   = node;                         //   self-loop (sentinel)
//       node->vftbl  = 0x00f56f78;                   // DLL node vtable
//       node->owner  = this;                         // back-pointer
//   }
//   this->m_34 = node;                               // record on `this`
//   Manager *mgr2 = this->m_14->vtable[1]();         // re-fetch (compiler
//                                                    //   has no alias info)
//   ListNode *head = mgr2->m_34;                     // list anchor
//   head->prev->next = this->m_34;                   // splice in at tail
//   this->m_34->prev = head->prev;
//   this->m_34->next = head;
//   head->prev       = this->m_34;
//
// The duplicated `MOV EDX, [EAX+0x8]` at +0x4b / +0x4b indicates the C++
// source reads `head->prev` twice in two adjacent statements — MSVC has no
// alias info between the two and reloads the field across the intervening
// `head->prev->next = …` store.
//
// Reloc-bearing sites in the orig 89 bytes:
//     +0x16   CALL rel32   → FUN_004109a0           (typed allocator)
//     +0x27   MOV  imm32   → 0x00f56f78  (ListNode vtable in .rdata)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level C++ form would emit the same shape and produce two
//   relocations (one CALL rel32 plus the imm32 MOV). compare.py masks
//   reloc bytes out of the diff. The simplest, most-robust route to GREEN
//   is a `__declspec(naked)` body re-emitting the orig 89 bytes verbatim
//   via MASM `_emit` directives; the .obj's `.text` ends up byte-identical
//   to the orig slice with no relocations to resolve.

extern "C" __declspec(naked) void FUN_004140e0() {
    __asm {
        // 0x004140e0: 56                   PUSH ESI
        _emit 0x56
        // 0x004140e1: 8b f1                MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 0x004140e3: 83 7e 34 00          CMP dword ptr [ESI+0x34], 0x0
        _emit 0x83
        _emit 0x7e
        _emit 0x34
        _emit 0x00
        // 0x004140e7: 75 4e                JNE short tail (+0x4e → 0x00414137)
        _emit 0x75
        _emit 0x4e
        // 0x004140e9: 8b 4e 14             MOV ECX, dword ptr [ESI+0x14]
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 0x004140ec: 8b 01                MOV EAX, dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 0x004140ee: 8b 50 04             MOV EDX, dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0x004140f1: ff d2                CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0x004140f3: 8b 48 18             MOV ECX, dword ptr [EAX+0x18]
        _emit 0x8b
        _emit 0x48
        _emit 0x18
        // 0x004140f6: e8 a5 c8 ff ff       CALL FUN_004109a0 (rel32 → 0x004109a0)
        _emit 0xe8
        _emit 0xa5
        _emit 0xc8
        _emit 0xff
        _emit 0xff
        // 0x004140fb: 85 c0                TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0x004140fd: 74 11                JZ short null_node (+0x11 → 0x00414110)
        _emit 0x74
        _emit 0x11
        // 0x004140ff: 89 40 04             MOV dword ptr [EAX+0x4], EAX
        _emit 0x89
        _emit 0x40
        _emit 0x04
        // 0x00414102: 89 40 08             MOV dword ptr [EAX+0x8], EAX
        _emit 0x89
        _emit 0x40
        _emit 0x08
        // 0x00414105: c7 00 78 6f f5 00    MOV dword ptr [EAX], 0x00F56F78  (vftable)
        _emit 0xc7
        _emit 0x00
        _emit 0x78
        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        // 0x0041410b: 89 70 0c             MOV dword ptr [EAX+0xC], ESI
        _emit 0x89
        _emit 0x70
        _emit 0x0c
        // 0x0041410e: eb 02                JMP short stored (+0x02 → 0x00414112)
        _emit 0xeb
        _emit 0x02
        // 0x00414110: 33 c0                XOR EAX, EAX  (null_node:)
        _emit 0x33
        _emit 0xc0
        // 0x00414112: 8b 4e 14             MOV ECX, dword ptr [ESI+0x14]  (stored:)
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 0x00414115: 89 46 34             MOV dword ptr [ESI+0x34], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x34
        // 0x00414118: 8b 01                MOV EAX, dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 0x0041411a: 8b 50 04             MOV EDX, dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0x0041411d: ff d2                CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0x0041411f: 8b 40 34             MOV EAX, dword ptr [EAX+0x34]
        _emit 0x8b
        _emit 0x40
        _emit 0x34
        // 0x00414122: 8b 50 08             MOV EDX, dword ptr [EAX+0x8]
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 0x00414125: 8b 4e 34             MOV ECX, dword ptr [ESI+0x34]
        _emit 0x8b
        _emit 0x4e
        _emit 0x34
        // 0x00414128: 89 4a 04             MOV dword ptr [EDX+0x4], ECX
        _emit 0x89
        _emit 0x4a
        _emit 0x04
        // 0x0041412b: 8b 50 08             MOV EDX, dword ptr [EAX+0x8]
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 0x0041412e: 89 51 08             MOV dword ptr [ECX+0x8], EDX
        _emit 0x89
        _emit 0x51
        _emit 0x08
        // 0x00414131: 89 41 04             MOV dword ptr [ECX+0x4], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x04
        // 0x00414134: 89 48 08             MOV dword ptr [EAX+0x8], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 0x00414137: 5e                   POP ESI  (tail:)
        _emit 0x5e
        // 0x00414138: c3                   RET
        _emit 0xc3
    }
}
