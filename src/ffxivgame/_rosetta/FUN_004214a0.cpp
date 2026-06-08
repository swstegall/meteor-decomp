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
// FUNCTION: ffxivgame 0x000214a0 — std::_Tree insert helper (__thiscall, 185 B / 0xb9)
//
// Binary tree lower-bound traversal + conditional insert.  Called by
// std::map / std::set insert paths in the MSVC 2005 STL _Tree template.
//
// Signature (inferred):
//   __thiscall void FUN_004214a0(
//       this,
//       result_pair *out,   // [ESP+4] on entry — 9-byte {ptr,ptr,bool}
//       key_type    *key    // [ESP+8] on entry
//   )
//   RET 8 (callee cleans 2 stack args).
//
// Control flow:
//   1. Load root via this->field4->field4.  While node->isnil == 0, walk
//      the tree: if *key < node->key go left (CL=1) else go right (CL=0),
//      keeping the last visited non-nil node in ESI.
//   2. After the loop, if CL==1 (last step was left):
//      - Assert ESI == header->leftmost (invariant check via 0x00580d70).
//      - Call 0x00420dc0(this, &local_pair, ECX_this, 1, ESI, key) to
//        insert and get back an iterator; copy {iter[0], iter[4], 1} into
//        *out; RET.
//   3. If CL==0 (last step was right):
//      - Check if ESI->key >= *key; if so: *out = {this, ESI, 0}; RET.
//      - Else: call 0x00420dc0 to insert and return {iter[0], iter[4], 1}.
//
// Two internal CALL rel32 sites (masked by compare.py reloc wildcarding):
//   +0x5e  CALL rel32 → 0x00420dc0  (_Tree::_Buynode / insert helper)
//   +0x80  CALL rel32 → 0x00580d70  (assert / _SCL_SECURE_VALIDATE)
//
// Reconstruction strategy: naked-asm byte passthrough.  The complex
// mid-body loop (backward JZ), the interleaved CL flag across two loop
// iterations, and the two-exit epilogue make source-level C++ impractical
// (register allocator picks different slots for ESI/EDX/ECX than orig).
// Emitting the original 185 bytes verbatim via _emit gives a byte-exact
// match; compare.py wildcards the two CALL rel32 windows.

extern "C" __declspec(naked) void FUN_004214a0() {
    __asm {
        // 000214a0: 83 ec 0c — SUB ESP, 0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 000214a3: 55 — PUSH EBP
        _emit 0x55
        // 000214a4: 8b 6c 24 18 — MOV EBP, [ESP+0x18]  (arg2 = key ptr)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        // 000214a8: 56 — PUSH ESI
        _emit 0x56
        // 000214a9: 57 — PUSH EDI
        _emit 0x57
        // 000214aa: 8b f9 — MOV EDI, ECX  (this)
        _emit 0x8b
        _emit 0xf9
        // 000214ac: 8b 77 04 — MOV ESI, [EDI+0x4]  (header node)
        _emit 0x8b
        _emit 0x77
        _emit 0x04
        // 000214af: 8b 46 04 — MOV EAX, [ESI+0x4]  (root = header->_Parent)
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 000214b2: 80 78 11 00 — CMP byte ptr [EAX+0x11], 0x0  (isnil?)
        _emit 0x80
        _emit 0x78
        _emit 0x11
        _emit 0x00
        // 000214b6: b1 01 — MOV CL, 0x1  (pre-load CL=1)
        _emit 0xb1
        _emit 0x01
        // 000214b8: 88 4c 24 0c — MOV [ESP+0xc], CL
        _emit 0x88
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 000214bc: 75 20 — JNZ +0x20  (if nil, skip loop → 0x4214de)
        _emit 0x75
        _emit 0x20
        // --- loop body: 0x4214be ---
        // 000214be: 8b 55 00 — MOV EDX, [EBP+0]  (*key)
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        // 000214c1: 3b 50 0c — CMP EDX, [EAX+0xc]  (*key vs node->key)
        _emit 0x3b
        _emit 0x50
        _emit 0x0c
        // 000214c4: 8b f0 — MOV ESI, EAX  (save current node)
        _emit 0x8b
        _emit 0xf0
        // 000214c6: 0f 92 c1 — SETC CL  (CL = (*key < node->key))
        _emit 0x0f
        _emit 0x92
        _emit 0xc1
        // 000214c9: 84 c9 — TEST CL, CL
        _emit 0x84
        _emit 0xc9
        // 000214cb: 88 4c 24 0c — MOV [ESP+0xc], CL
        _emit 0x88
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 000214cf: 74 04 — JZ +0x4  (not less → go right @ 0x4214d5)
        _emit 0x74
        _emit 0x04
        // 000214d1: 8b 00 — MOV EAX, [EAX]  (left child)
        _emit 0x8b
        _emit 0x00
        // 000214d3: eb 03 — JMP +0x3  (→ 0x4214d8)
        _emit 0xeb
        _emit 0x03
        // 000214d5: 8b 40 08 — MOV EAX, [EAX+0x8]  (right child)
        _emit 0x8b
        _emit 0x40
        _emit 0x08
        // 000214d8: 80 78 11 00 — CMP byte ptr [EAX+0x11], 0x0  (isnil?)
        _emit 0x80
        _emit 0x78
        _emit 0x11
        _emit 0x00
        // 000214dc: 74 e3 — JZ -0x1d  (not nil → loop back @ 0x4214c1)
        _emit 0x74
        _emit 0xe3
        // --- post-loop: 0x4214de ---
        // 000214de: 84 c9 — TEST CL, CL
        _emit 0x84
        _emit 0xc9
        // 000214e0: 8b d6 — MOV EDX, ESI
        _emit 0x8b
        _emit 0xd6
        // 000214e2: 89 54 24 14 — MOV [ESP+0x14], EDX  (save last node)
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 000214e6: 89 7c 24 10 — MOV [ESP+0x10], EDI  (save this)
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 000214ea: 74 3d — JZ +0x3d  (CL==0 → right path @ 0x421529)
        _emit 0x74
        _emit 0x3d
        // --- CL==1 (went left) path: 0x4214ec ---
        // 000214ec: 8b 47 04 — MOV EAX, [EDI+0x4]  (header)
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // 000214ef: 3b 30 — CMP ESI, [EAX]  (ESI == header->leftmost?)
        _emit 0x3b
        _emit 0x30
        // 000214f1: 8d 4c 24 10 — LEA ECX, [ESP+0x10]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 000214f5: 75 29 — JNZ +0x29  (assert fail → 0x421520)
        _emit 0x75
        _emit 0x29
        // 000214f7: 55 — PUSH EBP  (key ptr)
        _emit 0x55
        // 000214f8: 56 — PUSH ESI  (parent node)
        _emit 0x56
        // 000214f9: 6a 01 — PUSH 0x1  (addleft=true)
        _emit 0x6a
        _emit 0x01
        // 000214fb: 51 — PUSH ECX  (&local result pair)
        _emit 0x51
        // 000214fc: 8b cf — MOV ECX, EDI  (this)
        _emit 0x8b
        _emit 0xcf
        // 000214fe: e8 bd f8 ff ff — CALL rel32 → 0x00420dc0
        _emit 0xe8
        _emit 0xbd
        _emit 0xf8
        _emit 0xff
        _emit 0xff
        // 00021503: 8b c8 — MOV ECX, EAX  (iterator ptr)
        _emit 0x8b
        _emit 0xc8
        // 00021505: 8b 11 — MOV EDX, [ECX]  (iter->field0)
        _emit 0x8b
        _emit 0x11
        // 00021507: 8b 44 24 1c — MOV EAX, [ESP+0x1c]  (arg1 = out buffer)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0002150b: 8b 49 04 — MOV ECX, [ECX+0x4]  (iter->field4)
        _emit 0x8b
        _emit 0x49
        _emit 0x04
        // 0002150e: 5f — POP EDI
        _emit 0x5f
        // 0002150f: 5e — POP ESI
        _emit 0x5e
        // 00021510: 89 10 — MOV [EAX], EDX
        _emit 0x89
        _emit 0x10
        // 00021512: 89 48 04 — MOV [EAX+0x4], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00021515: c6 40 08 01 — MOV byte ptr [EAX+0x8], 0x1
        _emit 0xc6
        _emit 0x40
        _emit 0x08
        _emit 0x01
        // 00021519: 5d — POP EBP
        _emit 0x5d
        // 0002151a: 83 c4 0c — ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0002151d: c2 08 00 — RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // --- assert/error path: 0x421520 ---
        // 00021520: e8 4b f8 15 00 — CALL rel32 → 0x00580d70
        _emit 0xe8
        _emit 0x4b
        _emit 0xf8
        _emit 0x15
        _emit 0x00
        // --- CL==0 (went right) path: 0x421525 ---
        // 00021525: 8b 54 24 14 — MOV EDX, [ESP+0x14]  (last node)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 00021529: 8b 42 0c — MOV EAX, [EDX+0xc]  (last_node->key)
        _emit 0x8b
        _emit 0x42
        _emit 0x0c
        // 0002152c: 3b 45 00 — CMP EAX, [EBP+0]  (vs *key)
        _emit 0x3b
        _emit 0x45
        _emit 0x00
        // 0002152f: 73 0e — JNC +0xe  (last_node->key >= *key → end @ 0x42153f)
        _emit 0x73
        _emit 0x0e
        // --- insert-right path: 0x421531 ---
        // 00021531: 8b 4c 24 0c — MOV ECX, [ESP+0xc]  (go_left flag)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 00021535: 55 — PUSH EBP  (key ptr)
        _emit 0x55
        // 00021536: 56 — PUSH ESI  (parent node)
        _emit 0x56
        // 00021537: 51 — PUSH ECX  (go_left)
        _emit 0x51
        // 00021538: 8d 54 24 1c — LEA EDX, [ESP+0x1c]  (&out buffer)
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 0002153c: 52 — PUSH EDX
        _emit 0x52
        // 0002153d: eb bd — JMP -0x43  (→ 0x4214fc, CALL 0x00420dc0)
        _emit 0xeb
        _emit 0xbd
        // --- not-inserted (end) path: 0x42153f ---
        // 0002153f: 8b 44 24 1c — MOV EAX, [ESP+0x1c]  (out buffer)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00021543: 8b 4c 24 10 — MOV ECX, [ESP+0x10]  (saved this)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00021547: 5f — POP EDI
        _emit 0x5f
        // 00021548: 5e — POP ESI
        _emit 0x5e
        // 00021549: 89 08 — MOV [EAX], ECX  (out->ptr0 = this)
        _emit 0x89
        _emit 0x08
        // 0002154b: 89 50 04 — MOV [EAX+0x4], EDX  (out->ptr1 = last_node)
        _emit 0x89
        _emit 0x50
        _emit 0x04
        // 0002154e: c6 40 08 00 — MOV byte ptr [EAX+0x8], 0x0  (not inserted)
        _emit 0xc6
        _emit 0x40
        _emit 0x08
        _emit 0x00
        // 00021552: 5d — POP EBP
        _emit 0x5d
        // 00021553: 83 c4 0c — ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00021556: c2 08 00 — RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
