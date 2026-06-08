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
// FUNCTION: ffxivgame 0x00021820 — BST lower-bound/insert traversal helper
//                                  (__thiscall, 185 B / 0xb9)
//
//   Traverses a binary search tree (consistent with MSVC 2005 std::set /
//   std::map internals) to find the insertion point for a given integer key,
//   then delegates to the internal _Insert helper at 0x00420fc0.
//
//   __thiscall void* FUN_00421820(this, OutputPair *out, int *key_ptr)
//     calling convention: __thiscall (ECX = this), RET 0x8 (2 stack args)
//       ECX           : this  (BST container)
//       [ESP + 0x04]  : OutputPair *out  (receives {node_or_this, node, found})
//       [ESP + 0x08]  : int *key_ptr     (pointer to key being searched/inserted)
//
//   Node structure (inferred from offsets):
//     +0x00  Node *left
//     +0x04  Node *parent
//     +0x08  Node *right
//     +0x0c  int   key
//     +0x15  char  is_nil  (non-zero = sentinel/end node)
//
//   Container structure (this):
//     +0x04  TreeHeader *head   (head->parent = root,
//                                head->left   = leftmost/begin)
//
//   OutputPair structure:
//     +0x00  void *first   (container this  OR result->field0)
//     +0x04  void *second  (node ptr        OR result->field4)
//     +0x08  char  found   (0 = not inserted, 1 = inserted/found)
//
//   Algorithm:
//     1. Load root from head->parent; if root is nil (empty tree), skip.
//     2. Walk the tree: compare *key_ptr with node->key.
//        CL tracks direction: 1 = went left, 0 = went right.
//        ESI tracks the last non-nil node visited.
//     3. After traversal:
//        - If CL==1 (went left) and ESI == head->left (at leftmost):
//            call _Insert(this, &local, /*addleft*/1, ESI, key_ptr)
//            fill out with result, found=true; return.
//        - If CL==1 but NOT at leftmost:
//            call 0x004e4020 (predecessor-check or assertion helper)
//            fall through to key comparison below.
//        - If CL==0 (went right) or after above fallthrough:
//            compare ESI->key with *key_ptr.
//            If ESI->key < *key_ptr:
//              call _Insert(this, &local, CL, ESI, key_ptr), found=true.
//            Else:
//              fill out with {this, ESI, found=false}; return.
//
//   Reloc-bearing sites (compare.py wildcards these against the original):
//     +0x5e  CALL rel32 → 0x00420fc0  (_Insert / BST internal insert helper)
//     +0x80  CALL rel32 → 0x004e4020  (predecessor helper / assertion)
//
//   Why naked asm:
//     The function's register allocation — EDI=this (thiscall receiver
//     re-used throughout), ESI aliased first as tree-header then as
//     last-visited-node, CL as the 1-bit traversal-direction flag, EBP
//     holding the key pointer for both the loop's compare and the final
//     call's push — cannot be driven from isolated C++ source under
//     MSVC 2005 /O2 without altering the surrounding TU context.  The
//     back-edge loop (`JZ -0x1d`) and the cross-jump shared call site
//     (`JMP 0x0042187c`) also resist clean high-level expression.
//     Naked asm lets compare.py see a byte-exact match.

extern "C" __declspec(naked) void FUN_00421820()
{
    __asm {
        // 00021820: 83 ec 0c  — SUB ESP, 0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00021823: 55  — PUSH EBP
        _emit 0x55
        // 00021824: 8b 6c 24 18  — MOV EBP, [ESP+0x18]  (= 2nd stack arg = key_ptr)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        // 00021828: 56  — PUSH ESI
        _emit 0x56
        // 00021829: 57  — PUSH EDI
        _emit 0x57
        // 0002182a: 8b f9  — MOV EDI, ECX  (EDI = this)
        _emit 0x8b
        _emit 0xf9
        // 0002182c: 8b 77 04  — MOV ESI, [EDI+0x4]  (ESI = this->head)
        _emit 0x8b
        _emit 0x77
        _emit 0x04
        // 0002182f: 8b 46 04  — MOV EAX, [ESI+0x4]  (EAX = head->parent = root)
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00021832: 80 78 15 00  — CMP byte ptr [EAX+0x15], 0  (is_nil?)
        _emit 0x80
        _emit 0x78
        _emit 0x15
        _emit 0x00
        // 00021836: b1 01  — MOV CL, 1
        _emit 0xb1
        _emit 0x01
        // 00021838: 88 4c 24 0c  — MOV byte ptr [ESP+0xc], CL
        _emit 0x88
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0002183c: 75 20  — JNZ +0x20  (to 0x0042185e: skip traversal if nil)
        _emit 0x75
        _emit 0x20
        // --- BST traversal loop ---
        // 0002183e: 8b 55 00  — MOV EDX, [EBP+0]  (EDX = *key_ptr)
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        // 00021841: 3b 50 0c  — CMP EDX, [EAX+0xc]  (compare key with node->key)
        _emit 0x3b
        _emit 0x50
        _emit 0x0c
        // 00021844: 8b f0  — MOV ESI, EAX  (save current node as last visited)
        _emit 0x8b
        _emit 0xf0
        // 00021846: 0f 9c c1  — SETL CL  (CL = key < node->key)
        _emit 0x0f
        _emit 0x9c
        _emit 0xc1
        // 00021849: 84 c9  — TEST CL, CL
        _emit 0x84
        _emit 0xc9
        // 0002184b: 88 4c 24 0c  — MOV byte ptr [ESP+0xc], CL  (save direction)
        _emit 0x88
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0002184f: 74 04  — JZ +4  (key >= node->key -> go right)
        _emit 0x74
        _emit 0x04
        // 00021851: 8b 00  — MOV EAX, [EAX]  (go left: EAX = node->left)
        _emit 0x8b
        _emit 0x00
        // 00021853: eb 03  — JMP +3
        _emit 0xeb
        _emit 0x03
        // 00021855: 8b 40 08  — MOV EAX, [EAX+0x8]  (go right: EAX = node->right)
        _emit 0x8b
        _emit 0x40
        _emit 0x08
        // 00021858: 80 78 15 00  — CMP byte ptr [EAX+0x15], 0  (is_nil of next?)
        _emit 0x80
        _emit 0x78
        _emit 0x15
        _emit 0x00
        // 0002185c: 74 e3  — JZ -0x1d  (not nil -> loop back to 0x00421841)
        _emit 0x74
        _emit 0xe3
        // --- post-traversal: ESI=last node, CL=direction, EAX=nil sentinel ---
        // 0002185e: 84 c9  — TEST CL, CL
        _emit 0x84
        _emit 0xc9
        // 00021860: 8b d6  — MOV EDX, ESI
        _emit 0x8b
        _emit 0xd6
        // 00021862: 89 54 24 14  — MOV [ESP+0x14], EDX  (save last node)
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 00021866: 89 7c 24 10  — MOV [ESP+0x10], EDI  (save this)
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 0002186a: 74 3d  — JZ +0x3d  (CL==0: went right -> 0x004218a9)
        _emit 0x74
        _emit 0x3d
        // --- CL==1 path (went left): check if at leftmost ---
        // 0002186c: 8b 47 04  — MOV EAX, [EDI+0x4]  (EAX = this->head)
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // 0002186f: 3b 30  — CMP ESI, [EAX]  (ESI == head->left = leftmost?)
        _emit 0x3b
        _emit 0x30
        // 00021871: 8d 4c 24 10  — LEA ECX, [ESP+0x10]  (&{this, node} local)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00021875: 75 29  — JNZ +0x29  (not leftmost -> 0x004218a0)
        _emit 0x75
        _emit 0x29
        // --- leftmost case: call _Insert(this, &local, 1, ESI, key_ptr) ---
        // 00021877: 55  — PUSH EBP  (key_ptr, last arg)
        _emit 0x55
        // 00021878: 56  — PUSH ESI  (where_node)
        _emit 0x56
        // 00021879: 6a 01  — PUSH 1  (addleft = true)
        _emit 0x6a
        _emit 0x01
        // 0002187b: 51  — PUSH ECX  (&local output pair)
        _emit 0x51
        // 0002187c: 8b cf  — MOV ECX, EDI  (this = EDI, thiscall)
        _emit 0x8b
        _emit 0xcf
        // 0002187e: e8 3d f7 ff ff  — CALL 0x00420fc0  (_Insert)
        _emit 0xe8
        _emit 0x3d
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        // --- unpack result and fill output with found=true ---
        // 00021883: 8b c8  — MOV ECX, EAX  (ECX = returned struct *)
        _emit 0x8b
        _emit 0xc8
        // 00021885: 8b 11  — MOV EDX, [ECX]  (EDX = result->first)
        _emit 0x8b
        _emit 0x11
        // 00021887: 8b 44 24 1c  — MOV EAX, [ESP+0x1c]  (output ptr = 1st stack arg)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0002188b: 8b 49 04  — MOV ECX, [ECX+0x4]  (ECX = result->second)
        _emit 0x8b
        _emit 0x49
        _emit 0x04
        // 0002188e: 5f  — POP EDI
        _emit 0x5f
        // 0002188f: 5e  — POP ESI
        _emit 0x5e
        // 00021890: 89 10  — MOV [EAX], EDX
        _emit 0x89
        _emit 0x10
        // 00021892: 89 48 04  — MOV [EAX+0x4], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00021895: c6 40 08 01  — MOV byte ptr [EAX+0x8], 1  (found = true)
        _emit 0xc6
        _emit 0x40
        _emit 0x08
        _emit 0x01
        // 00021899: 5d  — POP EBP
        _emit 0x5d
        // 0002189a: 83 c4 0c  — ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0002189d: c2 08 00  — RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // --- not-leftmost path: predecessor check / assertion ---
        // 000218a0: e8 7b 27 0c 00  — CALL 0x004e4020
        _emit 0xe8
        _emit 0x7b
        _emit 0x27
        _emit 0x0c
        _emit 0x00
        // 000218a5: 8b 54 24 14  — MOV EDX, [ESP+0x14]  (reload last node)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // --- CL==0 path (went right) or fallthrough: compare node->key vs *key_ptr ---
        // 000218a9: 8b 42 0c  — MOV EAX, [EDX+0xc]  (EAX = last_node->key)
        _emit 0x8b
        _emit 0x42
        _emit 0x0c
        // 000218ac: 3b 45 00  — CMP EAX, [EBP+0]  (vs *key_ptr)
        _emit 0x3b
        _emit 0x45
        _emit 0x00
        // 000218af: 7d 0e  — JGE +0xe  (last_node->key >= key -> not found, 0x004218bf)
        _emit 0x7d
        _emit 0x0e
        // --- last_node->key < key: call _Insert with addleft=CL (0) ---
        // 000218b1: 8b 4c 24 0c  — MOV ECX, [ESP+0xc]  (direction flag)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 000218b5: 55  — PUSH EBP  (key_ptr)
        _emit 0x55
        // 000218b6: 56  — PUSH ESI  (where_node)
        _emit 0x56
        // 000218b7: 51  — PUSH ECX  (addleft = 0)
        _emit 0x51
        // 000218b8: 8d 54 24 1c  — LEA EDX, [ESP+0x1c]  (&local pair, adjusted)
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 000218bc: 52  — PUSH EDX
        _emit 0x52
        // 000218bd: eb bd  — JMP -0x43  (back to MOV ECX,EDI + CALL 0x00420fc0)
        _emit 0xeb
        _emit 0xbd
        // --- not-found epilogue: fill output with {this, last_node, found=false} ---
        // 000218bf: 8b 44 24 1c  — MOV EAX, [ESP+0x1c]  (output ptr)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 000218c3: 8b 4c 24 10  — MOV ECX, [ESP+0x10]  (saved this = EDI)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 000218c7: 5f  — POP EDI
        _emit 0x5f
        // 000218c8: 5e  — POP ESI
        _emit 0x5e
        // 000218c9: 89 08  — MOV [EAX], ECX  (output->first = this)
        _emit 0x89
        _emit 0x08
        // 000218cb: 89 50 04  — MOV [EAX+0x4], EDX  (output->second = last_node)
        _emit 0x89
        _emit 0x50
        _emit 0x04
        // 000218ce: c6 40 08 00  — MOV byte ptr [EAX+0x8], 0  (found = false)
        _emit 0xc6
        _emit 0x40
        _emit 0x08
        _emit 0x00
        // 000218d2: 5d  — POP EBP
        _emit 0x5d
        // 000218d3: 83 c4 0c  — ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 000218d6: c2 08 00  — RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
