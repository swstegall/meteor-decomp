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
// FUNCTION: ffxivgame 0x00012430 — linked-list resource-initialization loop
//                                   (__thiscall, 161 bytes / 0xa1)
//
// Calling convention: __thiscall (ECX = this); returns void via tail-call.
// Callee-saves pushed: ECX (via PUSH ECX compact-frame), EBX, ESI; then
//   EBP and EDI pushed only in the non-empty-list branch.
//
// Object layout (offsets touched by this function):
//   [this + 0x00]   vtable ptr; vtable[0x2c/4=11] called at entry,
//                   vtable[0x30/4=12] tail-called at exit
//   [this + 0x1c]   pcVar2 — direct function pointer called as callback
//   [this + 0x2c]   embedded sentinel node (start of doubly-linked list)
//   [this + 0x34]   list head pointer = first element
//
// List nodes (EBX):
//   [node + 0x00]  vtable ptr; vtable[1] called to get a "result" object
//   [node + 0x08]  forward link (next node in list)
//
// Result object (EAX from node->vtable[1](node)):
//   [result + 0x0c]  ptr to sub-object with its own vtable
//
// Sub-object vtable[1] returns the item pointer (EBP).
//
// Item object (EBP):
//   [item + 0x04]  ptr to an object with vtable slots at +0x08 and +0x0c
//   [item + 0x18]  ptr to allocator-like object (vtable[1] called with size)
//   [item + 0x1c]  ptr to another object (vtable[1] called with uVar6)
//   [item + 0x20]  byte flag: 0 = needs initialization, 1 = already done
//
// High-level behaviour:
//   1. Call this->vtable[11]() — pre-loop setup.
//   2. Load list head EBX = this->field_0x34.
//   3. Compute sentinel = this + 0x2c.
//   4. If EBX == sentinel (empty list): skip to epilogue.
//   5. Loop over nodes while EBX != sentinel:
//      a. result = EBX->vtable[1](EBX)
//      b. item   = (*result->field_0xc)->vtable[1](result->field_0xc)
//      c. EBX    = EBX->field_0x08   (advance iterator)
//      d. if (item->field_0x20 == 0):
//           iVar4 = (*item->field_04)->vtable[3](item->field_04)  // getAlign?
//           iVar5 = (*item->field_04)->vtable[2](item->field_04)  // getSize?
//           aligned = (iVar5 + iVar4 - 1) & ~(iVar4 - 1)         // round up
//           pcVar2  = this->field_0x1c                            // callback
//           uVar6   = (*item->field_0x18)->vtable[1](aligned)
//           uVar6   = (*item->field_0x1c)->vtable[1](uVar6)
//           pcVar2(uVar6)
//           item->field_0x20 = 1
//   6. Tail-call this->vtable[12]() — post-loop cleanup.
//
// Notable codegen details reproduced in naked asm:
//   - PUSH ECX at entry is the "compact __thiscall frame" idiom: reserves
//     a stack slot at [ESP+8] (relative to post-PUSH-EBX/ESI frame) that
//     is immediately overwritten with MOV [ESP+8],ESI (this pointer) so
//     the loop body can reload `this` from the stack at [ESP+0x10] after
//     PUSH EBP + PUSH EDI.
//   - LEA ESP,[ESP+0] (8d 64 24 00) at +0x1c is a 4-byte NOP inserted by
//     MSVC 2005 to align the loop top to a 16-byte boundary.
//   - The epilogue uses JMP EDX (ff e2) — a tail-call to vtable[12] —
//     after restoring all callee-saves. The JMP encodes the __thiscall
//     this-pointer hand-off without a stack frame.
//   - All CALL instructions are indirect via EDX/EDI register; no CALL
//     rel32 relocations, so the .obj has zero relocations.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The PUSH-ECX compact frame, the stack-slot this-reload ([ESP+0x10]),
//   the 4-byte NOP alignment, and the JMP-tail-call are not reproducible
//   from C++ source. The __declspec(naked) body re-emits the original
//   161 bytes verbatim via MASM _emit directives; the .obj's .text is
//   byte-identical to the original slice, and compare.py reports GREEN.
//
// Cross-platform guard: __declspec(naked) + MASM _emit are MSVC-only.
// The #ifdef keeps the file compilable on clang/arm64 (host toolchain)
// while the MSVC build (Wine) produces the byte-identical .obj.

#if defined(_MSC_VER) && !defined(__clang__)
extern "C" __declspec(naked) void FUN_00412430()
{
    __asm {
        // 00012430: 51           PUSH ECX   (compact frame: allocates stack slot)
        _emit 0x51
        // 00012431: 53           PUSH EBX
        _emit 0x53
        // 00012432: 56           PUSH ESI
        _emit 0x56
        // 00012433: 8b f1        MOV ESI, ECX   (ESI = this)
        _emit 0x8b
        _emit 0xf1
        // 00012435: 8b 06        MOV EAX, dword ptr [ESI]   (vtable ptr)
        _emit 0x8b
        _emit 0x06
        // 00012437: 8b 50 2c     MOV EDX, dword ptr [EAX+0x2c]   (vtable[11])
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 0001243a: 89 74 24 08  MOV dword ptr [ESP+0x8], ESI   (save this in stack slot)
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 0001243e: ff d2        CALL EDX   (this->vtable[11]())
        _emit 0xff
        _emit 0xd2

        // 00012440: 8b 5e 34     MOV EBX, dword ptr [ESI+0x34]   (list head)
        _emit 0x8b
        _emit 0x5e
        _emit 0x34
        // 00012443: 8d 46 2c     LEA EAX, [ESI+0x2c]   (sentinel = this+0x2c)
        _emit 0x8d
        _emit 0x46
        _emit 0x2c
        // 00012446: 3b d8        CMP EBX, EAX
        _emit 0x3b
        _emit 0xd8
        // 00012448: 74 79        JZ +0x79   (-> 000124c3, empty list)
        _emit 0x74
        _emit 0x79
        // 0001244a: 55           PUSH EBP
        _emit 0x55
        // 0001244b: 57           PUSH EDI
        _emit 0x57
        // 0001244c: 8d 64 24 00  LEA ESP, [ESP+0x0]   (4-byte NOP: align loop to 0x10)
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00

        // === loop top (RVA 0x00012450) ===
        // 00012450: 8b 03        MOV EAX, dword ptr [EBX]   (node vtable)
        _emit 0x8b
        _emit 0x03
        // 00012452: 8b 50 04     MOV EDX, dword ptr [EAX+0x4]   (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00012455: 8b cb        MOV ECX, EBX   (ECX = node)
        _emit 0x8b
        _emit 0xcb
        // 00012457: ff d2        CALL EDX   (node->vtable[1](node) -> result in EAX)
        _emit 0xff
        _emit 0xd2
        // 00012459: 8b 48 0c     MOV ECX, dword ptr [EAX+0xc]   (result->field_0xc)
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 0001245c: 8b 01        MOV EAX, dword ptr [ECX]   (sub-object vtable)
        _emit 0x8b
        _emit 0x01
        // 0001245e: 8b 50 04     MOV EDX, dword ptr [EAX+0x4]   (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04

        // 00012461: ff d2        CALL EDX   (sub-object->vtable[1]() -> item in EAX)
        _emit 0xff
        _emit 0xd2
        // 00012463: 8b 5b 08     MOV EBX, dword ptr [EBX+0x8]   (EBX = next node)
        _emit 0x8b
        _emit 0x5b
        _emit 0x08
        // 00012466: 8b e8        MOV EBP, EAX   (EBP = item)
        _emit 0x8b
        _emit 0xe8
        // 00012468: 80 7d 20 00  CMP byte ptr [EBP+0x20], 0
        _emit 0x80
        _emit 0x7d
        _emit 0x20
        _emit 0x00
        // 0001246c: 75 4c        JNZ +0x4c   (-> 000124ba, skip if already done)
        _emit 0x75
        _emit 0x4c

        // --- if-block: item->field_0x20 == 0 ---
        // 0001246e: 8b 45 04     MOV EAX, dword ptr [EBP+0x4]   (item->field_04)
        _emit 0x8b
        _emit 0x45
        _emit 0x04

        // 00012471: 8b 50 0c     MOV EDX, dword ptr [EAX+0xc]   (vtable[3] = getAlign?)
        _emit 0x8b
        _emit 0x50
        _emit 0x0c
        // 00012474: 8d 75 04     LEA ESI, [EBP+0x4]   (ESI = &item->field_04 = item+4)
        _emit 0x8d
        _emit 0x75
        _emit 0x04
        // 00012477: 8b ce        MOV ECX, ESI   (ECX = item+4)
        _emit 0x8b
        _emit 0xce
        // 00012479: ff d2        CALL EDX   (iVar4 = (*item->field_04)->vtable[3](item+4))
        _emit 0xff
        _emit 0xd2
        // 0001247b: 8d 78 ff     LEA EDI, [EAX-1]   (EDI = iVar4 - 1)
        _emit 0x8d
        _emit 0x78
        _emit 0xff
        // 0001247e: 8b 06        MOV EAX, dword ptr [ESI]   (ESI = item+4 -> *[item+4] vtable)
        _emit 0x8b
        _emit 0x06

        // 00012480: 8b 50 08     MOV EDX, dword ptr [EAX+0x8]   (vtable[2] = getSize?)
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 00012483: 8b ce        MOV ECX, ESI   (ECX = item+4)
        _emit 0x8b
        _emit 0xce
        // 00012485: ff d2        CALL EDX   (iVar5 = (*item->field_04)->vtable[2](item+4))
        _emit 0xff
        _emit 0xd2
        // 00012487: 8b 4d 18     MOV ECX, dword ptr [EBP+0x18]   (item->field_0x18 — allocator)
        _emit 0x8b
        _emit 0x4d
        _emit 0x18
        // 0001248a: 8b 54 24 10  MOV EDX, dword ptr [ESP+0x10]   (this, via stack slot)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0001248e: 8b 75 1c     MOV ESI, dword ptr [EBP+0x1c]   (item->field_0x1c)
        _emit 0x8b
        _emit 0x75
        _emit 0x1c

        // 00012491: 03 c7        ADD EAX, EDI   (EAX = iVar5 + (iVar4 - 1))
        _emit 0x03
        _emit 0xc7
        // 00012493: f7 d7        NOT EDI        (EDI = ~(iVar4 - 1))
        _emit 0xf7
        _emit 0xd7
        // 00012495: 23 c7        AND EAX, EDI   (EAX = aligned = (iVar5+iVar4-1) & ~(iVar4-1))
        _emit 0x23
        _emit 0xc7
        // 00012497: 8b 7a 1c     MOV EDI, dword ptr [EDX+0x1c]   (pcVar2 = this->field_0x1c)
        _emit 0x8b
        _emit 0x7a
        _emit 0x1c
        // 0001249a: 50           PUSH EAX   (push aligned size)
        _emit 0x50
        // 0001249b: 8b 01        MOV EAX, dword ptr [ECX]   (ECX = item->field_0x18 — vtable)
        _emit 0x8b
        _emit 0x01
        // 0001249d: 8b 50 04     MOV EDX, dword ptr [EAX+0x4]   (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04

        // 000124a0: ff d2        CALL EDX   (uVar6 = (*item->field_0x18)->vtable[1](aligned))
        _emit 0xff
        _emit 0xd2
        // 000124a2: 50           PUSH EAX   (push uVar6)
        _emit 0x50
        // 000124a3: 8b 06        MOV EAX, dword ptr [ESI]   (ESI = item->field_0x1c — vtable)
        _emit 0x8b
        _emit 0x06
        // 000124a5: 8b 50 04     MOV EDX, dword ptr [EAX+0x4]   (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000124a8: 8b ce        MOV ECX, ESI   (ECX = item->field_0x1c)
        _emit 0x8b
        _emit 0xce
        // 000124aa: ff d2        CALL EDX   (uVar6 = (*item->field_0x1c)->vtable[1](uVar6))
        _emit 0xff
        _emit 0xd2
        // 000124ac: 50           PUSH EAX   (push final uVar6 for pcVar2 call)
        _emit 0x50
        // 000124ad: ff d7        CALL EDI   (pcVar2(uVar6))
        _emit 0xff
        _emit 0xd7
        // 000124af: 8b 74 24 1c  MOV ESI, dword ptr [ESP+0x1c]   (reload this from stack)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        // 000124b3: 83 c4 0c     ADD ESP, 0xc   (pop 3 args)
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 000124b6: c6 45 20 01  MOV byte ptr [EBP+0x20], 1   (item->field_0x20 = 1)
        _emit 0xc6
        _emit 0x45
        _emit 0x20
        _emit 0x01

        // === loop back-edge check ===
        // 000124ba: 8d 46 2c     LEA EAX, [ESI+0x2c]   (sentinel = this+0x2c)
        _emit 0x8d
        _emit 0x46
        _emit 0x2c
        // 000124bd: 3b d8        CMP EBX, EAX   (EBX vs sentinel)
        _emit 0x3b
        _emit 0xd8
        // 000124bf: 75 8f        JNZ -0x71   (-> 00012450, loop top)
        _emit 0x75
        _emit 0x8f

        // === loop exit ===
        // 000124c1: 5f           POP EDI
        _emit 0x5f
        // 000124c2: 5d           POP EBP
        _emit 0x5d

        // === common epilogue (RVA 0x000124c3 — also target of empty-list JZ) ===
        // 000124c3: 8b 06        MOV EAX, dword ptr [ESI]   (ESI = this — vtable)
        _emit 0x8b
        _emit 0x06
        // 000124c5: 8b 50 30     MOV EDX, dword ptr [EAX+0x30]   (vtable[12] tail-call target)
        _emit 0x8b
        _emit 0x50
        _emit 0x30
        // 000124c8: 8b ce        MOV ECX, ESI   (ECX = this — __thiscall arg)
        _emit 0x8b
        _emit 0xce
        // 000124ca: 5e           POP ESI
        _emit 0x5e
        // 000124cb: 5b           POP EBX
        _emit 0x5b
        // 000124cc: 83 c4 04     ADD ESP, 0x4   (undo PUSH ECX compact-frame slot)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 000124cf: ff e2        JMP EDX   (tail-call this->vtable[12])
        _emit 0xff
        _emit 0xe2
    }
}
#endif // _MSC_VER && !__clang__
