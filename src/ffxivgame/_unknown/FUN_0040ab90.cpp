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
// FUNCTION: ffxivgame 0x0000ab90 — sorted linked-list insert
//                                   (__thiscall, 66 B / 0x42)
//
// __thiscall void insert_sorted(this, Node *node, int index)
//   ECX        : this  — pointer to an array of Node* heads
//   [ESP+0x04] : Node *node  — the node to insert
//   [ESP+0x08] : int   index — which bucket/slot in the array
//
// Memory layout of Node (inferred from +0x1c offset):
//   struct Node { /* ... 0x1c bytes ... */ Node *next; };
//
// Behaviour:
//   Inserts `node` into the sorted (ascending by pointer address) singly-
//   linked list stored at this->arr[index].  If the list is empty, sets
//   arr[index] = node.  Otherwise, walks the list to find the insertion
//   point and splices node in (updating arr[index] if inserting at head).
//
// Calling convention: __thiscall, callee cleans 2 stack args (RET 0x8).
// Registers saved: EDI (across whole function), ESI (only in non-null path).
// No /GS cookie (no local array >= 5 bytes).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ at /O2 produces an inverted branch layout (JNZ instead
//   of the original JZ, and a 2-byte NOP loop-alignment pad) because MSVC
//   2005 places the null-head "early return" path before the main loop.
//   The original binary has the null path at the end (JZ forward to 0x37).
//   The naked-asm passthrough reproduces the exact wire image byte-for-byte
//   with no relocations — the function is pure computation (no CALL, no
//   absolute data references).

extern "C" __declspec(naked) void FUN_0040ab90() {
    __asm {
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0xC]
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x8b              // MOV EAX, dword ptr [ECX + EDI*4]
        _emit 0x04
        _emit 0xb9
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x2b  -> null head case (offset 0x37)
        _emit 0x2b
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0xC]
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x33              // XOR EDX, EDX        (prev = 0)
        _emit 0xd2
        _emit 0x3b              // CMP ESI, EAX        (loop top, offset 0x13)
        _emit 0xf0
        _emit 0x72              // JC +0x11 -> insert_before (offset 0x28)
        _emit 0x11
        _emit 0x8b              // MOV EDX, EAX        (prev = cur)
        _emit 0xd0
        _emit 0x8b              // MOV EAX, dword ptr [EAX+0x1c] (cur = cur->next)
        _emit 0x40
        _emit 0x1c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ -0x0d -> loop top (0x13)
        _emit 0xf3
        // end-of-list: prev->next = node
        _emit 0x89              // MOV dword ptr [EDX+0x1c], ESI
        _emit 0x72
        _emit 0x1c
        _emit 0x5e              // POP ESI
        _emit 0x5f              // POP EDI
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        // insert_before: (offset 0x28) — node < cur
        _emit 0x85              // TEST EDX, EDX       (is prev == NULL?)
        _emit 0xd2
        _emit 0x89              // MOV dword ptr [ESI+0x1c], EAX  (node->next = cur)
        _emit 0x46
        _emit 0x1c
        _emit 0x75              // JNZ -0x0f -> prev->next = node (offset 0x20)
        _emit 0xf1
        // prev == NULL: update head
        _emit 0x89              // MOV dword ptr [ECX + EDI*4], ESI  (arr[index] = node)
        _emit 0x34
        _emit 0xb9
        _emit 0x5e              // POP ESI
        _emit 0x5f              // POP EDI
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        // null head case: (offset 0x37)
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x8]  (param_1, after 1 PUSH)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ECX + EDI*4], EAX  (arr[index] = node)
        _emit 0x04
        _emit 0xb9
        _emit 0x5f              // POP EDI
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
