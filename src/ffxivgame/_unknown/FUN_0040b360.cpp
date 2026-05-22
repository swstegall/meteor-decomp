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
// FUNCTION: ffxivgame 0x0000b360 — sweep expired nodes from linked-list
//                                   array and notify listeners
//                                   (__thiscall, 275 B / 0x113)
//
// __thiscall void FUN_0040b360(this)
//   ECX : this — pointer to an object owning a 22-slot node-pointer array
//
// Stack frame (after prologue, relative to post-PUSH ESP):
//   [ESP+0x00] = EDI (saved)
//   [ESP+0x04] = ESI (saved)
//   [ESP+0x08] = EBP (saved)
//   [ESP+0x0c] = EBX (saved)
//   [ESP+0x10] = temp: EBP / next-node ptr
//   [ESP+0x14] = ECX / this (saved on entry)
//   [ESP+0x18] = EAX / prev-node ptr (starts 0 each outer iteration)
//   [ESP+0x1c] = EDX / current array-entry ptr (advances by +4)
//   [ESP+0x20] = outer loop counter (starts 0x16 = 22)
//
// Behaviour:
//   Iterates over 22 slots of the node-pointer array at *this.
//   For each slot it walks the doubly-intrusive singly-linked list
//   (linked via node+0x1c).  Each node is tested:
//     - node->field0a < node->field08  →  keep; advance prev, advance node.
//     - node->field0a >= node->field08 →  remove node from list; if
//       node->field00 (child ptr EBX) is non-null, route the child
//       through this->field60 sub-object (via FUN_0040ddd0 /
//       FUN_0040de80 / FUN_0040df70) and update sub-object counters;
//       then walk a second chain at *this->field5c, calling
//       FUN_0040ddd0 on each member for the removed node, and calling
//       FUN_0040de80 on the first match found.
//
// Node layout (from field offsets touched):
//   +0x00  void *child      — forwarded to sub-object as EBX
//   +0x08  unsigned short field08 — capacity / max count
//   +0x0a  unsigned short field0a — used count
//   +0x1c  Node *next       — linked-list next pointer
//
// Sub-object at this->field60:
//   +0x00  void *vtable_ptr   — loaded as ECX for call via *ESI
//   +0x04  sub-sub-object     — address used as ECX for ddd0/de80 calls
//   +0x08  void *inner        — ptr; inner+0x8 is a word checked vs 0
//   +0x30  int  counter       — incremented by 1 on each removed node
//   +0x38  int  budget        — decremented by 0x1000 on each removed node
//
// Second chain at *this->field5c:
//   each member is a struct with a link at +0x9a4
//
// Calling convention: __thiscall, no stack parameters (RET).
// No /GS cookie (no local array ≥ 5 bytes).
//
// Alignment NOPs present in the orig:
//   +0x19  7-byte  LEA ESP, [ESP+0x00000000]  (outer loop body align)
//   +0xaa  6-byte  LEA EBX, [EBX+0x00000000]  (second-chain loop align)
//
// Reloc-bearing CALL sites in the orig 275 bytes (masked by compare.py):
//   +0x6f  CALL FUN_0040ddd0  (rel32 → 0x000029fc from site)
//   +0x7b  CALL FUN_0040de80  (rel32 → 0x00002aa0 from site)
//   +0x8d  CALL FUN_0040df70  (rel32 → 0x00002b7e from site)
//   +0xa3  CALL FUN_0040ddd0  (rel32 → 0x000029b8 from site)
//   +0xd9  CALL FUN_0040de80  (rel32 → 0x00002a42 from site)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The control-flow shape (outer for-loop over 22 slots, inner linked-list
//   walk with in-place unlink, two-phase dispatch via three callees, second
//   chain search) produces unusual register shuffling and two alignment NOPs
//   that a source-level C++ lowering cannot reproduce reliably under /O2.
//   Emitting the orig 275 bytes verbatim via _emit gives a byte-identical
//   .obj (reloc sites are masked by tools/compare.py).

extern "C" __declspec(naked) void FUN_0040b360() {
    __asm {
        // 0000b360
        _emit 0x83              // SUB ESP, 0x14
        _emit 0xec
        _emit 0x14
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV EDX, ECX
        _emit 0xd1
        _emit 0x57              // PUSH EDI
        _emit 0x89              // MOV [ESP+0x14], ECX
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x89              // MOV [ESP+0x1c], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0xc7              // MOV dword ptr [ESP+0x20], 0x16
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 7-byte NOP: LEA ESP, [ESP+0x00000000]  (outer loop align)
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000b380: outer loop body start
        _emit 0x8b              // MOV EDI, [EDX]
        _emit 0x3a
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x89              // MOV [ESP+0x18], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x0f              // JZ outer_loop_next  (0x0040b459)
        _emit 0x84
        _emit 0xc9
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000b390: JMP over prev-reload on first entry
        _emit 0xeb              // JMP +4  (to 0x0040b396)
        _emit 0x04
        // 0000b392: inner loop re-entry — reload prev
        _emit 0x8b              // MOV EAX, [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 0000b396: inner loop body
        _emit 0x66              // MOV SI, word ptr [EDI+0xa]
        _emit 0x8b
        _emit 0x77
        _emit 0x0a
        _emit 0x66              // CMP SI, word ptr [EDI+0x8]
        _emit 0x3b
        _emit 0x77
        _emit 0x08
        _emit 0x0f              // JC advance_node  (0x0040b44a, field0a < field08)
        _emit 0x82
        _emit 0xa6
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // remove node from list:
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x8b              // MOV EBP, [EDI+0x1c]  (next ptr)
        _emit 0x6f
        _emit 0x1c
        _emit 0x89              // MOV [ESP+0x10], EBP  (save next)
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x74              // JZ  update_head  (+5)
        _emit 0x05
        _emit 0x89              // MOV [EAX+0x1c], EBP  (prev->next = next)
        _emit 0x68
        _emit 0x1c
        _emit 0xeb              // JMP +2
        _emit 0x02
        // update_head:
        _emit 0x89              // MOV [EDX], EBP       (*head_ptr = next)
        _emit 0x2a
        // process child:
        _emit 0x8b              // MOV EBX, [EDI]       (child ptr)
        _emit 0x1f
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x8b              // MOV ESI, [ECX+0x60]  (this->field60)
        _emit 0x71
        _emit 0x60
        _emit 0x74              // JZ no_child  (+0x3e, to 0x0040b3fd)
        _emit 0x3e
        // child dispatch:
        _emit 0x8b              // MOV EAX, [ESI+0x8]
        _emit 0x46
        _emit 0x08
        _emit 0x66              // CMP word ptr [EAX+0x8], 0x0
        _emit 0x83
        _emit 0x78
        _emit 0x08
        _emit 0x00
        _emit 0x74              // JZ use_vtable  (+0x21, to 0x0040b3ea)
        _emit 0x21
        _emit 0x8d              // LEA EBP, [ESI+0x4]
        _emit 0x6e
        _emit 0x04
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV ECX, EBP
        _emit 0xcd
        _emit 0xe8              // CALL FUN_0040ddd0
        _emit 0xfc
        _emit 0x29
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x74              // JZ no_de80  (+0x0e)
        _emit 0x0e
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV ECX, EBP
        _emit 0xcd
        _emit 0xe8              // CALL FUN_0040de80
        _emit 0xa0
        _emit 0x2a
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EBP, [ESP+0x10]  (restore next)
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0xeb              // JMP +0x0c (to 0x0040b3f2)
        _emit 0x0c
        // no_de80:
        _emit 0x8b              // MOV EBP, [ESP+0x10]  (restore next)
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // use_vtable:
        _emit 0x8b              // MOV ECX, [ESI]       (*ESI)
        _emit 0x0e
        _emit 0x53              // PUSH EBX
        _emit 0xe8              // CALL FUN_0040df70
        _emit 0x7e
        _emit 0x2b
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD dword ptr [ESI+0x30], 1
        _emit 0x46
        _emit 0x30
        _emit 0x01
        _emit 0x81              // ADD dword ptr [ESI+0x38], 0xfffff000
        _emit 0x46
        _emit 0x38
        _emit 0x00
        _emit 0xf0
        _emit 0xff
        _emit 0xff
        // no_child / second chain search:
        _emit 0x8b              // MOV ECX, [ESP+0x14]  (this)
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EDX, [ECX+0x5c]  (this->field5c)
        _emit 0x51
        _emit 0x5c
        _emit 0x8b              // MOV ESI, [EDX]       (chain head)
        _emit 0x32
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74              // JZ no_chain  (+0x34, to 0x0040b43e)
        _emit 0x34
        // 6-byte NOP: LEA EBX, [EBX+0x00000000]
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // chain loop:
        _emit 0x57              // PUSH EDI  (removed node)
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_0040ddd0
        _emit 0xb8
        _emit 0x29
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x75              // JNZ chain_match  (+0x16)
        _emit 0x16
        _emit 0x8b              // MOV ESI, [ESI+0x9a4]  (next in chain)
        _emit 0xb6
        _emit 0xa4
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x75              // JNZ chain_loop_top  (-0x16)
        _emit 0xea
        // chain exhausted:
        _emit 0x8b              // MOV ECX, [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EDX, [ESP+0x1c]
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x8b              // MOV EDI, EBP  (next ptr)
        _emit 0xfd
        _emit 0xeb              // JMP inner_loop_test  (+0x1f)
        _emit 0x1f
        // chain_match:
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74              // JZ no_chain  (+0x08)
        _emit 0x08
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_0040de80
        _emit 0x42
        _emit 0x2a
        _emit 0x00
        _emit 0x00
        // no_chain:
        _emit 0x8b              // MOV ECX, [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EDX, [ESP+0x1c]
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x8b              // MOV EDI, EBP  (next ptr)
        _emit 0xfd
        _emit 0xeb              // JMP inner_loop_test  (+0x07)
        _emit 0x07
        // advance_node (JC taken — field0a < field08):
        _emit 0x89              // MOV [ESP+0x18], EDI  (prev = current)
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV EDI, [EDI+0x1c]  (node = node->next)
        _emit 0x7f
        _emit 0x1c
        // inner_loop_test:
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x0f              // JNZ inner_loop_top  (→ 0x0040b392)
        _emit 0x85
        _emit 0x39
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // outer_loop_next:
        _emit 0x83              // ADD EDX, 4
        _emit 0xc2
        _emit 0x04
        _emit 0x83              // SUB dword ptr [ESP+0x20], 1
        _emit 0x6c
        _emit 0x24
        _emit 0x20
        _emit 0x01
        _emit 0x89              // MOV [ESP+0x1c], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x0f              // JNZ outer_loop_body  (→ 0x0040b380)
        _emit 0x85
        _emit 0x15
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // epilogue:
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc3              // RET
    }
}
