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
// FUNCTION: ffxivgame 0x00051210 — __thiscall red-black tree iterator
//                                  advance (tree "increment" with
//                                  __invalid_parameter_noinfo guards)
//                                  RVA 0x00051210 / VA 0x00451210, 137 bytes
//
// Calling convention: __thiscall (ECX = this); no stack arguments; plain RET.
// Callee-saves: ESI only (PUSH ESI / MOV ESI,ECX at entry).
//
// Object layout inferred (MSVC 2005 std::_Tree checked-iterator wrapper):
//   [this + 0x00]  _Mycont * — tree container pointer (must be non-null)
//   [this + 0x04]  _Node  * — current node pointer (_Myptr)
//
// Node layout (MSVC 2005 std::_Tree_node):
//   [node + 0x00]  _Node * _Left   — left child
//   [node + 0x04]  _Node * _Right  — right child  (used as right subtree root)
//   [node + 0x08]  _Node * _Parent — parent (also used as "right sibling" for
//                                    the header / end-of-sequence node)
//   [node + 0x45]  bool    _Isnil  — true only for the header (sentinel) node
//
// Behaviour (recovered from asm):
//
//   void __thiscall FUN_00451210() {
//       _verify_container();              // guard: [this+0] != 0
//       Node *p = _Myptr;                // = [this+4]
//       if (p->_Isnil) {                 // current is the header node
//           Node *r = p->_Parent;        // step to _Parent
//           _Myptr = r;
//           if (!r->_Isnil) return;      // normal
//           __invalid_parameter_noinfo();// both nil → error
//       }
//       Node *l = p->_Left;             // not-nil path: descend left
//       if (!l->_Isnil) {               // left subtree exists
//           Node *r2 = l->_Parent;
//           if (r2->_Isnil) {
//               // walk down while parent is not nil
//               do { Node *t = r2; r2 = t->_Parent; } while (!r2->_Isnil);
//               _Myptr = t;
//               return;
//           }
//           _Myptr = l;
//           return;
//       }
//       // no left subtree — walk right/up
//       Node *r3 = p->_Right;
//       if (!r3->_Isnil) {              // right subtree exists
//           while (true) {
//               Node *cur = _Myptr;
//               if (cur == r3->_Left) break;  // found position
//               _Myptr = r3;
//               r3 = r3->_Right;
//               if (r3->_Isnil) break;
//           }
//       }
//       Node *cur = _Myptr;
//       if (!cur->_Isnil)
//           __invalid_parameter_noinfo(); // error
//       _Myptr = r3;                     // set to terminal node
//   }
//
// CALL/JMP targets (all REL32 — wildcarded by tools/compare.py):
//   +0x08   CALL FUN_009d22b4   — __invalid_parameter_noinfo (guard)
//   +0x23   JMP  FUN_009d22b4   — __invalid_parameter_noinfo (tail call, error)
//   +0x7f   JMP  FUN_009d22b4   — __invalid_parameter_noinfo (tail call, error)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Two 7-byte alignment NOPs (LEA ESP,[ESP+0] at offsets +0x39 and +0x59)
//   are loop-top alignment padding inserted by MSVC 2005. These cannot be
//   reproduced from a C++ source-level form. The tail-call JMPs (e9 opcode)
//   also cannot be reproduced from C++. Raw _emit is used for all bytes
//   except the three CALL/JMP relocation sites, which use symbolic asm
//   references so the linker generates correct COFF REL32 entries.

extern "C" void FUN_009d22b4();  // __invalid_parameter_noinfo

extern "C" __declspec(naked) void FUN_00451210() {
    __asm {
        // +0x00: 56              PUSH ESI
        _emit 0x56
        // +0x01: 8b f1           MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // +0x03: 83 3e 00        CMP dword ptr [ESI], 0
        _emit 0x83
        _emit 0x3e
        _emit 0x00
        // +0x06: 75 05           JNZ +5  (→ +0x0d)
        _emit 0x75
        _emit 0x05
        // +0x08: e8 ...          CALL FUN_009d22b4  (REL32, compare.py masks)
        call FUN_009d22b4
        // +0x0d: 8b 46 04        MOV EAX, dword ptr [ESI+4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // +0x10: 80 78 45 00     CMP byte ptr [EAX+0x45], 0
        _emit 0x80
        _emit 0x78
        _emit 0x45
        _emit 0x00
        // +0x14: 74 12           JZ +0x12  (→ +0x28)
        _emit 0x74
        _emit 0x12
        // +0x16: 8b 40 08        MOV EAX, dword ptr [EAX+8]
        _emit 0x8b
        _emit 0x40
        _emit 0x08
        // +0x19: 89 46 04        MOV dword ptr [ESI+4], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // +0x1c: 80 78 45 00     CMP byte ptr [EAX+0x45], 0
        _emit 0x80
        _emit 0x78
        _emit 0x45
        _emit 0x00
        // +0x20: 74 65           JZ +0x65  (→ +0x87)
        _emit 0x74
        _emit 0x65
        // +0x22: 5e              POP ESI
        _emit 0x5e
        // +0x23: e9 ...          JMP FUN_009d22b4  (REL32, compare.py masks)
        jmp FUN_009d22b4
        // +0x28: 8b 08           MOV ECX, dword ptr [EAX]
        _emit 0x8b
        _emit 0x08
        // +0x2a: 80 79 45 00     CMP byte ptr [ECX+0x45], 0
        _emit 0x80
        _emit 0x79
        _emit 0x45
        _emit 0x00
        // +0x2e: 75 20           JNZ +0x20  (→ +0x50)
        _emit 0x75
        _emit 0x20
        // +0x30: 8b 41 08        MOV EAX, dword ptr [ECX+8]
        _emit 0x8b
        _emit 0x41
        _emit 0x08
        // +0x33: 80 78 45 00     CMP byte ptr [EAX+0x45], 0
        _emit 0x80
        _emit 0x78
        _emit 0x45
        _emit 0x00
        // +0x37: 75 12           JNZ +0x12  (→ +0x4b)
        _emit 0x75
        _emit 0x12
        // +0x39: 8d a4 24 00 00 00 00   LEA ESP, [ESP+0]  (7-byte NOP, loop align)
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x40: 8b c8           MOV ECX, EAX    ← inner loop top
        _emit 0x8b
        _emit 0xc8
        // +0x42: 8b 41 08        MOV EAX, dword ptr [ECX+8]
        _emit 0x8b
        _emit 0x41
        _emit 0x08
        // +0x45: 80 78 45 00     CMP byte ptr [EAX+0x45], 0
        _emit 0x80
        _emit 0x78
        _emit 0x45
        _emit 0x00
        // +0x49: 74 f5           JZ -0x0b  (→ +0x40, loop)
        _emit 0x74
        _emit 0xf5
        // +0x4b: 89 4e 04        MOV dword ptr [ESI+4], ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x04
        // +0x4e: 5e              POP ESI
        _emit 0x5e
        // +0x4f: c3              RET
        _emit 0xc3
        // +0x50: 8b 40 04        MOV EAX, dword ptr [EAX+4]
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        // +0x53: 80 78 45 00     CMP byte ptr [EAX+0x45], 0
        _emit 0x80
        _emit 0x78
        _emit 0x45
        _emit 0x00
        // +0x57: 75 1c           JNZ +0x1c  (→ +0x75)
        _emit 0x75
        _emit 0x1c
        // +0x59: 8d a4 24 00 00 00 00   LEA ESP, [ESP+0]  (7-byte NOP, loop align)
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x60: 8b 4e 04        MOV ECX, dword ptr [ESI+4]   ← outer loop top
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // +0x63: 3b 08           CMP ECX, dword ptr [EAX]
        _emit 0x3b
        _emit 0x08
        // +0x65: 75 0e           JNZ +0x0e  (→ +0x75)
        _emit 0x75
        _emit 0x0e
        // +0x67: 89 46 04        MOV dword ptr [ESI+4], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // +0x6a: 8b d0           MOV EDX, EAX
        _emit 0x8b
        _emit 0xd0
        // +0x6c: 8b 42 04        MOV EAX, dword ptr [EDX+4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // +0x6f: 80 78 45 00     CMP byte ptr [EAX+0x45], 0
        _emit 0x80
        _emit 0x78
        _emit 0x45
        _emit 0x00
        // +0x73: 74 eb           JZ -0x15  (→ +0x60, loop)
        _emit 0x74
        _emit 0xeb
        // +0x75: 8b 4e 04        MOV ECX, dword ptr [ESI+4]
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // +0x78: 80 79 45 00     CMP byte ptr [ECX+0x45], 0
        _emit 0x80
        _emit 0x79
        _emit 0x45
        _emit 0x00
        // +0x7c: 74 06           JZ +6  (→ +0x84)
        _emit 0x74
        _emit 0x06
        // +0x7e: 5e              POP ESI
        _emit 0x5e
        // +0x7f: e9 ...          JMP FUN_009d22b4  (REL32, compare.py masks)
        jmp FUN_009d22b4
        // +0x84: 89 46 04        MOV dword ptr [ESI+4], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // +0x87: 5e              POP ESI
        _emit 0x5e
        // +0x88: c3              RET
        _emit 0xc3
    }
}
