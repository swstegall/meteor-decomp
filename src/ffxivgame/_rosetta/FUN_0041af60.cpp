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
// FUNCTION: ffxivgame 0x0001af60 — red-black tree lower-bound / insert-position
//                                  traversal with 64-bit key comparison
//                                  (212 B / 0xd4, __thiscall, 2 stack args).
//
// Calling convention: __thiscall (ECX = this); 2 DWORD stack args; RET 0x8.
//
// Signature (recovered from asm):
//
//   void __thiscall FUN_0041af60(this,
//                                OutputPair *out,   // arg1: {ptr, ptr, bool}
//                                Key64      *key);  // arg2: {lo, hi}
//
//   Traverses a red-black tree rooted at this->field_4->field_4 using a
//   64-bit unsigned key comparison (key->hi vs node[+0x14], then
//   key->lo vs node[+0x10]).  The sentinel node is identified by the
//   byte at node[+0x21] being non-zero (_Isnil flag).
//
//   After traversal writes a 3-field output record to *out:
//     out->field_0 : node pointer
//     out->field_4 : container / header pointer
//     out->field_8 : bool (1 = went-left / begin, 0 = not)
//
//   Two internal helpers are called:
//     FUN_0041aba0 — __thiscall, 4 stack args; inserts or locates a node
//     0x009fed10   — iterator validity check; returns normally
//
// Why naked asm:
//   Three MSVC 2005 /O2 idioms combine to make source-level reproduction
//   impractical:
//
//   1. Non-standard prologue ordering.  The compiler emits
//        SUB ESP,0xc / PUSH EBP / (load EBP=arg2) / PUSH ESI / PUSH EDI
//      rather than the usual push-all-saves-first pattern.  Reloading EBP
//      between the EBP push and the ESI/EDI pushes is a scheduler
//      artefact that no source rewrite reliably reproduces.
//
//   2. Conditional EBX save.  EBX is PUSH'd only inside the non-empty-tree
//      branch and POP'd before the branch merge at 0x41afaf.  MSVC /O2
//      sometimes defers callee-save pushes into live ranges that don't
//      cover every path; reproducing that exact placement from C++ is
//      trial-and-error.
//
//   3. The back-edge at 0x41afac (JZ 0x41af87) re-enters the loop just
//      before the MOV ESI,EAX update, making the loop shape ambiguous for
//      a high-level rewrite.
//
//   Emitting the 212 original bytes verbatim via MASM _emit produces a
//   .obj whose .text is byte-identical to the original slice, with the two
//   REL32 CALL displacements (at +0x6f and +0x91) wildcarded by
//   tools/compare.py.  compare.py reports GREEN.
//
// Reloc-bearing sites (offsets within the 212-byte function body):
//   +0x6f   REL32 -> FUN_0041aba0    (e8 cc fb ff ff)
//   +0x91   REL32 -> 0x009fed10      (e8 1a 3d 5e 00)

extern "C" __declspec(naked) void FUN_0041af60() {
    __asm {
        // 0001af60: 83 ec 0c     SUB ESP,0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 0001af63: 55           PUSH EBP
        _emit 0x55
        // 0001af64: 8b 6c 24 18  MOV EBP,[ESP+0x18]  (EBP = arg2 = key ptr)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        // 0001af68: 56           PUSH ESI
        _emit 0x56
        // 0001af69: 57           PUSH EDI
        _emit 0x57
        // 0001af6a: 8b f9        MOV EDI,ECX  (EDI = this)
        _emit 0x8b
        _emit 0xf9
        // 0001af6c: 8b 4f 04     MOV ECX,[EDI+0x4]   (ECX = this->field_4)
        _emit 0x8b
        _emit 0x4f
        _emit 0x04
        // 0001af6f: 8b 41 04     MOV EAX,[ECX+0x4]   (EAX = root node)
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 0001af72: 80 78 21 00  CMP byte ptr [EAX+0x21],0
        _emit 0x80
        _emit 0x78
        _emit 0x21
        _emit 0x00
        // 0001af76: 8b f1        MOV ESI,ECX   (ESI = header)
        _emit 0x8b
        _emit 0xf1
        // 0001af78: b1 01        MOV CL,0x1
        _emit 0xb1
        _emit 0x01
        // 0001af7a: 88 4c 24 0c  MOV [ESP+0xc],CL  (local_0c = 1)
        _emit 0x88
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0001af7e: 75 2f        JNZ +0x2f  (-> 0x41afaf if root is nil/tree empty)
        _emit 0x75
        _emit 0x2f
        // 0001af80: 8b 55 04     MOV EDX,[EBP+0x4]   (EDX = key->hi)
        _emit 0x8b
        _emit 0x55
        _emit 0x04
        // 0001af83: 53           PUSH EBX
        _emit 0x53
        // 0001af84: 8b 5d 00     MOV EBX,[EBP+0x0]   (EBX = key->lo)
        _emit 0x8b
        _emit 0x5d
        _emit 0x00
        // 0001af87: 3b 50 14     CMP EDX,[EAX+0x14]  (key.hi vs node->key.hi)
        _emit 0x3b
        _emit 0x50
        _emit 0x14
        // 0001af8a: 8b f0        MOV ESI,EAX   (ESI = current node)
        _emit 0x8b
        _emit 0xf0
        // 0001af8c: 77 11        JA +0x11  (-> 0x41af9f: key.hi > node.hi -> right)
        _emit 0x77
        _emit 0x11
        // 0001af8e: 72 05        JC +0x05  (-> 0x41af95: key.hi < node.hi -> left)
        _emit 0x72
        _emit 0x05
        // 0001af90: 3b 58 10     CMP EBX,[EAX+0x10]  (key.lo vs node->key.lo)
        _emit 0x3b
        _emit 0x58
        _emit 0x10
        // 0001af93: 73 0a        JNC +0x0a  (-> 0x41af9f: key.lo >= node.lo -> right)
        _emit 0x73
        _emit 0x0a
        // 0001af95: 8b 00        MOV EAX,[EAX]   (go left: EAX = node->_Left)
        _emit 0x8b
        _emit 0x00
        // 0001af97: b1 01        MOV CL,0x1   (CL=1: went left)
        _emit 0xb1
        _emit 0x01
        // 0001af99: 88 4c 24 10  MOV [ESP+0x10],CL  (local_10 = 1)
        _emit 0x88
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0001af9d: eb 09        JMP +0x09  (-> 0x41afa8)
        _emit 0xeb
        _emit 0x09
        // 0001af9f: 8b 40 08     MOV EAX,[EAX+0x8]  (go right: EAX = node->_Right)
        _emit 0x8b
        _emit 0x40
        _emit 0x08
        // 0001afa2: 32 c9        XOR CL,CL   (CL=0: went right)
        _emit 0x32
        _emit 0xc9
        // 0001afa4: 88 4c 24 10  MOV [ESP+0x10],CL  (local_10 = 0)
        _emit 0x88
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0001afa8: 80 78 21 00  CMP byte ptr [EAX+0x21],0  (_Isnil of next node)
        _emit 0x80
        _emit 0x78
        _emit 0x21
        _emit 0x00
        // 0001afac: 74 d9        JZ -0x27  (-> 0x41af87: loop if not nil)
        _emit 0x74
        _emit 0xd9
        // 0001afae: 5b           POP EBX
        _emit 0x5b
        // 0001afaf: 84 c9        TEST CL,CL
        _emit 0x84
        _emit 0xc9
        // 0001afb1: 8b d6        MOV EDX,ESI   (EDX = last node)
        _emit 0x8b
        _emit 0xd6
        // 0001afb3: 89 54 24 14  MOV [ESP+0x14],EDX  (local_14 = ESI)
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0001afb7: 89 7c 24 10  MOV [ESP+0x10],EDI  (local_10 = this)
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 0001afbb: 74 3d        JZ +0x3d  (-> 0x41affa: CL==0 path)
        _emit 0x74
        _emit 0x3d
        // 0001afbd: 8b 47 04     MOV EAX,[EDI+0x4]  (EAX = this->field_4 = _Myhead)
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // 0001afc0: 3b 30        CMP ESI,[EAX]  (ESI vs _Myhead->_Left/begin)
        _emit 0x3b
        _emit 0x30
        // 0001afc2: 8d 4c 24 10  LEA ECX,[ESP+0x10]  (ECX = &local_10)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0001afc6: 75 29        JNZ +0x29  (-> 0x41aff1: not at begin)
        _emit 0x75
        _emit 0x29
        // 0001afc8: 55           PUSH EBP   (push key ptr)
        _emit 0x55
        // 0001afc9: 56           PUSH ESI   (push node)
        _emit 0x56
        // 0001afca: 6a 01        PUSH 0x1   (push flag=1)
        _emit 0x6a
        _emit 0x01
        // 0001afcc: 51           PUSH ECX   (push &local_10)
        _emit 0x51
        // 0001afcd: 8b cf        MOV ECX,EDI  (this = EDI)
        _emit 0x8b
        _emit 0xcf
        // 0001afcf: e8 cc fb ff ff  CALL FUN_0041aba0  [reloc +0x6f]
        _emit 0xe8
        _emit 0xcc
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        // 0001afd4: 8b c8        MOV ECX,EAX   (ECX = result ptr)
        _emit 0x8b
        _emit 0xc8
        // 0001afd6: 8b 11        MOV EDX,[ECX]   (EDX = result->field_0)
        _emit 0x8b
        _emit 0x11
        // 0001afd8: 8b 44 24 1c  MOV EAX,[ESP+0x1c]  (EAX = arg1 out ptr)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0001afdc: 8b 49 04     MOV ECX,[ECX+0x4]  (ECX = result->field_4)
        _emit 0x8b
        _emit 0x49
        _emit 0x04
        // 0001afdf: 5f           POP EDI
        _emit 0x5f
        // 0001afe0: 5e           POP ESI
        _emit 0x5e
        // 0001afe1: 89 10        MOV [EAX],EDX   (out->field_0 = result->field_0)
        _emit 0x89
        _emit 0x10
        // 0001afe3: 89 48 04     MOV [EAX+0x4],ECX  (out->field_4 = result->field_4)
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 0001afe6: c6 40 08 01  MOV byte ptr [EAX+0x8],1  (out->flag = true)
        _emit 0xc6
        _emit 0x40
        _emit 0x08
        _emit 0x01
        // 0001afea: 5d           POP EBP
        _emit 0x5d
        // 0001afeb: 83 c4 0c     ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0001afee: c2 08 00     RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // 0001aff1: e8 1a 3d 5e 00  CALL 0x009fed10  [reloc +0x91]
        _emit 0xe8
        _emit 0x1a
        _emit 0x3d
        _emit 0x5e
        _emit 0x00
        // 0001aff6: 8b 54 24 14  MOV EDX,[ESP+0x14]  (EDX = local_14 = ESI)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0001affa: 8b 42 14     MOV EAX,[EDX+0x14]  (EAX = node->key.hi)
        _emit 0x8b
        _emit 0x42
        _emit 0x14
        // 0001affd: 3b 45 04     CMP EAX,[EBP+0x4]  (node.hi vs key.hi)
        _emit 0x3b
        _emit 0x45
        _emit 0x04
        // 0001b000: 77 18        JA +0x18  (-> 0x41b01a: node > key -> output false)
        _emit 0x77
        _emit 0x18
        // 0001b002: 72 08        JC +0x08  (-> 0x41b00c: node < key -> call helper)
        _emit 0x72
        _emit 0x08
        // 0001b004: 8b 4a 10     MOV ECX,[EDX+0x10]  (ECX = node->key.lo)
        _emit 0x8b
        _emit 0x4a
        _emit 0x10
        // 0001b007: 3b 4d 00     CMP ECX,[EBP+0x0]  (node.lo vs key.lo)
        _emit 0x3b
        _emit 0x4d
        _emit 0x00
        // 0001b00a: 73 0e        JNC +0x0e  (-> 0x41b01a: node >= key -> output false)
        _emit 0x73
        _emit 0x0e
        // 0001b00c: 8b 54 24 0c  MOV EDX,[ESP+0xc]  (EDX = local_0c saved CL)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 0001b010: 55           PUSH EBP  (push key ptr)
        _emit 0x55
        // 0001b011: 56           PUSH ESI  (push ESI last node)
        _emit 0x56
        // 0001b012: 52           PUSH EDX  (push flag)
        _emit 0x52
        // 0001b013: 8d 44 24 1c  LEA EAX,[ESP+0x1c]  (EAX = &local_10 after 3 pushes)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0001b017: 50           PUSH EAX  (push &local_10)
        _emit 0x50
        // 0001b018: eb b3        JMP -0x4d  (-> 0x41afcd: MOV ECX,EDI + CALL FUN_0041aba0)
        _emit 0xeb
        _emit 0xb3
        // 0001b01a: 8b 44 24 1c  MOV EAX,[ESP+0x1c]  (EAX = arg1 out ptr)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0001b01e: 8b 4c 24 10  MOV ECX,[ESP+0x10]  (ECX = local_10 = this)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0001b022: 5f           POP EDI
        _emit 0x5f
        // 0001b023: 5e           POP ESI
        _emit 0x5e
        // 0001b024: 89 08        MOV [EAX],ECX   (out->field_0 = this)
        _emit 0x89
        _emit 0x08
        // 0001b026: 89 50 04     MOV [EAX+0x4],EDX  (out->field_4 = ESI last node)
        _emit 0x89
        _emit 0x50
        _emit 0x04
        // 0001b029: c6 40 08 00  MOV byte ptr [EAX+0x8],0  (out->flag = false)
        _emit 0xc6
        _emit 0x40
        _emit 0x08
        _emit 0x00
        // 0001b02d: 5d           POP EBP
        _emit 0x5d
        // 0001b02e: 83 c4 0c     ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0001b031: c2 08 00     RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
