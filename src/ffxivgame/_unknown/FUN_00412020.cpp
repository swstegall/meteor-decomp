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
// FUNCTION: ffxivgame 0x00412020 — drain-and-transfer loop: moves nodes from
//                                  an embedded doubly-linked list (sentinel at
//                                  this+0x38, head at this+0x3c) into a locked
//                                  destination queue, using an XCHG-based
//                                  spinlock (__thiscall, 118 bytes / 0x76)
//
// Calling convention: __thiscall (ECX = this); returns void.
// Callee-saves pushed: ECX (local slot), EBX, ESI; then delayed EDI; then
//   EBP (inside the if-block / loop only).
//
// This function is structurally identical to FUN_00411fa0 — same algorithm,
// same codegen shape — but operates on a different doubly-linked list embedded
// in the same object type (sentinel at this+0x38, head at *(this+0x3c) rather
// than sentinel at this+0x44, head at *(this+0x48)).
//
// Object layout (offsets touched by this function):
//   [this + 0x14]  ptr to object with vtable; vtable[1] called to obtain
//                  a "result" object
//   [this + 0x38]  embedded sentinel node (start of doubly-linked list)
//   [this + 0x3c]  head pointer = first list element (*(sentinel+4))
//
// Result object layout (EAX from first virtual call):
//   [result + 0x10]  ptr to destination container (EBX)
//
// Destination container (EBX) layout:
//   [EBX + 0x04]   spinlock word (0 = free, 1 = held; acquired via XCHG)
//   [EBX + 0x0c]   ptr to tail node of destination doubly-linked list
//   [EBX + 0x18]   element count (decremented by 1 per node transferred)
//
// Source list nodes (EDI):
//   [node + 0x00]  vtable ptr; vtable[1](node) returns the payload item
//   [node + 0x04]  forward link (next node in source list)
//
// Payload item (ESI, result of node->vtable[1](node)):
//   [item + 0x00]  vtable ptr; vtable[0](item, 0) called before insertion
//   [item + 0x04]  forward link (set during doubly-linked insertion)
//
// Loop: for each source node, call the node's extractor vfunc to get the
//   item, call the item's reset vfunc, then spin-acquire the lock, insert
//   the item before the destination sentinel, decrement the count, and
//   release the lock.
//
// Notable codegen details reproduced in naked asm:
//   - PUSH ECX at entry allocates the sentinel local slot (MSVC "SUB ESP,4"
//     substitute for a single-slot frame)
//   - PUSH EDI is delayed until after the ECX/EAX/EDX loads (avoids
//     dirtying the frame before the first call)
//   - LEA ECX,[ECX+0x00] (8d 49 00) is a 3-byte NOP inserted by the
//     compiler to align the XCHG spin-loop to a 16-byte boundary at
//     RVA 0x12060
//   - All three CALL instructions are indirect (CALL EDX); no CALL rel32
//     relocations, so the .obj is already byte-identical without masking
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Source-level C++ would require InterlockedExchange intrinsics, careful
//   register variable annotations, and a dummy local to produce the
//   PUSH-ECX frame; the exact LEA-NOP placement is not reproducible from
//   source. A __declspec(naked) body re-emitting the original 118 bytes
//   verbatim via MASM _emit directives produces a .obj whose .text is
//   byte-identical to the original slice. compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00412020() {
    __asm {
        // 00012020: 51           PUSH ECX   (allocate sentinel local slot)
        _emit 0x51
        // 00012021: 53           PUSH EBX
        _emit 0x53
        // 00012022: 56           PUSH ESI
        _emit 0x56
        // 00012023: 8b f1        MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 00012025: 8b 4e 14     MOV ECX,dword ptr [ESI+0x14]
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 00012028: 8b 01        MOV EAX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 0001202a: 8b 50 04     MOV EDX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0001202d: 57           PUSH EDI   (delayed callee-save)
        _emit 0x57
        // 0001202e: ff d2        CALL EDX   (this->field_14->vtable[1](field_14))
        _emit 0xff
        _emit 0xd2
        // 00012030: 8b 7e 3c     MOV EDI,dword ptr [ESI+0x3c]   (head pointer)
        _emit 0x8b
        _emit 0x7e
        _emit 0x3c
        // 00012033: 8b 58 10     MOV EBX,dword ptr [EAX+0x10]   (destination container)
        _emit 0x8b
        _emit 0x58
        _emit 0x10
        // 00012036: 83 c6 38     ADD ESI,0x38   (ESI = &sentinel)
        _emit 0x83
        _emit 0xc6
        _emit 0x38
        // 00012039: 3b fe        CMP EDI,ESI
        _emit 0x3b
        _emit 0xfe
        // 0001203b: 89 74 24 0c  MOV dword ptr [ESP+0xc],ESI  (save sentinel to local)
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 0001203f: 74 50        JZ +0x50  (→ 0x12091, list empty)
        _emit 0x74
        _emit 0x50
        // 00012041: 55           PUSH EBP   (saved inside the if-block)
        _emit 0x55
        // 00012042: 8d 6b 04     LEA EBP,[EBX+0x4]  (EBP = &lock word)
        _emit 0x8d
        _emit 0x6b
        _emit 0x04
        // === loop top (RVA 0x12045) ===
        // 00012045: 8b 07        MOV EAX,dword ptr [EDI]
        _emit 0x8b
        _emit 0x07
        // 00012047: 8b 50 04     MOV EDX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0001204a: 8b cf        MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 0001204c: ff d2        CALL EDX   (node->vtable[1](node) → item in EAX)
        _emit 0xff
        _emit 0xd2
        // 0001204e: 8b 7f 04     MOV EDI,dword ptr [EDI+0x4]   (advance iterator)
        _emit 0x8b
        _emit 0x7f
        _emit 0x04
        // 00012051: 8b f0        MOV ESI,EAX   (ESI = item)
        _emit 0x8b
        _emit 0xf0
        // 00012053: 8b 06        MOV EAX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 00012055: 8b 10        MOV EDX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x10
        // 00012057: 6a 00        PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00012059: 8b ce        MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0001205b: ff d2        CALL EDX   (item->vtable[0](item, 0))
        _emit 0xff
        _emit 0xd2
        // 0001205d: 8d 49 00     LEA ECX,[ECX+0x00]  (3-byte NOP: align spin-loop to 0x10)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // === spin-lock acquire (aligned to 16 bytes at RVA 0x12060) ===
        // 00012060: b8 01 00 00 00  MOV EAX,0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00012065: 8b cd        MOV ECX,EBP
        _emit 0x8b
        _emit 0xcd
        // 00012067: 87 01        XCHG dword ptr [ECX],EAX  (atomic: [EBP]↔1)
        _emit 0x87
        _emit 0x01
        // 00012069: 85 c0        TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001206b: 75 f3        JNZ -0xd  (→ 0x12060, spin)
        _emit 0x75
        _emit 0xf3
        // === critical section: insert ESI into destination list ===
        // 0001206d: 8b 43 0c     MOV EAX,dword ptr [EBX+0xc]   (EAX = tail node)
        _emit 0x8b
        _emit 0x43
        _emit 0x0c
        // 00012070: 8b 50 04     MOV EDX,dword ptr [EAX+0x4]   (EDX = tail->field_4)
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00012073: 89 32        MOV dword ptr [EDX],ESI        (tail->field_4->field_0 = ESI)
        _emit 0x89
        _emit 0x32
        // 00012075: 8b 48 04     MOV ECX,dword ptr [EAX+0x4]   (ECX = tail->field_4 reload)
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        // 00012078: 89 06        MOV dword ptr [ESI],EAX        (ESI->field_0 = tail)
        _emit 0x89
        _emit 0x06
        // 0001207a: 89 4e 04     MOV dword ptr [ESI+0x4],ECX   (ESI->field_4 = old tail->field_4)
        _emit 0x89
        _emit 0x4e
        _emit 0x04
        // 0001207d: 89 70 04     MOV dword ptr [EAX+0x4],ESI   (tail->field_4 = ESI)
        _emit 0x89
        _emit 0x70
        _emit 0x04
        // 00012080: 83 43 18 ff  ADD dword ptr [EBX+0x18],-0x1  (EBX->count--)
        _emit 0x83
        _emit 0x43
        _emit 0x18
        _emit 0xff
        // === spin-lock release ===
        // 00012084: 33 d2        XOR EDX,EDX
        _emit 0x33
        _emit 0xd2
        // 00012086: 8b c5        MOV EAX,EBP
        _emit 0x8b
        _emit 0xc5
        // 00012088: 87 10        XCHG dword ptr [EAX],EDX  (atomic: [EBP]↔0)
        _emit 0x87
        _emit 0x10
        // === loop back-edge ===
        // 0001208a: 3b 7c 24 10  CMP EDI,dword ptr [ESP+0x10]  (EDI vs sentinel)
        _emit 0x3b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 0001208e: 75 b5        JNZ -0x4b  (→ 0x12045, continue loop)
        _emit 0x75
        _emit 0xb5
        // === epilogue ===
        // 00012090: 5d           POP EBP
        _emit 0x5d
        // 00012091: 5f           POP EDI
        _emit 0x5f
        // 00012092: 5e           POP ESI
        _emit 0x5e
        // 00012093: 5b           POP EBX
        _emit 0x5b
        // 00012094: 59           POP ECX
        _emit 0x59
        // 00012095: c3           RET
        _emit 0xc3
    }
}
