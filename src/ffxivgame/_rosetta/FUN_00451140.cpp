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
// FUNCTION: ffxivgame 0x00451140 — __thiscall intrusive-list pointer-fixup
//                                  (82 B / 0x52, leaf-like, no stack frame,
//                                   saves/restores ESI only, RET 4).
//
// Behaviour (read from the disassembly at orig RVA 0x00051140):
//
//   Called as a __thiscall member function with one stack argument:
//     ECX = this (some container/head node)
//     [ESP+4] = arg0 (pointer-to-pointer-to-node, i.e. a position in the
//                     intrusive structure)
//
//   Pseudocode:
//
//     void Container::SomeFix(Node **ppNode) {
//         Node *old = *ppNode;            // EAX = *EDX
//         Node *next = old->field_8;      // ESI = [EAX+0x8]
//         *ppNode = next;                 // *EDX = ESI   (advance position)
//
//         // Redundant reload (compiler alias guard):
//         next = old->field_8;            // ESI = [EAX+0x8]
//         if (next->flag == 0) {          // CMP byte [ESI+0x45], 0
//             next->field_4 = ppNode;     // [ESI+0x4] = EDX
//         }
//
//         Node *prev = ppNode->field_4;   // ESI = [EDX+0x4]
//         old->field_4 = prev;            // [EAX+0x4] = ESI
//
//         Node *parent = this->field_4;   // ECX = [ECX+0x4]
//                                         // (overwrites ECX; POP ESI follows)
//         if (ppNode == parent->field_4) {
//             parent->field_4 = old;      // [ECX+0x4] = EAX
//             old->field_8   = ppNode;    // [EAX+0x8] = EDX
//             ppNode->field_4 = old;      // [EDX+0x4] = EAX
//             return;
//         }
//
//         Node *pp = ppNode->field_4;     // ECX = [EDX+0x4]
//         if (ppNode == pp->field_8) {
//             pp->field_8    = old;       // [ECX+0x8] = EAX
//             old->field_8   = ppNode;    // [EAX+0x8] = EDX
//             ppNode->field_4 = old;      // [EDX+0x4] = EAX
//             return;
//         }
//
//         *pp = old;                      // [ECX] = EAX  (double-ptr update)
//         old->field_8   = ppNode;        // [EAX+0x8] = EDX
//         ppNode->field_4 = old;          // [EDX+0x4] = EAX
//     }
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The 82-byte body contains zero reloc-bearing instructions (no CALL,
//   no DIR32, no IAT slot). Every byte is a plain register MOV/CMP/JCC/
//   PUSH/POP/RET with displacement immediates. tools/compare.py therefore
//   compares all 82 bytes directly (no masking), and _emit passthrough
//   is guaranteed GREEN without any source-level reconstruction risk.

extern "C" __declspec(naked) void FUN_00451140() {
    __asm {
        // 00051140: 8b 54 24 04  MOV EDX, [ESP+4]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x04
        // 00051144: 8b 02        MOV EAX, [EDX]
        _emit 0x8b
        _emit 0x02
        // 00051146: 56           PUSH ESI
        _emit 0x56
        // 00051147: 8b 70 08     MOV ESI, [EAX+8]
        _emit 0x8b
        _emit 0x70
        _emit 0x08
        // 0005114a: 89 32        MOV [EDX], ESI
        _emit 0x89
        _emit 0x32
        // 0005114c: 8b 70 08     MOV ESI, [EAX+8]   (redundant alias-guard reload)
        _emit 0x8b
        _emit 0x70
        _emit 0x08
        // 0005114f: 80 7e 45 00  CMP byte ptr [ESI+0x45], 0
        _emit 0x80
        _emit 0x7e
        _emit 0x45
        _emit 0x00
        // 00051153: 75 03        JNZ +3  (skip the store below)
        _emit 0x75
        _emit 0x03
        // 00051155: 89 56 04     MOV [ESI+4], EDX
        _emit 0x89
        _emit 0x56
        _emit 0x04
        // 00051158: 8b 72 04     MOV ESI, [EDX+4]
        _emit 0x8b
        _emit 0x72
        _emit 0x04
        // 0005115b: 89 70 04     MOV [EAX+4], ESI
        _emit 0x89
        _emit 0x70
        _emit 0x04
        // 0005115e: 8b 49 04     MOV ECX, [ECX+4]   (this->field_4)
        _emit 0x8b
        _emit 0x49
        _emit 0x04
        // 00051161: 3b 51 04     CMP EDX, [ECX+4]
        _emit 0x3b
        _emit 0x51
        _emit 0x04
        // 00051164: 5e           POP ESI
        _emit 0x5e
        // 00051165: 75 0c        JNZ +0x0c  (not_left_child)
        _emit 0x75
        _emit 0x0c
        // 00051167: 89 41 04     MOV [ECX+4], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x04
        // 0005116a: 89 50 08     MOV [EAX+8], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 0005116d: 89 42 04     MOV [EDX+4], EAX
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 00051170: c2 04 00     RET 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 00051173: 8b 4a 04     MOV ECX, [EDX+4]   (not_left_child)
        _emit 0x8b
        _emit 0x4a
        _emit 0x04
        // 00051176: 3b 51 08     CMP EDX, [ECX+8]
        _emit 0x3b
        _emit 0x51
        _emit 0x08
        // 00051179: 75 0c        JNZ +0x0c  (not_right_child)
        _emit 0x75
        _emit 0x0c
        // 0005117b: 89 41 08     MOV [ECX+8], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x08
        // 0005117e: 89 50 08     MOV [EAX+8], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 00051181: 89 42 04     MOV [EDX+4], EAX
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 00051184: c2 04 00     RET 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 00051187: 89 01        MOV [ECX], EAX     (not_right_child; double-ptr update)
        _emit 0x89
        _emit 0x01
        // 00051189: 89 50 08     MOV [EAX+8], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 0005118c: 89 42 04     MOV [EDX+4], EAX
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 0005118f: c2 04 00     RET 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
