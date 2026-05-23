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
// FUNCTION: ffxivgame 0x000122f0 — RemovableHeapBlock::AddListener (109 bytes / 0x6d)
//           __thiscall, 3 stack args
//
// ECX = this (RemovableHeapBlock*).
// this->field_0x10 is a pointer to a RemovableHeapSpace object (with a vtable).
// this->field_0x38 is a pointer to the sentinel node of the listener
//                  doubly-linked list.
//
// Register layout (differs from the SeparateHeapBlock variant at 0x10cb0):
//   ESI = this->m_space  (cached early, PUSH ESI after MOV EBX,ECX)
//   EDI = node           (allocated HandleListenerElement, or NULL)
//   EBX = this initially, then OVERWRITTEN with head-sentinel pointer
//         after the allocation (list insertion step reuses EBX)
//
// Behaviour:
//   space = this->field_0x10                     ; ESI throughout
//   space->vtable[0x2c](space)                   ; BeginLock()
//   block = this->field_0x10->vtable[0x4](space) ; GetBlock() (reloads from [EBX+0x10])
//   node  = FUN_004109a0(block->field_0x10)       ; allocate listener element
//   if (node) {
//       node->next   = node;
//       node->prev   = node;
//       node->vtable = RemovableHeapBlock::HandleListenerElement::vftable (0x00F56DD4)
//       node->field_c  = param1;   [ESP+0x10]
//       node->field_10 = param2;   [ESP+0x14]
//   } else {
//       node = NULL;
//   }
//   // Insert node before sentinel (append to tail):
//   EBX = this->field_0x38;       ; sentinel head — overwrites EBX = this
//   head->prev->next = node;
//   node->prev       = head->prev;
//   node->next       = head;
//   head->prev       = node;
//   space->vtable[0x30](space)                   ; EndLock() (called via EAX)
//   return node;
//
// Reconstruction: naked-asm byte passthrough.
//
// Key artefacts vs the SeparateHeapBlock sibling (0x10cb0):
//   1. ESI = space (not EDI); EDI = node (not ESI).
//   2. PUSH EDI is deferred until after ESI and the vtable pointer are
//      loaded — MSVC emits the callee-save push when it first needs a
//      free register, not necessarily at function entry.
//   3. block->field_0x10 (offset 0x10) is passed to FUN_004109a0 — the
//      sibling used field_0x14.
//   4. EBX is re-used for the head-sentinel pointer in the list
//      insertion step (MOV EBX, [EBX+0x38] overwrites `this`).
//   5. EndLock is dispatched via EAX (MOV EAX,[EDX+0x30]; CALL EAX)
//      vs EDX in the sibling.
//   6. RET 0xc (3 stack args); Ghidra only recovered 2 because param3
//      is not referenced inside the body.
//
// Globals / constants referenced (absolute VAs in the 1.23b image):
//   0x00F56DD4 — RemovableHeapBlock::HandleListenerElement::vftable
//
// Relocations (masked by tools/compare.py):
//   DIR32: 0xF56DD4 (@vtable MOV)
//   REL32: CALL FUN_004109a0

extern "C" __declspec(naked) void FUN_004122f0()
{
    __asm {
        // Prologue — ESI cached before EDI pushed
        // 000122f0:  53                 PUSH EBX
        _emit 0x53
        // 000122f1:  8b d9              MOV EBX, ECX        ; EBX = this
        _emit 0x8b
        _emit 0xd9
        // 000122f3:  56                 PUSH ESI
        _emit 0x56
        // 000122f4:  8b 73 10           MOV ESI, [EBX+0x10] ; ESI = this->m_space
        _emit 0x8b
        _emit 0x73
        _emit 0x10
        // 000122f7:  8b 06              MOV EAX, [ESI]       ; vtable ptr
        _emit 0x8b
        _emit 0x06
        // 000122f9:  8b 50 2c           MOV EDX, [EAX+0x2c] ; vtable[0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 000122fc:  57                 PUSH EDI             ; deferred callee-save
        _emit 0x57
        // 000122fd:  8b ce              MOV ECX, ESI         ; ECX = space
        _emit 0x8b
        _emit 0xce
        // 000122ff:  ff d2              CALL EDX             ; BeginLock()
        _emit 0xff
        _emit 0xd2
        // ---- GetBlock: reload this->m_space from [EBX+0x10] ----
        // 00012301:  8b 4b 10           MOV ECX, [EBX+0x10] ; reload this->m_space
        _emit 0x8b
        _emit 0x4b
        _emit 0x10
        // 00012304:  8b 01              MOV EAX, [ECX]       ; vtable ptr
        _emit 0x8b
        _emit 0x01
        // 00012306:  8b 50 04           MOV EDX, [EAX+0x4]  ; vtable[0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00012309:  ff d2              CALL EDX             ; GetBlock()
        _emit 0xff
        _emit 0xd2
        // 0001230b:  8b 48 10           MOV ECX, [EAX+0x10] ; block->field_0x10
        _emit 0x8b
        _emit 0x48
        _emit 0x10
        // ---- Allocate listener element ----
        // 0001230e:  e8 xx xx xx xx     CALL FUN_004109a0    ; REL32 reloc
        _emit 0xe8
        _emit 0x8d
        _emit 0xe6
        _emit 0xff
        _emit 0xff
        // 00012313:  85 c0              TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00012315:  74 1e              JZ +0x1e             ; null path (→ 00012335)
        _emit 0x74
        _emit 0x1e
        // ---- non-null path: initialise node ----
        // 00012317:  8b 4c 24 10        MOV ECX, [ESP+0x10]  ; param_1
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0001231b:  8b 54 24 14        MOV EDX, [ESP+0x14]  ; param_2
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0001231f:  89 40 04           MOV [EAX+0x4], EAX   ; node->next = self
        _emit 0x89
        _emit 0x40
        _emit 0x04
        // 00012322:  89 40 08           MOV [EAX+0x8], EAX   ; node->prev = self
        _emit 0x89
        _emit 0x40
        _emit 0x08
        // 00012325:  c7 00 d4 6d f5 00  MOV [EAX], 0x00F56DD4  ; DIR32: vftable
        _emit 0xc7
        _emit 0x00
        _emit 0xd4
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        // 0001232b:  89 48 0c           MOV [EAX+0xc], ECX   ; node->field_c  = param_1
        _emit 0x89
        _emit 0x48
        _emit 0x0c
        // 0001232e:  89 50 10           MOV [EAX+0x10], EDX  ; node->field_10 = param_2
        _emit 0x89
        _emit 0x50
        _emit 0x10
        // 00012331:  8b f8              MOV EDI, EAX         ; EDI = node
        _emit 0x8b
        _emit 0xf8
        // 00012333:  eb 02              JMP +0x2             ; → 00012337 (list_insert)
        _emit 0xeb
        _emit 0x02
        // ---- null path ----
        // 00012335:  33 ff              XOR EDI, EDI         ; EDI = NULL
        _emit 0x33
        _emit 0xff
        // ---- list insertion: append node before sentinel ----
        // EBX is reused — overwritten with head-sentinel pointer
        // 00012337:  8b 5b 38           MOV EBX, [EBX+0x38] ; EBX = sentinel head
        _emit 0x8b
        _emit 0x5b
        _emit 0x38
        // 0001233a:  8b 43 08           MOV EAX, [EBX+0x8]  ; EAX = sentinel->prev
        _emit 0x8b
        _emit 0x43
        _emit 0x08
        // 0001233d:  89 78 04           MOV [EAX+0x4], EDI  ; sentinel->prev->next = node
        _emit 0x89
        _emit 0x78
        _emit 0x04
        // 00012340:  8b 4b 08           MOV ECX, [EBX+0x8]  ; ECX = sentinel->prev (reload)
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 00012343:  89 4f 08           MOV [EDI+0x8], ECX  ; node->prev = sentinel->prev
        _emit 0x89
        _emit 0x4f
        _emit 0x08
        // 00012346:  89 5f 04           MOV [EDI+0x4], EBX  ; node->next = sentinel
        _emit 0x89
        _emit 0x5f
        _emit 0x04
        // 00012349:  89 7b 08           MOV [EBX+0x8], EDI  ; sentinel->prev = node
        _emit 0x89
        _emit 0x7b
        _emit 0x08
        // ---- EndLock: space->vtable[0x30]() via ESI / EAX ----
        // 0001234c:  8b 16              MOV EDX, [ESI]       ; ESI = space, load vtable
        _emit 0x8b
        _emit 0x16
        // 0001234e:  8b 42 30           MOV EAX, [EDX+0x30] ; vtable[0x30]
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 00012351:  8b ce              MOV ECX, ESI         ; ECX = space
        _emit 0x8b
        _emit 0xce
        // 00012353:  ff d0              CALL EAX             ; EndLock() via EAX
        _emit 0xff
        _emit 0xd0
        // ---- epilogue ----
        // 00012355:  8b c7              MOV EAX, EDI         ; return node
        _emit 0x8b
        _emit 0xc7
        // 00012357:  5f                 POP EDI
        _emit 0x5f
        // 00012358:  5e                 POP ESI
        _emit 0x5e
        // 00012359:  5b                 POP EBX
        _emit 0x5b
        // 0001235a:  c2 0c 00           RET 0xc              ; __thiscall, 3 stack args
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
