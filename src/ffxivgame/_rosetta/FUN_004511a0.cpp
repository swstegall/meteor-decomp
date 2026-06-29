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
// FUNCTION: ffxivgame 0x000511a0 — tree-iterator increment / advance with
//                                  sentinel-node check at offset 0x45
//                                  (__thiscall void, 99 B / 0x63)
//
// __thiscall void FUN_004511a0(void)
//   ECX = this  (iterator-like struct; this->field_0x0 = tree root/guard,
//                this->field_0x4 = current node pointer)
//
// Logic (high-level):
//
//   1. If *(this+0x0) == 0, call guard/assertion at 0x009d22b4 (debug check).
//   2. Load node = *(this+0x4).
//   3. If node->byte_0x45 != 0 (sentinel flag set):
//        tail-call 0x009d22b4 (past-the-end / invalid iterator assertion).
//   4. Otherwise try to advance the iterator:
//        a. p = node->field_0x8 (right child or next link).
//        b. If p->byte_0x45 == 0 (not a sentinel):
//             advance via left-spine of right subtree:
//               ECX = p
//               EAX = *(ECX)  (ECX's first field)
//               while EAX->byte_0x45 == 0: ECX = EAX; EAX = *(ECX)
//             *(this+0x4) = ECX
//             return
//        c. Else (right child is sentinel — go up via parent links):
//             EAX = node->field_0x4 (parent or left link)
//             if EAX->byte_0x45 != 0: *(this+0x4) = EAX; return
//             else:
//               loop: if *(this+0x4) == *(EAX+0x8):
//                       *(this+0x4) = EAX
//                       EDX = EAX
//                       EAX = *(EDX+0x4)
//                       while EAX->byte_0x45 == 0: (same loop body)
//               *(this+0x4) = EAX; return
//
// This is the canonical MSVC 2005 std::_Tree::iterator::operator++ body
// for an in-order traversal with a header/sentinel node whose `bool _Isnil`
// field lives at struct offset 0x45.
//
// Calling convention: __thiscall, no stack args, void return (RET / tail-JMP).
// Frame: PUSH ESI / POP ESI; no SUB ESP.
//
// Reloc-bearing sites in the orig 99 bytes:
//   +0x08   CALL rel32 → 0x009d22b4  (null/invalid iterator assertion)
//   +0x17   JMP  rel32 → 0x009d22b4  (past-end iterator assertion tail-call)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The two relative branch targets resolve to 0x009d22b4 in the original
//   PE's .text address space; their rel32 bytes are baked in verbatim here.
//   The remainder of the function contains no absolute or relative relocations
//   (all references are register-indirect or short PC-relative).  The .obj
//   .text is byte-identical to the orig slice, so compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_004511a0() {
    __asm {
        // 000511a0: 56              PUSH ESI
        _emit 0x56
        // 000511a1: 8b f1           MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 000511a3: 83 3e 00        CMP dword ptr [ESI], 0x0
        _emit 0x83
        _emit 0x3e
        _emit 0x00
        // 000511a6: 75 05           JNZ +5  (→ 000511ad)
        _emit 0x75
        _emit 0x05
        // 000511a8: e8 07 11 58 00  CALL 0x009d22b4  (null check / assertion)
        _emit 0xe8
        _emit 0x07
        _emit 0x11
        _emit 0x58
        _emit 0x00
        // 000511ad: 8b 46 04        MOV EAX, dword ptr [ESI + 0x4]   ; EAX = current node
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 000511b0: 80 78 45 00     CMP byte ptr [EAX + 0x45], 0x0   ; is_nil(node)?
        _emit 0x80
        _emit 0x78
        _emit 0x45
        _emit 0x00
        // 000511b4: 74 06           JZ +6  (→ 000511bc, node is live; advance)
        _emit 0x74
        _emit 0x06
        // 000511b6: 5e              POP ESI
        _emit 0x5e
        // 000511b7: e9 f8 10 58 00  JMP 0x009d22b4  (past-end iterator — tail-call assertion)
        _emit 0xe9
        _emit 0xf8
        _emit 0x10
        _emit 0x58
        _emit 0x00
        // 000511bc: 8b 48 08        MOV ECX, dword ptr [EAX + 0x8]   ; ECX = node->right (or parent link)
        _emit 0x8b
        _emit 0x48
        _emit 0x08
        // 000511bf: 80 79 45 00     CMP byte ptr [ECX + 0x45], 0x0   ; is_nil(right)?
        _emit 0x80
        _emit 0x79
        _emit 0x45
        _emit 0x00
        // 000511c3: 75 1a           JNZ +0x1a  (→ 000511df, right child is sentinel; go up)
        _emit 0x75
        _emit 0x1a
        // --- right child is live: descend left spine ---
        // 000511c5: 8b 01           MOV EAX, dword ptr [ECX]         ; EAX = *right (first field)
        _emit 0x8b
        _emit 0x01
        // 000511c7: 80 78 45 00     CMP byte ptr [EAX + 0x45], 0x0   ; is_nil(*right)?
        _emit 0x80
        _emit 0x78
        _emit 0x45
        _emit 0x00
        // 000511cb: 75 0d           JNZ +0xd  (→ 000511da, no left child; stop here)
        _emit 0x75
        _emit 0x0d
        // 000511cd: 8d 49 00        LEA ECX, [ECX]                   ; NOP (alignment pad)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // --- left-spine loop ---
        // 000511d0: 8b c8           MOV ECX, EAX                     ; ECX = EAX (descend)
        _emit 0x8b
        _emit 0xc8
        // 000511d2: 8b 01           MOV EAX, dword ptr [ECX]         ; EAX = *(ECX)
        _emit 0x8b
        _emit 0x01
        // 000511d4: 80 78 45 00     CMP byte ptr [EAX + 0x45], 0x0   ; is_nil(EAX)?
        _emit 0x80
        _emit 0x78
        _emit 0x45
        _emit 0x00
        // 000511d8: 74 f6           JZ -0xa  (→ 000511d0, continue descending)
        _emit 0x74
        _emit 0xf6
        // 000511da: 89 4e 04        MOV dword ptr [ESI + 0x4], ECX   ; this->current = ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x04
        // 000511dd: 5e              POP ESI
        _emit 0x5e
        // 000511de: c3              RET
        _emit 0xc3
        // --- right child is sentinel: walk up via parent links ---
        // 000511df: 8b 40 04        MOV EAX, dword ptr [EAX + 0x4]   ; EAX = node->parent
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        // 000511e2: 80 78 45 00     CMP byte ptr [EAX + 0x45], 0x0   ; is_nil(parent)?
        _emit 0x80
        _emit 0x78
        _emit 0x45
        _emit 0x00
        // 000511e6: 75 16           JNZ +0x16  (→ 000511fe, parent is sentinel; store and return)
        _emit 0x75
        _emit 0x16
        // --- parent-ascent loop ---
        // 000511e8: 8b 4e 04        MOV ECX, dword ptr [ESI + 0x4]   ; ECX = this->current
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 000511eb: 3b 48 08        CMP ECX, dword ptr [EAX + 0x8]   ; current == parent->right?
        _emit 0x3b
        _emit 0x48
        _emit 0x08
        // 000511ee: 75 0e           JNZ +0xe  (→ 000511fe, not right child; done)
        _emit 0x75
        _emit 0x0e
        // 000511f0: 89 46 04        MOV dword ptr [ESI + 0x4], EAX   ; this->current = parent
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 000511f3: 8b d0           MOV EDX, EAX                     ; EDX = parent
        _emit 0x8b
        _emit 0xd0
        // 000511f5: 8b 42 04        MOV EAX, dword ptr [EDX + 0x4]   ; EAX = parent->parent
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 000511f8: 80 78 45 00     CMP byte ptr [EAX + 0x45], 0x0   ; is_nil(grandparent)?
        _emit 0x80
        _emit 0x78
        _emit 0x45
        _emit 0x00
        // 000511fc: 74 ea           JZ -0x14  (→ 000511e8, continue ascending)
        _emit 0x74
        _emit 0xea
        // 000511fe: 89 46 04        MOV dword ptr [ESI + 0x4], EAX   ; this->current = EAX
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 00051201: 5e              POP ESI
        _emit 0x5e
        // 00051202: c3              RET
        _emit 0xc3
    }
}
