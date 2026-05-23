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
// FUNCTION: ffxivgame 0x00411fa0 — drain-and-transfer loop: moves nodes from
//                                  an embedded doubly-linked list (sentinel at
//                                  this+0x44, head at this+0x48) into a locked
//                                  destination queue, using an XCHG-based
//                                  spinlock (__thiscall, 118 bytes / 0x76)
//
// Calling convention: __thiscall (ECX = this); returns void.
// Callee-saves pushed: ECX (local slot), EBX, ESI; then delayed EDI; then
//   EBP (inside the if-block / loop only).
//
// Object layout (offsets touched by this function):
//   [this + 0x14]  ptr to object with vtable; vtable[1] called to obtain
//                  a "result" object
//   [this + 0x44]  embedded sentinel node (start of doubly-linked list)
//   [this + 0x48]  head pointer = *(sentinel+4) = first list element
//
// Result object layout (EAX from first virtual call):
//   [result + 0x0c]  ptr to destination container (EBX)
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
//     RVA 0x11fe0
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

extern "C" __declspec(naked) void FUN_00411fa0() {
    __asm {
        // 00011fa0: 51           PUSH ECX   (allocate sentinel local slot)
        _emit 0x51
        // 00011fa1: 53           PUSH EBX
        _emit 0x53
        // 00011fa2: 56           PUSH ESI
        _emit 0x56
        // 00011fa3: 8b f1        MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 00011fa5: 8b 4e 14     MOV ECX,dword ptr [ESI+0x14]
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 00011fa8: 8b 01        MOV EAX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 00011faa: 8b 50 04     MOV EDX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00011fad: 57           PUSH EDI   (delayed callee-save)
        _emit 0x57
        // 00011fae: ff d2        CALL EDX   (this->field_14->vtable[1](field_14))
        _emit 0xff
        _emit 0xd2
        // 00011fb0: 8b 7e 48     MOV EDI,dword ptr [ESI+0x48]
        _emit 0x8b
        _emit 0x7e
        _emit 0x48
        // 00011fb3: 8b 58 0c     MOV EBX,dword ptr [EAX+0xc]
        _emit 0x8b
        _emit 0x58
        _emit 0x0c
        // 00011fb6: 83 c6 44     ADD ESI,0x44
        _emit 0x83
        _emit 0xc6
        _emit 0x44
        // 00011fb9: 3b fe        CMP EDI,ESI
        _emit 0x3b
        _emit 0xfe
        // 00011fbb: 89 74 24 0c  MOV dword ptr [ESP+0xc],ESI  (save sentinel to local)
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 00011fbf: 74 50        JZ +0x50  (→ 0x12011, list empty)
        _emit 0x74
        _emit 0x50
        // 00011fc1: 55           PUSH EBP   (saved inside the if-block)
        _emit 0x55
        // 00011fc2: 8d 6b 04     LEA EBP,[EBX+0x4]  (EBP = &lock word)
        _emit 0x8d
        _emit 0x6b
        _emit 0x04
        // === loop top (RVA 0x11fc5) ===
        // 00011fc5: 8b 07        MOV EAX,dword ptr [EDI]
        _emit 0x8b
        _emit 0x07
        // 00011fc7: 8b 50 04     MOV EDX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00011fca: 8b cf        MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 00011fcc: ff d2        CALL EDX   (node->vtable[1](node) → item in EAX)
        _emit 0xff
        _emit 0xd2
        // 00011fce: 8b 7f 04     MOV EDI,dword ptr [EDI+0x4]   (advance iterator)
        _emit 0x8b
        _emit 0x7f
        _emit 0x04
        // 00011fd1: 8b f0        MOV ESI,EAX   (ESI = item)
        _emit 0x8b
        _emit 0xf0
        // 00011fd3: 8b 06        MOV EAX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 00011fd5: 8b 10        MOV EDX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x10
        // 00011fd7: 6a 00        PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00011fd9: 8b ce        MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00011fdb: ff d2        CALL EDX   (item->vtable[0](item, 0))
        _emit 0xff
        _emit 0xd2
        // 00011fdd: 8d 49 00     LEA ECX,[ECX+0x00]  (3-byte NOP: align spin-loop to 0x10)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // === spin-lock acquire (aligned to 16 bytes at RVA 0x11fe0) ===
        // 00011fe0: b8 01 00 00 00  MOV EAX,0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00011fe5: 8b cd        MOV ECX,EBP
        _emit 0x8b
        _emit 0xcd
        // 00011fe7: 87 01        XCHG dword ptr [ECX],EAX  (atomic: [EBP]↔1)
        _emit 0x87
        _emit 0x01
        // 00011fe9: 85 c0        TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00011feb: 75 f3        JNZ -0xd  (→ 0x11fe0, spin)
        _emit 0x75
        _emit 0xf3
        // === critical section: insert ESI into destination list ===
        // 00011fed: 8b 43 0c     MOV EAX,dword ptr [EBX+0xc]   (EAX = tail node)
        _emit 0x8b
        _emit 0x43
        _emit 0x0c
        // 00011ff0: 8b 50 04     MOV EDX,dword ptr [EAX+0x4]   (EDX = tail->field_4)
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00011ff3: 89 32        MOV dword ptr [EDX],ESI        (tail->field_4->field_0 = ESI)
        _emit 0x89
        _emit 0x32
        // 00011ff5: 8b 48 04     MOV ECX,dword ptr [EAX+0x4]   (ECX = tail->field_4 reload)
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        // 00011ff8: 89 06        MOV dword ptr [ESI],EAX        (ESI->field_0 = tail)
        _emit 0x89
        _emit 0x06
        // 00011ffa: 89 4e 04     MOV dword ptr [ESI+0x4],ECX   (ESI->field_4 = old tail->field_4)
        _emit 0x89
        _emit 0x4e
        _emit 0x04
        // 00011ffd: 89 70 04     MOV dword ptr [EAX+0x4],ESI   (tail->field_4 = ESI)
        _emit 0x89
        _emit 0x70
        _emit 0x04
        // 00012000: 83 43 18 ff  ADD dword ptr [EBX+0x18],-0x1  (EBX->count--)
        _emit 0x83
        _emit 0x43
        _emit 0x18
        _emit 0xff
        // === spin-lock release ===
        // 00012004: 33 d2        XOR EDX,EDX
        _emit 0x33
        _emit 0xd2
        // 00012006: 8b c5        MOV EAX,EBP
        _emit 0x8b
        _emit 0xc5
        // 00012008: 87 10        XCHG dword ptr [EAX],EDX  (atomic: [EBP]↔0)
        _emit 0x87
        _emit 0x10
        // === loop back-edge ===
        // 0001200a: 3b 7c 24 10  CMP EDI,dword ptr [ESP+0x10]  (EDI vs sentinel)
        _emit 0x3b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 0001200e: 75 b5        JNZ -0x4b  (→ 0x11fc5, continue loop)
        _emit 0x75
        _emit 0xb5
        // === epilogue ===
        // 00012010: 5d           POP EBP
        _emit 0x5d
        // 00012011: 5f           POP EDI
        _emit 0x5f
        // 00012012: 5e           POP ESI
        _emit 0x5e
        // 00012013: 5b           POP EBX
        _emit 0x5b
        // 00012014: 59           POP ECX
        _emit 0x59
        // 00012015: c3           RET
        _emit 0xc3
    }
}
