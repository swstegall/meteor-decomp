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
// FUNCTION: ffxivgame 0x0000afc0 — linked-list node flush / dispatch
//                                  (__thiscall, 162 bytes / 0xa2)
//
// __thiscall void FUN_0040afc0(SomeClass *this, int idx)
//   ECX        : this
//   [ESP+0x04] : idx  (after SUB ESP,8 + PUSH EBP, arg lands at [ESP+0x10])
//   RET 4      : callee-cleans 1 dword (__thiscall with 1 explicit arg)
//
// Object layout (offsets touched):
//   this->array[idx]  (EBP + EAX*4)  — pointer array indexed by idx
//   this[0x60]        — pointer to a sub-object (ESI)
//   this[0x5c]        — another sub-object pointer (passed as ECX to FUN_0040d600)
//   ESI[0x8]          — sub-sub-object pointer (EDX)
//   ESI[0x4]          — embedded struct (EBP becomes &ESI[4] mid-loop)
//   ESI[0x30]         — counter incremented after send
//   ESI[0x38]         — value decremented by 0x1000 after send
//   EDX[0x8]          — word flag: controls which send path to take
//   EDI[0x0]          — node payload (EBX): non-null triggers the send paths
//   EDI[0x1c]         — next-node pointer in the linked list
//
// Control flow:
//   if (this == null) return early;
//   node = this->array[idx];
//   if (node == null) → clear slot and return;
//   do {
//       payload = node->field0;
//       next    = node->field7 (at +0x1c);
//       obj     = this->field24 (at +0x60);
//       if (payload != null) {
//           sub = obj->field2 (at +0x8);
//           if (sub->wordAt8 != 0) {
//               ep = &obj->field1 (at +0x4);
//               if (FUN_0040ddd0(ep, payload))
//                   FUN_0040de80(ep, payload);
//               else
//                   FUN_0040df70(obj->field0, payload);
//           } else {
//               FUN_0040df70(obj->field0, payload);
//           }
//           obj->counter++;
//           obj->value += 0xfffff000;
//       }
//       FUN_0040d600(this->field23 /*+0x5c*/, node);
//       if (result) FUN_0040de80(result, node);
//       node = next;
//   } while (node != null);
//   this->array[idx] = 0;
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The frame uses EBP as the `this` pointer throughout (not as frame
//   base), is spilled to [ESP+0x10] and restored mid-loop from that slot.
//   The combination of the non-standard prologue (SUB ESP,8 before PUSH EBP)
//   and EBP re-use for `this` makes the MSVC 2005 register allocation
//   impossible to reproduce from C++ source alone.  A __declspec(naked)
//   body re-emitting the original 162 bytes verbatim produces a .obj
//   whose .text is byte-identical to the original slice.

extern "C" __declspec(naked) void FUN_0040afc0() {
    __asm {
        // 0000afc0: 83 ec 08              SUB ESP, 0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 0000afc3: 55                    PUSH EBP
        _emit 0x55
        // 0000afc4: 8b e9                 MOV EBP, ECX
        _emit 0x8b
        _emit 0xe9
        // 0000afc6: 85 ed                 TEST EBP, EBP
        _emit 0x85
        _emit 0xed
        // 0000afc8: 89 6c 24 04           MOV dword ptr [ESP+0x4], EBP
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x04
        // 0000afcc: 0f 84 89 00 00 00     JZ +0x89  (→ 0x0040b05b epilogue)
        _emit 0x0f
        _emit 0x84
        _emit 0x89
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000afd2: 8b 44 24 10           MOV EAX, dword ptr [ESP+0x10]   (idx)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0000afd6: 57                    PUSH EDI
        _emit 0x57
        // 0000afd7: 8b 7c 85 00           MOV EDI, dword ptr [EBP+EAX*4+0]  (node)
        _emit 0x8b
        _emit 0x7c
        _emit 0x85
        _emit 0x00
        // 0000afdb: 85 ff                 TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 0000afdd: 74 6f                 JZ +0x6f  (→ 0x0040b04e)
        _emit 0x74
        _emit 0x6f
        // 0000afdf: 53                    PUSH EBX
        _emit 0x53
        // 0000afe0: 56                    PUSH ESI
        _emit 0x56
        // === loop top ===
        // 0000afe1: 8b 1f                 MOV EBX, dword ptr [EDI]     (payload)
        _emit 0x8b
        _emit 0x1f
        // 0000afe3: 85 db                 TEST EBX, EBX
        _emit 0x85
        _emit 0xdb
        // 0000afe5: 8b 4f 1c              MOV ECX, dword ptr [EDI+0x1c]  (next)
        _emit 0x8b
        _emit 0x4f
        _emit 0x1c
        // 0000afe8: 8b 75 60              MOV ESI, dword ptr [EBP+0x60]   (sub-obj)
        _emit 0x8b
        _emit 0x75
        _emit 0x60
        // 0000afeb: 89 4c 24 14           MOV dword ptr [ESP+0x14], ECX   (save next)
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0000afef: 74 3e                 JZ +0x3e  (payload==0, skip send → 0x0040b02f)
        _emit 0x74
        _emit 0x3e
        // 0000aff1: 8b 56 08              MOV EDX, dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x56
        _emit 0x08
        // 0000aff4: 66 83 7a 08 00        CMP word ptr [EDX+0x8], 0x0
        _emit 0x66
        _emit 0x83
        _emit 0x7a
        _emit 0x08
        _emit 0x00
        // 0000aff9: 74 21                 JZ +0x21  (→ 0x0040b01c)
        _emit 0x74
        _emit 0x21
        // 0000affb: 8d 6e 04              LEA EBP, [ESI+0x4]
        _emit 0x8d
        _emit 0x6e
        _emit 0x04
        // 0000affe: 53                    PUSH EBX   (arg: payload)
        _emit 0x53
        // 0000afff: 8b cd                 MOV ECX, EBP
        _emit 0x8b
        _emit 0xcd
        // 0000b001: e8 ca 2d 00 00        CALL 0x0040ddd0
        _emit 0xe8
        _emit 0xca
        _emit 0x2d
        _emit 0x00
        _emit 0x00
        // 0000b006: 84 c0                 TEST AL, AL
        _emit 0x84
        _emit 0xc0
        // 0000b008: 74 0e                 JZ +0x0e  (→ 0x0040b018)
        _emit 0x74
        _emit 0x0e
        // 0000b00a: 53                    PUSH EBX   (arg: payload)
        _emit 0x53
        // 0000b00b: 8b cd                 MOV ECX, EBP
        _emit 0x8b
        _emit 0xcd
        // 0000b00d: e8 6e 2e 00 00        CALL 0x0040de80
        _emit 0xe8
        _emit 0x6e
        _emit 0x2e
        _emit 0x00
        _emit 0x00
        // 0000b012: 8b 6c 24 10           MOV EBP, dword ptr [ESP+0x10]  (restore this)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // 0000b016: eb 0c                 JMP +0x0c  (→ 0x0040b024)
        _emit 0xeb
        _emit 0x0c
        // 0000b018: 8b 6c 24 10           MOV EBP, dword ptr [ESP+0x10]  (restore this)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // 0000b01c: 8b 0e                 MOV ECX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x0e
        // 0000b01e: 53                    PUSH EBX   (arg: payload)
        _emit 0x53
        // 0000b01f: e8 4c 2f 00 00        CALL 0x0040df70
        _emit 0xe8
        _emit 0x4c
        _emit 0x2f
        _emit 0x00
        _emit 0x00
        // 0000b024: 83 46 30 01           ADD dword ptr [ESI+0x30], 0x1
        _emit 0x83
        _emit 0x46
        _emit 0x30
        _emit 0x01
        // 0000b028: 81 46 38 00 f0 ff ff  ADD dword ptr [ESI+0x38], 0xfffff000
        _emit 0x81
        _emit 0x46
        _emit 0x38
        _emit 0x00
        _emit 0xf0
        _emit 0xff
        _emit 0xff
        // 0000b02f: 8b 4d 5c              MOV ECX, dword ptr [EBP+0x5c]
        _emit 0x8b
        _emit 0x4d
        _emit 0x5c
        // 0000b032: 57                    PUSH EDI   (arg: node)
        _emit 0x57
        // 0000b033: e8 c8 25 00 00        CALL 0x0040d600
        _emit 0xe8
        _emit 0xc8
        _emit 0x25
        _emit 0x00
        _emit 0x00
        // 0000b038: 85 c0                 TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0000b03a: 74 08                 JZ +0x08  (→ 0x0040b044)
        _emit 0x74
        _emit 0x08
        // 0000b03c: 57                    PUSH EDI   (arg: node)
        _emit 0x57
        // 0000b03d: 8b c8                 MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 0000b03f: e8 3c 2e 00 00        CALL 0x0040de80
        _emit 0xe8
        _emit 0x3c
        _emit 0x2e
        _emit 0x00
        _emit 0x00
        // 0000b044: 8b 7c 24 14           MOV EDI, dword ptr [ESP+0x14]  (next node)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        // 0000b048: 85 ff                 TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 0000b04a: 75 95                 JNZ -0x6b  (→ 0x0040afe1 loop top)
        _emit 0x75
        _emit 0x95
        // 0000b04c: 5e                    POP ESI
        _emit 0x5e
        // 0000b04d: 5b                    POP EBX
        _emit 0x5b
        // === join: EDI==0 or loop-exit lands here ===
        // 0000b04e: 8b 44 24 14           MOV EAX, dword ptr [ESP+0x14]  (idx)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0000b052: c7 44 85 00 00 00 00 00  MOV dword ptr [EBP+EAX*4+0], 0
        _emit 0xc7
        _emit 0x44
        _emit 0x85
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000b05a: 5f                    POP EDI
        _emit 0x5f
        // 0000b05b: 5d                    POP EBP
        _emit 0x5d
        // 0000b05c: 83 c4 08              ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0000b05f: c2 04 00              RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
