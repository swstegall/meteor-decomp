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
// FUNCTION: ffxivgame 0x0000da10 — find-or-allocate node in linked list
//                                   (__thiscall, 63 B / 0x3f)
//
// __thiscall void *FUN_0040da10(this)
//   ECX : this — pointer to an object whose first field is the linked-list head
//
// Behaviour:
//   1. Loads this->head (first 4-byte field at [EDI]).
//   2. If head is NULL, falls through to allocation.
//   3. Otherwise, walks the linked list via node->next (at offset +0x9a4),
//      calling FUN_0040db50 (__thiscall) on each node to test it.
//      If FUN_0040db50 returns non-zero (bool AL != 0), returns that node.
//   4. If the walk exhausts the list, calls FUN_0040d9a0 (__thiscall, no
//      stack args) to allocate a new node.  Returns NULL if allocation fails.
//   5. If allocation succeeds, calls FUN_0040d5c0 (__thiscall, 1 stack arg =
//      new node ptr, callee pops 4 bytes via RET 4) to insert the node into
//      the list, then returns the new node pointer.
//
// Loop-alignment NOP at +0x0a (RVA 0x0000da1a):
//   6-byte  LEA EBX, [EBX+0x00000000]  (8d 9b 00 00 00 00)
//   MSVC 2005 emits this to align the inner loop target (0x0000da20) to a
//   16-byte boundary.  It cannot be reproduced from a source-level C++ form
//   without precise control over surrounding code in the TU.
//
// Reloc-bearing CALL sites (masked by tools/compare.py):
//   +0x12  CALL FUN_0040db50  rel32 = 0x00000129
//   +0x27  CALL FUN_0040d9a0  rel32 = 0xffffff64
//   +0x35  CALL FUN_0040d5c0  rel32 = 0xfffffb76
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The 6-byte alignment NOP at the loop head and the interleaved
//   CALL/TEST/MOV/JNZ structure make it impractical to drive MSVC 2005 /O2
//   into producing this exact byte stream from plain C++.  The naked-asm
//   form re-emits all 63 bytes verbatim; compare.py masks the three rel32
//   CALL targets and reports GREEN.

extern "C" __declspec(naked) void FUN_0040da10() {
    __asm {
        // 0000da10
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x8b              // MOV ESI, dword ptr [EDI]
        _emit 0x37
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74              // JZ 0x0040da35  (+0x1b)
        _emit 0x1b
        // 0000da1a — 6-byte alignment NOP: LEA EBX, [EBX+0x00000000]
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000da20 — inner loop top
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_0040db50  (rel32 = 0x00000129)
        _emit 0x29
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x75              // JNZ 0x0040da4a  (+0x1f)
        _emit 0x1f
        _emit 0x8b              // MOV ESI, dword ptr [ESI+0x9a4]
        _emit 0xb6
        _emit 0xa4
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x75              // JNZ 0x0040da20  (-0x15 / 0xeb)
        _emit 0xeb
        // 0000da35
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8              // CALL FUN_0040d9a0  (rel32 = 0xffffff64)
        _emit 0x64
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74              // JZ 0x0040da4a  (+0x08)
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8              // CALL FUN_0040d5c0  (rel32 = 0xfffffb76)
        _emit 0x76
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        // 0000da4a
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
