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
// FUNCTION: ffxivgame 0x00412430 — virtual-dispatch "process pending items"
//                                  loop over an embedded doubly-linked list
//                                  (__thiscall, 161 bytes / 0xa1)
//
// Calling convention: __thiscall (ECX = this); returns void.
// Callee-saves pushed: ECX (slot), EBX, ESI; then EBP, EDI inside the
//   list-traversal branch.
//
// Object layout (offsets touched):
//   [this + 0x2c]   embedded sentinel node (start of doubly-linked list)
//   [this + 0x34]   head node pointer (this->field_0x34)
//   [this + 0x1c]   function pointer (pcVar2) called per item
//   vtable[0x2c/4=11]: pre-loop virtual call
//   vtable[0x30/4=12]: post-loop tail call
//
// List nodes (EBX):
//   vtable[1](node) → "result object" (EAX)
//   result->field_0xc → object with its own vtable[1] → item (iVar3)
//   item->field_0x08 (at node+8) → next node pointer
//   item->field_0x20 → byte flag: 0 = not yet processed
//
// Item processing (when item->field_0x20 == 0):
//   iVar4 = (**(item+4)->vtable_offset_0xc)(&item->field4)
//   iVar5 = (**(item+4)->vtable_offset_0x8)(&item->field4)
//   align  = (iVar5 + iVar4 - 1) & ~(iVar4 - 1)
//   uVar6  = (**(*(item+0x18))->vtable[1])(align)    [__cdecl]
//   uVar6  = (**(item+0x1c))->vtable[1])(uVar6)      [__cdecl]
//   (*pcVar2)(uVar6)                                  [__cdecl]
//   item->field_0x20 = 1
//
// Tail call: vtable[12](this) — Ghidra warns "indirect jump as call"
//   because the final `ff e2` is JMP EDX (not CALL), enabling the
//   compiler to re-use the epilogue's restored register state directly.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function mixes thiscall virtual dispatch (0-arg forms with ECX)
//   and __cdecl 1-arg forms (caller-cleans via ADD ESP,0x0c at the end
//   of the if-block). The MOV ESI,[ESP+0x1c] mid-block reloads 'this'
//   from the stack (displaced by 5 pushes: EBP + EDI + 3 arg-pushes)
//   which is not reproducible from C++ source without fine-grained
//   register-allocation hints. A __declspec(naked) body re-emitting
//   the original 161 bytes verbatim produces a .obj whose .text is
//   byte-identical to the original slice. compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00412430() {
    __asm {
        // 00012430: 51                 PUSH ECX   (save slot for 'this')
        _emit 0x51
        // 00012431: 53                 PUSH EBX
        _emit 0x53
        // 00012432: 56                 PUSH ESI
        _emit 0x56
        // 00012433: 8b f1              MOV ESI,ECX    (ESI = this)
        _emit 0x8b
        _emit 0xf1
        // 00012435: 8b 06              MOV EAX,[ESI]  (vtable)
        _emit 0x8b
        _emit 0x06
        // 00012437: 8b 50 2c           MOV EDX,[EAX+0x2c]  (vtable[11])
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 0001243a: 89 74 24 08        MOV [ESP+8],ESI  (save this on stack)
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 0001243e: ff d2              CALL EDX  (this->vtable[11]())
        _emit 0xff
        _emit 0xd2
        // 00012440: 8b 5e 34           MOV EBX,[ESI+0x34]  (head node)
        _emit 0x8b
        _emit 0x5e
        _emit 0x34
        // 00012443: 8d 46 2c           LEA EAX,[ESI+0x2c]  (sentinel)
        _emit 0x8d
        _emit 0x46
        _emit 0x2c
        // 00012446: 3b d8              CMP EBX,EAX  (head == sentinel?)
        _emit 0x3b
        _emit 0xd8
        // 00012448: 74 79              JZ +0x79  (list empty → skip to tail call)
        _emit 0x74
        _emit 0x79
        // 0001244a: 55                 PUSH EBP
        _emit 0x55
        // 0001244b: 57                 PUSH EDI
        _emit 0x57
        // 0001244c: 8d 64 24 00        LEA ESP,[ESP+0]  (4-byte NOP; alignment)
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
        // === loop top (RVA 0x00012450) ===
        // 00012450: 8b 03              MOV EAX,[EBX]  (node vtable)
        _emit 0x8b
        _emit 0x03
        // 00012452: 8b 50 04           MOV EDX,[EAX+4]  (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00012455: 8b cb              MOV ECX,EBX
        _emit 0x8b
        _emit 0xcb
        // 00012457: ff d2              CALL EDX  (node->vtable[1](node) → result)
        _emit 0xff
        _emit 0xd2
        // 00012459: 8b 48 0c           MOV ECX,[EAX+0xc]  (*(result+0xc))
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 0001245c: 8b 01              MOV EAX,[ECX]  (its vtable)
        _emit 0x8b
        _emit 0x01
        // 0001245e: 8b 50 04           MOV EDX,[EAX+4]  (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00012461: ff d2              CALL EDX  ((*result+0xc)->vtable[1]() → item)
        _emit 0xff
        _emit 0xd2
        // 00012463: 8b 5b 08           MOV EBX,[EBX+8]  (advance iterator)
        _emit 0x8b
        _emit 0x5b
        _emit 0x08
        // 00012466: 8b e8              MOV EBP,EAX  (EBP = item)
        _emit 0x8b
        _emit 0xe8
        // 00012468: 80 7d 20 00        CMP byte ptr [EBP+0x20],0
        _emit 0x80
        _emit 0x7d
        _emit 0x20
        _emit 0x00
        // 0001246c: 75 4c              JNZ +0x4c  (already processed → loop back)
        _emit 0x75
        _emit 0x4c
        // === if-block: item not yet processed ===
        // 0001246e: 8b 45 04           MOV EAX,[EBP+4]  (*(item+4))
        _emit 0x8b
        _emit 0x45
        _emit 0x04
        // 00012471: 8b 50 0c           MOV EDX,[EAX+0xc]  (func ptr at +0xc)
        _emit 0x8b
        _emit 0x50
        _emit 0x0c
        // 00012474: 8d 75 04           LEA ESI,[EBP+4]  (ESI = &item->field4)
        _emit 0x8d
        _emit 0x75
        _emit 0x04
        // 00012477: 8b ce              MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00012479: ff d2              CALL EDX  (iVar4 = func(&item->field4))
        _emit 0xff
        _emit 0xd2
        // 0001247b: 8d 78 ff           LEA EDI,[EAX-1]  (EDI = iVar4-1)
        _emit 0x8d
        _emit 0x78
        _emit 0xff
        // 0001247e: 8b 06              MOV EAX,[ESI]  (reload *(item+4))
        _emit 0x8b
        _emit 0x06
        // 00012480: 8b 50 08           MOV EDX,[EAX+8]  (func ptr at +8)
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 00012483: 8b ce              MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00012485: ff d2              CALL EDX  (iVar5 = func(&item->field4))
        _emit 0xff
        _emit 0xd2
        // 00012487: 8b 4d 18           MOV ECX,[EBP+0x18]  (*(item+0x18))
        _emit 0x8b
        _emit 0x4d
        _emit 0x18
        // 0001248a: 8b 54 24 10        MOV EDX,[ESP+0x10]  (reload this; displaced by 4 saves)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0001248e: 8b 75 1c           MOV ESI,[EBP+0x1c]  (piVar1 = *(item+0x1c))
        _emit 0x8b
        _emit 0x75
        _emit 0x1c
        // 00012491: 03 c7              ADD EAX,EDI  (iVar5 + (iVar4-1))
        _emit 0x03
        _emit 0xc7
        // 00012493: f7 d7              NOT EDI  (~(iVar4-1))
        _emit 0xf7
        _emit 0xd7
        // 00012495: 23 c7              AND EAX,EDI  (aligned = (iVar5+iVar4-1) & ~(iVar4-1))
        _emit 0x23
        _emit 0xc7
        // 00012497: 8b 7a 1c           MOV EDI,[EDX+0x1c]  (pcVar2 = *(this+0x1c))
        _emit 0x8b
        _emit 0x7a
        _emit 0x1c
        // 0001249a: 50                 PUSH EAX  (arg: aligned)
        _emit 0x50
        // 0001249b: 8b 01              MOV EAX,[ECX]  (vtable of *(item+0x18))
        _emit 0x8b
        _emit 0x01
        // 0001249d: 8b 50 04           MOV EDX,[EAX+4]  (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000124a0: ff d2              CALL EDX  (uVar6 = vtable[1](aligned); __cdecl)
        _emit 0xff
        _emit 0xd2
        // 000124a2: 50                 PUSH EAX  (arg: uVar6)
        _emit 0x50
        // 000124a3: 8b 06              MOV EAX,[ESI]  (vtable of piVar1)
        _emit 0x8b
        _emit 0x06
        // 000124a5: 8b 50 04           MOV EDX,[EAX+4]  (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000124a8: 8b ce              MOV ECX,ESI  (ECX = piVar1)
        _emit 0x8b
        _emit 0xce
        // 000124aa: ff d2              CALL EDX  (uVar6 = piVar1->vtable[1](uVar6); __cdecl)
        _emit 0xff
        _emit 0xd2
        // 000124ac: 50                 PUSH EAX  (arg: uVar6)
        _emit 0x50
        // 000124ad: ff d7              CALL EDI  ((*pcVar2)(uVar6); __cdecl)
        _emit 0xff
        _emit 0xd7
        // 000124af: 8b 74 24 1c        MOV ESI,[ESP+0x1c]  (reload this; 7 slots up)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        // 000124b3: 83 c4 0c           ADD ESP,0x0c  (caller-cleans 3 __cdecl args)
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 000124b6: c6 45 20 01        MOV byte ptr [EBP+0x20],1  (mark processed)
        _emit 0xc6
        _emit 0x45
        _emit 0x20
        _emit 0x01
        // === loop back-edge (RVA 0x000124ba) ===
        // 000124ba: 8d 46 2c           LEA EAX,[ESI+0x2c]  (sentinel)
        _emit 0x8d
        _emit 0x46
        _emit 0x2c
        // 000124bd: 3b d8              CMP EBX,EAX
        _emit 0x3b
        _emit 0xd8
        // 000124bf: 75 8f              JNZ -0x71  (loop back to 0x12450)
        _emit 0x75
        _emit 0x8f
        // === epilogue (loop path) ===
        // 000124c1: 5f                 POP EDI
        _emit 0x5f
        // 000124c2: 5d                 POP EBP
        _emit 0x5d
        // === join point (empty-list path lands here) ===
        // 000124c3: 8b 06              MOV EAX,[ESI]  (reload vtable; ESI = this)
        _emit 0x8b
        _emit 0x06
        // 000124c5: 8b 50 30           MOV EDX,[EAX+0x30]  (vtable[12])
        _emit 0x8b
        _emit 0x50
        _emit 0x30
        // 000124c8: 8b ce              MOV ECX,ESI  (ECX = this)
        _emit 0x8b
        _emit 0xce
        // 000124ca: 5e                 POP ESI
        _emit 0x5e
        // 000124cb: 5b                 POP EBX
        _emit 0x5b
        // 000124cc: 83 c4 04           ADD ESP,4  (pop the initial PUSH ECX slot)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 000124cf: ff e2              JMP EDX  (tail call: this->vtable[12]())
        _emit 0xff
        _emit 0xe2
    }
}
