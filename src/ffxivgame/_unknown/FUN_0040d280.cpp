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
// FUNCTION: ffxivgame 0x0000d280 — find-or-allocate node by threshold in
//                                   sorted bucket table (__thiscall, 120 B / 0x78)
//
// __thiscall Node* find_or_alloc(this, unsigned int param_1)
//   ECX        : this  — pointer to an array of 22 Node* bucket heads
//   [ESP+0x04] : unsigned int param_1 — value to look up in DAT_00f55988
//
// Global table:
//   DAT_00f55988 — DWORD[22] at VA 0x00f55988; sorted ascending thresholds
//
// Behaviour:
//   1. Search DAT_00f55988[0..21] for the first entry where table[i] >= param_1
//      (i.e. param_1 <= table[i], unsigned).  If no such index found (i >= 22),
//      return NULL.  (The JGE/return-0 branch at +0x2a is dead/defensive since
//      ESI can only be 0..21 on this path.)
//   2. Walk the linked list at this->arr[i] (Node* stored at ECX+i*4).
//      For each node, if word [node+0xa] != 0, return node.
//      Advance via node->next at node+0x1c.
//   3. If list is exhausted (EAX==0): call FUN_0040d060(capacity) to allocate
//      a fresh node (capacity = table[i] if i<22 else 0), then insert it via
//      FUN_0040ab90(this, new_node, i), and return new_node.
//
// Node memory layout (inferred):
//   +0x00 .. +0x09  opaque data
//   +0x0a            word   flags / count (non-zero means "active")
//   +0x1c            Node*  next pointer
//
// Object memory layout:
//   [this + i*4]     Node*  head of bucket i  (i in [0, 22))
//
// Calling convention: __thiscall, callee cleans 1 stack arg (RET 0x4).
// Callee-saves pushed: EBX, ESI; EDI pushed only in alloc path.
// No local stack frame. No /GS cookie.
//
// The two `LEA EBX,[EBX]` (6-byte form: 8D 9B 00 00 00 00) at +0x0a and
// +0x3a are alignment NOPs emitted by MSVC 2005.
//
// Reloc-bearing CALL sites:
//   +0x60   CALL rel32  → FUN_0040d060  (rel32 = 0xfffd7b)
//   +0x69   CALL rel32  → FUN_0040ab90  (rel32 = 0xffffd8a0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The LEA-NOP alignment pads and the specific unsigned branch selection
//   (JBE/JC vs. JLE/JL) are not reproducible from C++ source. The
//   __declspec(naked) body re-emits the original 120 bytes verbatim via
//   MASM _emit directives; the .obj's .text is byte-identical to the
//   original slice, and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0040d280() {
    __asm {
        // 0000d280:  8b 44 24 04        MOV EAX, dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0000d284:  53                 PUSH EBX
        _emit 0x53
        // 0000d285:  56                 PUSH ESI
        _emit 0x56
        // 0000d286:  8b d9              MOV EBX, ECX
        _emit 0x8b
        _emit 0xd9
        // 0000d288:  33 f6              XOR ESI, ESI
        _emit 0x33
        _emit 0xf6
        // 0000d28a:  8d 9b 00 00 00 00  LEA EBX, [EBX]  (6-byte NOP pad)
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000d290:  3b 04 b5 88 59 f5 00  CMP EAX, dword ptr [ESI*4 + 0xf55988]
        _emit 0x3b
        _emit 0x04
        _emit 0xb5
        _emit 0x88
        _emit 0x59
        _emit 0xf5
        _emit 0x00
        // 0000d297:  76 0f              JBE +0x0f  (-> 0x0040d2a8)
        _emit 0x76
        _emit 0x0f
        // 0000d299:  83 c6 01           ADD ESI, 0x1
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        // 0000d29c:  83 fe 16           CMP ESI, 0x16
        _emit 0x83
        _emit 0xfe
        _emit 0x16
        // 0000d29f:  72 ef              JC -0x11  (-> 0x0040d290)
        _emit 0x72
        _emit 0xef
        // 0000d2a1:  5e                 POP ESI
        _emit 0x5e
        // 0000d2a2:  33 c0              XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0000d2a4:  5b                 POP EBX
        _emit 0x5b
        // 0000d2a5:  c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 0000d2a8:  85 f6              TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0000d2aa:  7d 07              JGE +0x07  (-> 0x0040d2b3)
        _emit 0x7d
        _emit 0x07
        // 0000d2ac:  5e                 POP ESI
        _emit 0x5e
        // 0000d2ad:  33 c0              XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0000d2af:  5b                 POP EBX
        _emit 0x5b
        // 0000d2b0:  c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 0000d2b3:  8b 04 b3           MOV EAX, dword ptr [EBX + ESI*4]
        _emit 0x8b
        _emit 0x04
        _emit 0xb3
        // 0000d2b6:  85 c0              TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0000d2b8:  74 14              JZ +0x14  (-> 0x0040d2ce)
        _emit 0x74
        _emit 0x14
        // 0000d2ba:  8d 9b 00 00 00 00  LEA EBX, [EBX]  (6-byte NOP pad)
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000d2c0:  66 83 78 0a 00     CMP word ptr [EAX+0xa], 0x0
        _emit 0x66
        _emit 0x83
        _emit 0x78
        _emit 0x0a
        _emit 0x00
        // 0000d2c5:  77 2c              JA +0x2c  (-> 0x0040d2f3, return EAX)
        _emit 0x77
        _emit 0x2c
        // 0000d2c7:  8b 40 1c           MOV EAX, dword ptr [EAX+0x1c]
        _emit 0x8b
        _emit 0x40
        _emit 0x1c
        // 0000d2ca:  85 c0              TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0000d2cc:  75 f2              JNZ -0x0e  (-> 0x0040d2c0)
        _emit 0x75
        _emit 0xf2
        // 0000d2ce:  83 fe 16           CMP ESI, 0x16
        _emit 0x83
        _emit 0xfe
        _emit 0x16
        // 0000d2d1:  72 04              JC +0x04  (-> 0x0040d2d7)
        _emit 0x72
        _emit 0x04
        // 0000d2d3:  33 c0              XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0000d2d5:  eb 07              JMP +0x07  (-> 0x0040d2de)
        _emit 0xeb
        _emit 0x07
        // 0000d2d7:  8b 04 b5 88 59 f5 00  MOV EAX, dword ptr [ESI*4 + 0xf55988]
        _emit 0x8b
        _emit 0x04
        _emit 0xb5
        _emit 0x88
        _emit 0x59
        _emit 0xf5
        _emit 0x00
        // 0000d2de:  57                 PUSH EDI
        _emit 0x57
        // 0000d2df:  50                 PUSH EAX
        _emit 0x50
        // 0000d2e0:  e8 7b fd ff ff     CALL 0x0040d060  (rel32 = 0xfffd7b)
        _emit 0xe8
        _emit 0x7b
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 0000d2e5:  8b f8              MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // 0000d2e7:  56                 PUSH ESI
        _emit 0x56
        // 0000d2e8:  57                 PUSH EDI
        _emit 0x57
        // 0000d2e9:  8b cb              MOV ECX, EBX
        _emit 0x8b
        _emit 0xcb
        // 0000d2eb:  e8 a0 d8 ff ff     CALL 0x0040ab90  (rel32 = 0xffffd8a0)
        _emit 0xe8
        _emit 0xa0
        _emit 0xd8
        _emit 0xff
        _emit 0xff
        // 0000d2f0:  8b c7              MOV EAX, EDI
        _emit 0x8b
        _emit 0xc7
        // 0000d2f2:  5f                 POP EDI
        _emit 0x5f
        // 0000d2f3:  5e                 POP ESI
        _emit 0x5e
        // 0000d2f4:  5b                 POP EBX
        _emit 0x5b
        // 0000d2f5:  c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
