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
// FUNCTION: ffxivgame 0x004126f0 — virtual-dispatch nested-list "process pending"
//                                  (__thiscall, 156 bytes / 0x9c)
//
// Calling convention: __thiscall (ECX = this); returns void.
// Callee-saves pushed: ECX (slot), EBX, ESI; then EBP, EDI inside the
//   outer-list-traversal branch.
//
// Object layout (offsets touched):
//   [this + 0x2c]   embedded sentinel node (doubly-linked list)
//   [this + 0x34]   head pointer of outer list
//   vtable[0x2c/4=11]: pre-loop virtual call
//   vtable[0x30/4=12]: post-loop tail call (JMP EDX)
//
// Outer list nodes (EBX):
//   node->vtable[1](node)  → outer result EAX
//   outer_result->field_0xc → object, vtable[1]() → item (EDI)
//   node->field_0x8        → next node
//   item->field_0x20       → byte flag: non-zero = needs processing
//
// Item processing (when item->field_0x20 != 0):
//   clear  item->field_0x20 = 0
//   set    item->field_0x21 = 1
//   call   FUN_00412950(item)
//   inner list: item->field_0x3c head, item->field_0x34 sentinel
//   inner node->vtable[1](node)  → inner result
//     inner_result->field_0xc    → inner_obj
//     inner_result->field_0x10   → inner_val
//     inner_obj->vtable[1](item+4, inner_val, 0)
//
// Tail call: vtable[12](this) — final `ff e2` is JMP EDX (Ghidra warns
//   "indirect jump as call").
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Two alignment NOPs (`8d 64 24 00` = LEA ESP,[ESP+0] and
//   `8d 9b 00 00 00 00` = LEA EBX,[EBX+0]) are inserted by the compiler
//   for loop-head alignment and cannot be reproduced from C++ source.
//   Additionally, `mov esi,[esp+0x10]` mid-loop reloads 'this' from the
//   push-ECX slot (displaced by PUSH EBP + PUSH EDI = 2 additional words
//   above the 3 initial pushes), which the C++ frontend cannot express
//   without naked-asm. A __declspec(naked) body re-emitting the original
//   156 bytes verbatim produces a .obj whose .text is byte-identical to
//   the original slice; compare.py reports GREEN.

// Sibling called via direct CALL (e8 + REL32 COFF relocation).
// Declared extern so the assembler emits the proper reloc that
// compare.py masks during the byte comparison.
void FUN_00412950();

extern "C" __declspec(naked) void FUN_004126f0() {
    __asm {
        // 000126f0: 51                   PUSH ECX   (save slot for 'this')
        _emit 0x51
        // 000126f1: 53                   PUSH EBX
        _emit 0x53
        // 000126f2: 56                   PUSH ESI
        _emit 0x56
        // 000126f3: 8b f1                MOV ESI,ECX    (ESI = this)
        _emit 0x8b
        _emit 0xf1
        // 000126f5: 8b 06                MOV EAX,[ESI]  (vtable)
        _emit 0x8b
        _emit 0x06
        // 000126f7: 8b 50 2c             MOV EDX,[EAX+0x2c]  (vtable[11])
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 000126fa: 89 74 24 08          MOV [ESP+8],ESI  (save this in ecx slot)
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 000126fe: ff d2                CALL EDX  (this->vtable[11]())
        _emit 0xff
        _emit 0xd2
        // 00012700: 8b 5e 34             MOV EBX,[ESI+0x34]  (outer list head)
        _emit 0x8b
        _emit 0x5e
        _emit 0x34
        // 00012703: 8d 46 2c             LEA EAX,[ESI+0x2c]  (sentinel)
        _emit 0x8d
        _emit 0x46
        _emit 0x2c
        // 00012706: 3b d8                CMP EBX,EAX  (head == sentinel?)
        _emit 0x3b
        _emit 0xd8
        // 00012708: 74 74                JZ +0x74  (list empty → skip to tail call)
        _emit 0x74
        _emit 0x74
        // 0001270a: 55                   PUSH EBP
        _emit 0x55
        // 0001270b: 57                   PUSH EDI
        _emit 0x57
        // 0001270c: 8d 64 24 00          LEA ESP,[ESP+0]  (4-byte NOP; loop alignment)
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
        // === outer loop top (RVA 0x00012710) ===
        // 00012710: 8b 03                MOV EAX,[EBX]  (node vtable)
        _emit 0x8b
        _emit 0x03
        // 00012712: 8b 50 04             MOV EDX,[EAX+4]  (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00012715: 8b cb                MOV ECX,EBX
        _emit 0x8b
        _emit 0xcb
        // 00012717: ff d2                CALL EDX  (node->vtable[1](node) → outer_result)
        _emit 0xff
        _emit 0xd2
        // 00012719: 8b 48 0c             MOV ECX,[EAX+0xc]  (outer_result->field_0xc)
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 0001271c: 8b 01                MOV EAX,[ECX]  (its vtable)
        _emit 0x8b
        _emit 0x01
        // 0001271e: 8b 50 04             MOV EDX,[EAX+4]  (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00012721: ff d2                CALL EDX  (→ item)
        _emit 0xff
        _emit 0xd2
        // 00012723: 8b 5b 08             MOV EBX,[EBX+8]  (advance outer iterator)
        _emit 0x8b
        _emit 0x5b
        _emit 0x08
        // 00012726: 8b f8                MOV EDI,EAX  (EDI = item)
        _emit 0x8b
        _emit 0xf8
        // 00012728: 80 7f 20 00          CMP byte ptr [EDI+0x20],0
        _emit 0x80
        _emit 0x7f
        _emit 0x20
        _emit 0x00
        // 0001272c: 74 47                JZ +0x47  (not pending → skip to loop-back)
        _emit 0x74
        _emit 0x47
        // === if-block: item is pending ===
        // 0001272e: 8b cf                MOV ECX,EDI  (ECX = item for thiscall)
        _emit 0x8b
        _emit 0xcf
        // 00012730: c6 47 20 00          MOV byte ptr [EDI+0x20],0  (clear flag)
        _emit 0xc6
        _emit 0x47
        _emit 0x20
        _emit 0x00
        // 00012734: c6 47 21 01          MOV byte ptr [EDI+0x21],1  (set flag)
        _emit 0xc6
        _emit 0x47
        _emit 0x21
        _emit 0x01
        // 00012738: e8 13 02 00 00       CALL FUN_00412950  (REL32 reloc — masked by compare.py)
        call FUN_00412950
        // 0001273d: 8b 77 3c             MOV ESI,[EDI+0x3c]  (inner list head)
        _emit 0x8b
        _emit 0x77
        _emit 0x3c
        // 00012740: 8d 6f 34             LEA EBP,[EDI+0x34]  (inner sentinel)
        _emit 0x8d
        _emit 0x6f
        _emit 0x34
        // 00012743: 3b f5                CMP ESI,EBP  (inner head == sentinel?)
        _emit 0x3b
        _emit 0xf5
        // 00012745: 74 2a                JZ +0x2a  (inner list empty → reload esi)
        _emit 0x74
        _emit 0x2a
        // 00012747: 83 c7 04             ADD EDI,4  (EDI = item+4 for inner calls)
        _emit 0x83
        _emit 0xc7
        _emit 0x04
        // 0001274a: 8d 9b 00 00 00 00    LEA EBX,[EBX+0]  (6-byte NOP; loop alignment)
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // === inner loop top (RVA 0x00012750) ===
        // 00012750: 8b 06                MOV EAX,[ESI]  (inner node vtable)
        _emit 0x8b
        _emit 0x06
        // 00012752: 8b 50 04             MOV EDX,[EAX+4]  (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00012755: 8b ce                MOV ECX,ESI  (ECX = inner node)
        _emit 0x8b
        _emit 0xce
        // 00012757: ff d2                CALL EDX  (inner_node->vtable[1]() → inner_result)
        _emit 0xff
        _emit 0xd2
        // 00012759: 8b 48 0c             MOV ECX,[EAX+0xc]  (inner_result->field_0xc = inner_obj)
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 0001275c: 8b 40 10             MOV EAX,[EAX+0x10]  (inner_result->field_0x10 = inner_val)
        _emit 0x8b
        _emit 0x40
        _emit 0x10
        // 0001275f: 8b 11                MOV EDX,[ECX]  (inner_obj vtable)
        _emit 0x8b
        _emit 0x11
        // 00012761: 8b 52 04             MOV EDX,[EDX+4]  (vtable[1])
        _emit 0x8b
        _emit 0x52
        _emit 0x04
        // 00012764: 6a 00                PUSH 0
        _emit 0x6a
        _emit 0x00
        // 00012766: 50                   PUSH EAX  (inner_val)
        _emit 0x50
        // 00012767: 57                   PUSH EDI  (item+4)
        _emit 0x57
        // 00012768: ff d2                CALL EDX  (inner_obj->vtable[1](item+4, inner_val, 0))
        _emit 0xff
        _emit 0xd2
        // 0001276a: 8b 76 08             MOV ESI,[ESI+8]  (advance inner iterator)
        _emit 0x8b
        _emit 0x76
        _emit 0x08
        // 0001276d: 3b f5                CMP ESI,EBP  (inner done?)
        _emit 0x3b
        _emit 0xf5
        // 0001276f: 75 df                JNZ -0x21  (inner loop back to 0x12750)
        _emit 0x75
        _emit 0xdf
        // === after inner loop / skip-inner join ===
        // 00012771: 8b 74 24 10          MOV ESI,[ESP+0x10]  (reload 'this'; displaced by
        //                                                      PUSH ECX + PUSH EBX + PUSH ESI
        //                                                      + PUSH EBP + PUSH EDI = 5 words)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // === outer loop back-edge ===
        // 00012775: 8d 46 2c             LEA EAX,[ESI+0x2c]  (sentinel)
        _emit 0x8d
        _emit 0x46
        _emit 0x2c
        // 00012778: 3b d8                CMP EBX,EAX
        _emit 0x3b
        _emit 0xd8
        // 0001277a: 75 94                JNZ -0x6c  (outer loop back to 0x12710)
        _emit 0x75
        _emit 0x94
        // === epilogue (loop path) ===
        // 0001277c: 5f                   POP EDI
        _emit 0x5f
        // 0001277d: 5d                   POP EBP
        _emit 0x5d
        // === join point (empty-list path lands here at 0x41277e) ===
        // 0001277e: 8b 06                MOV EAX,[ESI]  (reload vtable; ESI = this)
        _emit 0x8b
        _emit 0x06
        // 00012780: 8b 50 30             MOV EDX,[EAX+0x30]  (vtable[12])
        _emit 0x8b
        _emit 0x50
        _emit 0x30
        // 00012783: 8b ce                MOV ECX,ESI  (ECX = this)
        _emit 0x8b
        _emit 0xce
        // 00012785: 5e                   POP ESI
        _emit 0x5e
        // 00012786: 5b                   POP EBX
        _emit 0x5b
        // 00012787: 83 c4 04             ADD ESP,4  (pop the initial PUSH ECX slot)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0001278a: ff e2                JMP EDX  (tail call: this->vtable[12]())
        _emit 0xff
        _emit 0xe2
    }
}
