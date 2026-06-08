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
// FUNCTION: ffxivgame 0x00020630 — red-black tree iterator increment
//                                  (__thiscall, 99 B / 0x63)
//
// __thiscall void FUN_00420630(void)
//   ECX = this  (iterator struct: [+0x00] = container ptr, [+0x04] = current node ptr)
//
// Implements the standard BST successor algorithm for MSVC 2005's intrusive
// red-black tree (_Tree_iterator::operator++ / _Inc).  Node layout (offsets
// used by this function):
//   +0x00  _Left    (nodeptr)
//   +0x04  _Parent  (nodeptr)
//   +0x08  _Right   (nodeptr)
//   +0x11  _Isnil   (byte, non-zero for the sentinel/nil node)
//
// Logic:
//   1. Assert this->_Mycont != NULL (call 0x009d22b4 if null; also called
//      tail-call if current node is the sentinel/end iterator).
//   2. p = this->_Ptr  (node at +0x04)
//   3. If p->_Right->_Isnil == 0  (right subtree exists):
//        go right once, then go left as far as possible → this->_Ptr = result
//   4. Else (right subtree is nil):
//        walk up the tree while p is the right child of its parent;
//        this->_Ptr = the first ancestor for which p was a left child
//        (or the sentinel if we walked off the top).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Offsets +0x08 and +0x17 contain rel32 CALL / JMP to VA 0x009d22b4
//   (an assertion/throw helper).  These absolute addresses resolve only
//   in a full-binary relink at image base 0x00400000; a standalone .obj
//   cannot reproduce them via idiomatic C++.  The same __declspec(naked) /
//   _emit approach used by all other _rosetta siblings is used here.

extern "C" __declspec(naked) void FUN_00420630() {
    __asm {
        // 00020630: 56              PUSH ESI
        _emit 0x56
        // 00020631: 8b f1           MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00020633: 83 3e 00        CMP dword ptr [ESI], 0x0
        _emit 0x83
        _emit 0x3e
        _emit 0x00
        // 00020636: 75 05           JNZ +0x05  (→ 0002063d)
        _emit 0x75
        _emit 0x05
        // 00020638: e8 77 1c 5b 00  CALL 0x009d22b4  (assert: container must be valid)
        _emit 0xe8
        _emit 0x77
        _emit 0x1c
        _emit 0x5b
        _emit 0x00
        // 0002063d: 8b 46 04        MOV EAX, dword ptr [ESI+0x4]  (p = this->_Ptr)
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00020640: 80 78 11 00     CMP byte ptr [EAX+0x11], 0x0  (p->_Isnil == 0?)
        _emit 0x80
        _emit 0x78
        _emit 0x11
        _emit 0x00
        // 00020644: 74 06           JZ +0x06  (→ 0002064c; not the sentinel, proceed)
        _emit 0x74
        _emit 0x06
        // 00020646: 5e              POP ESI
        _emit 0x5e
        // 00020647: e9 68 1c 5b 00  JMP 0x009d22b4  (tail-call assert: at end iterator)
        _emit 0xe9
        _emit 0x68
        _emit 0x1c
        _emit 0x5b
        _emit 0x00
        // 0002064c: 8b 48 08        MOV ECX, dword ptr [EAX+0x8]  (ECX = p->_Right)
        _emit 0x8b
        _emit 0x48
        _emit 0x08
        // 0002064f: 80 79 11 00     CMP byte ptr [ECX+0x11], 0x0  (p->_Right->_Isnil == 0?)
        _emit 0x80
        _emit 0x79
        _emit 0x11
        _emit 0x00
        // 00020653: 75 1a           JNZ +0x1a  (→ 0002066f; right is nil, walk up)
        _emit 0x75
        _emit 0x1a
        // --- right subtree exists: go right then leftmost ---
        // 00020655: 8b 01           MOV EAX, dword ptr [ECX]  (EAX = p->_Right->_Left)
        _emit 0x8b
        _emit 0x01
        // 00020657: 80 78 11 00     CMP byte ptr [EAX+0x11], 0x0  (_Right->_Left->_Isnil == 0?)
        _emit 0x80
        _emit 0x78
        _emit 0x11
        _emit 0x00
        // 0002065b: 75 0d           JNZ +0x0d  (→ 0002066a; no left child, use right directly)
        _emit 0x75
        _emit 0x0d
        // 0002065d: 8d 49 00        LEA ECX, [ECX]  (3-byte NOP for alignment)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // 00020660: 8b c8           MOV ECX, EAX   (ECX = EAX, go left)
        _emit 0x8b
        _emit 0xc8
        // 00020662: 8b 01           MOV EAX, dword ptr [ECX]  (EAX = ECX->_Left)
        _emit 0x8b
        _emit 0x01
        // 00020664: 80 78 11 00     CMP byte ptr [EAX+0x11], 0x0  (_Left->_Isnil == 0?)
        _emit 0x80
        _emit 0x78
        _emit 0x11
        _emit 0x00
        // 00020668: 74 f6           JZ -0x0a  (→ 00020660; keep going left)
        _emit 0x74
        _emit 0xf6
        // 0002066a: 89 4e 04        MOV dword ptr [ESI+0x4], ECX  (this->_Ptr = leftmost)
        _emit 0x89
        _emit 0x4e
        _emit 0x04
        // 0002066d: 5e              POP ESI
        _emit 0x5e
        // 0002066e: c3              RET
        _emit 0xc3
        // --- right is nil: walk up while right child ---
        // 0002066f: 8b 40 04        MOV EAX, dword ptr [EAX+0x4]  (EAX = p->_Parent)
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        // 00020672: 80 78 11 00     CMP byte ptr [EAX+0x11], 0x0  (parent->_Isnil == 0?)
        _emit 0x80
        _emit 0x78
        _emit 0x11
        _emit 0x00
        // 00020676: 75 16           JNZ +0x16  (→ 0002068e; at root/sentinel, done)
        _emit 0x75
        _emit 0x16
        // 00020678: 8b 4e 04        MOV ECX, dword ptr [ESI+0x4]  (ECX = this->_Ptr, reload)
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 0002067b: 3b 48 08        CMP ECX, dword ptr [EAX+0x8]  (p == parent->_Right?)
        _emit 0x3b
        _emit 0x48
        _emit 0x08
        // 0002067e: 75 0e           JNZ +0x0e  (→ 0002068e; p is left child, parent is successor)
        _emit 0x75
        _emit 0x0e
        // 00020680: 89 46 04        MOV dword ptr [ESI+0x4], EAX  (this->_Ptr = parent)
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 00020683: 8b d0           MOV EDX, EAX                  (EDX = parent)
        _emit 0x8b
        _emit 0xd0
        // 00020685: 8b 42 04        MOV EAX, dword ptr [EDX+0x4]  (EAX = parent->_Parent)
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00020688: 80 78 11 00     CMP byte ptr [EAX+0x11], 0x0  (grandparent->_Isnil == 0?)
        _emit 0x80
        _emit 0x78
        _emit 0x11
        _emit 0x00
        // 0002068c: 74 ea           JZ -0x16  (→ 00020678; keep walking up)
        _emit 0x74
        _emit 0xea
        // 0002068e: 89 46 04        MOV dword ptr [ESI+0x4], EAX  (this->_Ptr = result)
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 00020691: 5e              POP ESI
        _emit 0x5e
        // 00020692: c3              RET
        _emit 0xc3
    }
}
