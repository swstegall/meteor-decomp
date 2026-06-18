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
// FUNCTION: ffxivgame 0x000346f0 — `__thiscall` list-iteration + slot-swap
//                                  (344 B / 0x158).
//
// Inspection (read from the disassembly at orig RVA 0x000346f0):
//
//   __thiscall void FUN_004346f0(this);
//
//   ECX = this (saved to ESI).  No arguments, void return (tail-call at end).
//
//   High-level structure:
//
//     // Compute current entry pointer from index
//     int   idx   = this->field_0x30;
//     void* entry = this + 0x38 + idx * 0x10;   // EDI
//
//     // Walk a circular doubly-linked list stored in *entry
//     void* list_head_next = entry[+0x4];        // saved at [ESP+0x14]
//     void* first_node     = *list_head_next;    // saved at [ESP+0x1c]
//     void* cur_ptr        = entry;              // saved at [ESP+0x18]
//
//     // LOOP: process nodes until first_node == list_head_next
//     while (true) {
//         assert(cur_ptr != NULL && cur_ptr == entry); // structural invariant
//         if (first_node == list_head_next) break;
//
//         assert(cur_ptr != NULL);
//         assert(first_node != cur_ptr[+0x4]);
//
//         void* sub = first_node[+0xc];           // EBP = EBX->field_c
//         void* vtbl = sub[0];                    // double vtable deref
//         int   out  = -1;
//         bool  ok   = vtbl[0]->vtable[7](sub, &out, 4, 0);  // virtual call
//
//         bool flag = (!ok && out >= 0);
//         xchg(&sub[+0x4], out);
//         xchg(&sub[+0xc], flag ? 1 : 0);
//
//         advance(&cur_ptr);                      // CALL 0x420630
//         first_node = saved[+0x1c];
//         cur_ptr    = saved[+0x18];
//     }
//
//     // Post-loop: iterate entry->next chain, releasing items with field_0x11==0
//     void* next_item = entry[+0x4][+0x4];
//     cur_item = next_item;
//     if (cur_item->field_0x11 == 0) {
//         do {
//             some_release(entry, cur_item[+0x8]);   // CALL 0xc2bb10
//             void* prev = cur_item;
//             cur_item   = cur_item[0];              // advance via field_0
//             if (prev != NULL) release(prev);       // CALL 0x40df70 (ECX=[prev-4])
//         } while (cur_item->field_0x11 == 0);
//     }
//
//     // Reinitialise the linked-list sentinel in entry
//     entry[+0x4][+0x4] = entry[+0x4];
//     entry[+0x8]        = 0;
//     *entry[+0x4]       = entry[+0x4];
//     entry = entry[+0x4];
//     entry[+0x8] = entry;
//
//     // Virtual call through this->field_0x14
//     this->field_0x14->vtable[0]();
//
//     // Update index fields: field_0x30 = field_0x34; field_0x34 = (field_0x34+1 <= 1) ? field_0x34+1 : 0
//     this->field_0x30 = this->field_0x34;
//     int next_idx = this->field_0x34 + 1;
//     this->field_0x34 = (next_idx <= 1) ? next_idx : 0;
//
//     // Call on this->field_0xc object
//     this->field_0xc->some_method();              // CALL 0x43bf60
//
//     // Toggle global byte flag at [0x01328d90]
//     byte* g = *(byte**)0x01328d90;
//     byte old = g[0];
//     g[0] = (old == 0) ? 1 : 0;
//     g[1] = old;
//
//     // Index into g[+4] array at stride 28 (7*4), call method
//     int  aidx = (uint8_t)g[0];
//     void* arr = *(void**)(g + 4);
//     arr[aidx * 28]->some_method();              // CALL 0x4176b0
//
//     // Tail-call via this->field_0xc
//     JMP 0x0043bf30 (ECX = this->field_0xc)
//
//   Stack frame (ESP-relative after prologue: SUB ESP,0x10 + PUSH EBX/EBP/ESI + MOV ESI,ECX + PUSH EDI):
//     [ESP+0x00] saved EDI
//     [ESP+0x04] saved ESI
//     [ESP+0x08] saved EBP
//     [ESP+0x0c] saved EBX
//     [ESP+0x10] local_0 (output param for virtual call, init -1)
//     [ESP+0x14] local_1 (EAX = list_head_next ptr)
//     [ESP+0x18] local_2 (EBP = current iterator ptr, updated by advance())
//     [ESP+0x1c] local_3 (EBX = first_node, stays fixed through iterations)
//
//   Reloc-bearing sites (REL32 calls and DIR32 absolute loads):
//     +0x2e  REL32 → 0x009d22b4  (assert-failure call)
//     +0x3d  REL32 → 0x009d22b4  (assert-failure call)
//     +0x47  REL32 → 0x009d22b4  (assert-failure call)
//     +0x91  REL32 → 0x00420630  (advance iterator)
//     +0xb6  REL32 → 0x00c2bb10  (some_release)
//     +0xc5  REL32 → 0x0040df70  (release/destroy)
//     +0x10f REL32 → 0x0043bf60  (method on field_0xc)
//     +0x114 DIR32 → 0x01328d90  (global ptr load #1)
//     +0x123 DIR32 → 0x01328d90  (global ptr load #2)
//     +0x12c DIR32 → 0x01328d90  (global ptr load #3)
//     +0x144 REL32 → 0x004176b0  (array element method)
//     +0x153 REL32 → 0x0043bf30  (tail JMP)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function's loop structure, virtual dispatch double-dereference,
//   XCHG sequences, and the SBB/NOT/AND index-cycling idiom all require
//   exact register allocation that a source-level port at /O2 would not
//   reproduce without extensive coaxing.  Naked-asm passthrough (the same
//   approach taken by FUN_00405080, FUN_004014b0, FUN_00408f10, and the
//   rest of this _rosetta set) yields a byte-identical .obj slice.

extern "C" __declspec(naked) void FUN_004346f0() {
    __asm {
        // 000346f0: SUB ESP,0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 000346f3: PUSH EBX
        _emit 0x53
        // 000346f4: PUSH EBP
        _emit 0x55
        // 000346f5: PUSH ESI
        _emit 0x56
        // 000346f6: MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 000346f8: MOV EAX,[ESI+0x30]
        _emit 0x8b
        _emit 0x46
        _emit 0x30
        // 000346fb: SHL EAX,0x4
        _emit 0xc1
        _emit 0xe0
        _emit 0x04
        // 000346fe: PUSH EDI
        _emit 0x57
        // 000346ff: LEA EDI,[EAX+ESI+0x38]
        _emit 0x8d
        _emit 0x7c
        _emit 0x30
        _emit 0x38
        // 00034703: MOV EAX,[EDI+0x4]
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // 00034706: MOV EBX,[EAX]
        _emit 0x8b
        _emit 0x18
        // 00034708: MOV EBP,EDI
        _emit 0x8b
        _emit 0xef
        // 0003470a: MOV [ESP+0x14],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0003470e: MOV [ESP+0x1c],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        // 00034712: MOV [ESP+0x18],EBP
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        // 00034716: TEST EBP,EBP
        _emit 0x85
        _emit 0xed
        // 00034718: JZ 0x0043471e
        _emit 0x74
        _emit 0x04
        // 0003471a: CMP EBP,EDI
        _emit 0x3b
        _emit 0xef
        // 0003471c: JZ 0x00434723
        _emit 0x74
        _emit 0x05
        // 0003471e: CALL 0x009d22b4
        _emit 0xe8
        _emit 0x91
        _emit 0xdb
        _emit 0x59
        _emit 0x00
        // 00034723: CMP EBX,[ESP+0x14]
        _emit 0x3b
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        // 00034727: JZ 0x00434790
        _emit 0x74
        _emit 0x67
        // 00034729: TEST EBP,EBP
        _emit 0x85
        _emit 0xed
        // 0003472b: JNZ 0x00434732
        _emit 0x75
        _emit 0x05
        // 0003472d: CALL 0x009d22b4
        _emit 0xe8
        _emit 0x82
        _emit 0xdb
        _emit 0x59
        _emit 0x00
        // 00034732: CMP EBX,[EBP+0x4]
        _emit 0x3b
        _emit 0x5d
        _emit 0x04
        // 00034735: JNZ 0x0043473c
        _emit 0x75
        _emit 0x05
        // 00034737: CALL 0x009d22b4
        _emit 0xe8
        _emit 0x78
        _emit 0xdb
        _emit 0x59
        _emit 0x00
        // 0003473c: MOV EBP,[EBX+0xc]
        _emit 0x8b
        _emit 0x6b
        _emit 0x0c
        // 0003473f: MOV EAX,[EBP]
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        // 00034742: PUSH 0
        _emit 0x6a
        _emit 0x00
        // 00034744: PUSH 4
        _emit 0x6a
        _emit 0x04
        // 00034746: LEA EDX,[ESP+0x18]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 0003474a: MOV [ESP+0x18],0xffffffff
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00034752: MOV ECX,[EAX]
        _emit 0x8b
        _emit 0x08
        // 00034754: PUSH EDX
        _emit 0x52
        // 00034755: PUSH EAX
        _emit 0x50
        // 00034756: MOV EAX,[ECX+0x1c]
        _emit 0x8b
        _emit 0x41
        _emit 0x1c
        // 00034759: XOR BL,BL
        _emit 0x32
        _emit 0xdb
        // 0003475b: CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0003475d: TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0003475f: MOV EAX,[ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00034763: JNZ 0x0043476b
        _emit 0x75
        _emit 0x06
        // 00034765: TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00034767: JS 0x0043476b
        _emit 0x78
        _emit 0x02
        // 00034769: MOV BL,0x1
        _emit 0xb3
        _emit 0x01
        // 0003476b: MOV ECX,[EBP+0x4]
        _emit 0x8b
        _emit 0x4d
        _emit 0x04
        // 0003476e: XCHG [ECX],EAX
        _emit 0x87
        _emit 0x01
        // 00034770: MOV EAX,[EBP+0xc]
        _emit 0x8b
        _emit 0x45
        _emit 0x0c
        // 00034773: XOR EDX,EDX
        _emit 0x33
        _emit 0xd2
        // 00034775: CMP BL,0x1
        _emit 0x80
        _emit 0xfb
        _emit 0x01
        // 00034778: SETZ DL
        _emit 0x0f
        _emit 0x94
        _emit 0xc2
        // 0003477b: XCHG [EAX],EDX
        _emit 0x87
        _emit 0x10
        // 0003477d: LEA ECX,[ESP+0x18]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00034781: CALL 0x00420630
        _emit 0xe8
        _emit 0xaa
        _emit 0xbe
        _emit 0xfe
        _emit 0xff
        // 00034786: MOV EBX,[ESP+0x1c]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        // 0003478a: MOV EBP,[ESP+0x18]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        // 0003478e: JMP 0x00434716
        _emit 0xeb
        _emit 0x86
        // 00034790: MOV ECX,[EDI+0x4]
        _emit 0x8b
        _emit 0x4f
        _emit 0x04
        // 00034793: MOV EBP,[ECX+0x4]
        _emit 0x8b
        _emit 0x69
        _emit 0x04
        // 00034796: CMP byte ptr [EBP+0x11],0x0
        _emit 0x80
        _emit 0x7d
        _emit 0x11
        _emit 0x00
        // 0003479a: MOV EBX,EBP
        _emit 0x8b
        _emit 0xdd
        // 0003479c: JNZ 0x004347c2
        _emit 0x75
        _emit 0x24
        // 0003479e: MOV EDI,EDI  (align NOP)
        _emit 0x8b
        _emit 0xff
        // 000347a0: MOV EDX,[EBX+0x8]
        _emit 0x8b
        _emit 0x53
        _emit 0x08
        // 000347a3: PUSH EDX
        _emit 0x52
        // 000347a4: MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 000347a6: CALL 0x00c2bb10
        _emit 0xe8
        _emit 0x65
        _emit 0x73
        _emit 0x7f
        _emit 0x00
        // 000347ab: TEST EBP,EBP
        _emit 0x85
        _emit 0xed
        // 000347ad: MOV EBX,[EBX]
        _emit 0x8b
        _emit 0x1b
        // 000347af: JZ 0x004347ba
        _emit 0x74
        _emit 0x09
        // 000347b1: MOV ECX,[EBP-0x4]
        _emit 0x8b
        _emit 0x4d
        _emit 0xfc
        // 000347b4: PUSH EBP
        _emit 0x55
        // 000347b5: CALL 0x0040df70
        _emit 0xe8
        _emit 0xb6
        _emit 0x97
        _emit 0xfd
        _emit 0xff
        // 000347ba: CMP byte ptr [EBX+0x11],0x0
        _emit 0x80
        _emit 0x7b
        _emit 0x11
        _emit 0x00
        // 000347be: MOV EBP,EBX
        _emit 0x8b
        _emit 0xeb
        // 000347c0: JZ 0x004347a0
        _emit 0x74
        _emit 0xde
        // 000347c2: MOV EAX,[EDI+0x4]
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // 000347c5: MOV [EAX+0x4],EAX
        _emit 0x89
        _emit 0x40
        _emit 0x04
        // 000347c8: MOV EAX,[EDI+0x4]
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // 000347cb: MOV [EDI+0x8],0x0
        _emit 0xc7
        _emit 0x47
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000347d2: MOV [EAX],EAX
        _emit 0x89
        _emit 0x00
        // 000347d4: MOV EDI,[EDI+0x4]
        _emit 0x8b
        _emit 0x7f
        _emit 0x04
        // 000347d7: MOV [EDI+0x8],EDI
        _emit 0x89
        _emit 0x7f
        _emit 0x08
        // 000347da: MOV ECX,[ESI+0x14]
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 000347dd: MOV EAX,[ECX]
        _emit 0x8b
        _emit 0x01
        // 000347df: MOV EDX,[EAX]
        _emit 0x8b
        _emit 0x10
        // 000347e1: CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000347e3: MOV EAX,[ESI+0x34]
        _emit 0x8b
        _emit 0x46
        _emit 0x34
        // 000347e6: LEA ECX,[EAX+0x1]
        _emit 0x8d
        _emit 0x48
        _emit 0x01
        // 000347e9: MOV [ESI+0x30],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x30
        // 000347ec: MOV EAX,0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000347f1: CMP EAX,ECX
        _emit 0x3b
        _emit 0xc1
        // 000347f3: SBB EAX,EAX
        _emit 0x1b
        _emit 0xc0
        // 000347f5: NOT EAX
        _emit 0xf7
        _emit 0xd0
        // 000347f7: AND EAX,ECX
        _emit 0x23
        _emit 0xc1
        // 000347f9: MOV ECX,[ESI+0xc]
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        // 000347fc: MOV [ESI+0x34],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x34
        // 000347ff: CALL 0x0043bf60
        _emit 0xe8
        _emit 0x5c
        _emit 0x77
        _emit 0x00
        _emit 0x00
        // 00034804: MOV ECX,[0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 0003480a: MOV AL,[ECX]
        _emit 0x8a
        _emit 0x01
        // 0003480c: TEST AL,AL
        _emit 0x84
        _emit 0xc0
        // 0003480e: SETZ DL
        _emit 0x0f
        _emit 0x94
        _emit 0xc2
        // 00034811: MOV [ECX],DL
        _emit 0x88
        _emit 0x11
        // 00034813: MOV ECX,[0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00034819: MOV [ECX+0x1],AL
        _emit 0x88
        _emit 0x41
        _emit 0x01
        // 0003481c: MOV ECX,[0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00034822: MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00034825: LEA EDX,[EAX*8+0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003482c: SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 0003482e: MOV EAX,[ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00034831: LEA ECX,[EAX+EDX*4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00034834: CALL 0x004176b0
        _emit 0xe8
        _emit 0x77
        _emit 0x2e
        _emit 0xfe
        _emit 0xff
        // 00034839: MOV ECX,[ESI+0xc]
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        // 0003483c: POP EDI
        _emit 0x5f
        // 0003483d: POP ESI
        _emit 0x5e
        // 0003483e: POP EBP
        _emit 0x5d
        // 0003483f: POP EBX
        _emit 0x5b
        // 00034840: ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00034843: JMP 0x0043bf30
        _emit 0xe9
        _emit 0xe8
        _emit 0x76
        _emit 0x00
        _emit 0x00
    }
}
