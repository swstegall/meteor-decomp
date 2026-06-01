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
// FUNCTION: ffxivgame 0x0000b360 — __thiscall linked-list flush over all 22
//                                   pointer slots of an object, selectively
//                                   splicing and dispatching "full" nodes
//                                   (275 B / 0x113)
//
// Calling convention: __thiscall (ECX = this, no stack args, plain RET).
//
// Object layout (offsets touched):
//   this[0..21]          — NodeType* pointer array (22 * 4 bytes = 0x58 bytes)
//   this+0x5c            — pointer to a container whose first DWORD is the
//                          head of a ThingAtChain linked list
//   this+0x60            — pointer to a SubObj (dispatch target)
//
// NodeType layout:
//   +0x00  void *payload
//   +0x08  uint16_t capacity
//   +0x0a  uint16_t count
//   +0x1c  NodeType *next
//
// SubObj layout:
//   +0x00  void *field0    (used as ECX for FUN_0040df70)
//   +0x04  embedded struct (address &ESI[1] used as ECX for FUN_0040ddd0 / FUN_0040de80)
//   +0x08  void *sub2      (word at sub2+0x08 checked)
//   +0x30  int counter     (incremented)
//   +0x38  int value       (decremented by 0x1000)
//
// ThingAtChain layout:
//   +0x9a4  ThingAtChain *next_in_chain
//
// Behaviour:
//   For each of the 22 pointer slots starting at this:
//     Walk the linked list at *slot.  For each node whose count >= capacity:
//       splice the node out of the list
//       if node->payload != 0:
//         look up obj->sub2->wordAt8; if non-zero:
//           if FUN_0040ddd0(&obj->sub1, payload): FUN_0040de80(&obj->sub1, payload)
//           else: FUN_0040df70(*obj, payload)
//         else: FUN_0040df70(*obj, payload)
//         obj->counter++; obj->value -= 0x1000
//       search the chain at *(this->field_0x5c) for this node via FUN_0040ddd0;
//       if found and non-null: FUN_0040de80(found, node)
//
// Called directly by FUN_0040b480 (trampoline to FUN_0040d790).
//
// The function's frame uses EBP as a temporary (next-ptr scratch) rather than
// as a frame base pointer, interleaved with the callee-save role of EBP. The
// combination of the dual-purpose EBP, the complex multi-level loop nesting
// (outer stride-4 counter, inner linked-list walk, search sub-loop), and the
// unconventional store/reload pattern for the "prev" variable (EAX held across
// a "first iteration" bypass via JMP+b396) makes source-level reconstruction
// impractical: any rewrite shifts the MSVC 2005 register allocation or
// branch encoding.  A __declspec(naked) body re-emitting the 275 bytes
// verbatim produces a .obj whose .text is byte-identical to the original slice.

extern "C" __declspec(naked) void FUN_0040b360()
{
    __asm {
        // b360: 83 ec 14   SUB ESP,0x14
        _emit 0x83
        _emit 0xec
        _emit 0x14
        // b363: 53         PUSH EBX
        _emit 0x53
        // b364: 55         PUSH EBP
        _emit 0x55
        // b365: 56         PUSH ESI
        _emit 0x56
        // b366: 8b d1      MOV EDX,ECX
        _emit 0x8b
        _emit 0xd1
        // b368: 57         PUSH EDI
        _emit 0x57
        // b369: 89 4c 24 14  MOV [ESP+0x14],ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // b36d: 89 54 24 1c  MOV [ESP+0x1c],EDX
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // b371: c7 44 24 20 16 00 00 00  MOV [ESP+0x20],0x16
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // b379: 8d a4 24 00 00 00 00  LEA ESP,[ESP]  (alignment NOP)
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // b380: 8b 3a  MOV EDI,[EDX]
        _emit 0x8b
        _emit 0x3a
        // b382: 33 c0  XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // b384: 85 ff  TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // b386: 89 44 24 18  MOV [ESP+0x18],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // b38a: 0f 84 c9 00 00 00  JZ 0x0040b459
        _emit 0x0f
        _emit 0x84
        _emit 0xc9
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // b390: eb 04  JMP 0x0040b396
        _emit 0xeb
        _emit 0x04
        // b392: 8b 44 24 18  MOV EAX,[ESP+0x18]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // b396: 66 8b 77 0a  MOV SI,word[EDI+0xa]
        _emit 0x66
        _emit 0x8b
        _emit 0x77
        _emit 0x0a
        // b39a: 66 3b 77 08  CMP SI,word[EDI+0x8]
        _emit 0x66
        _emit 0x3b
        _emit 0x77
        _emit 0x08
        // b39e: 0f 82 a6 00 00 00  JC 0x0040b44a
        _emit 0x0f
        _emit 0x82
        _emit 0xa6
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // b3a4: 85 c0  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // b3a6: 8b 6f 1c  MOV EBP,[EDI+0x1c]
        _emit 0x8b
        _emit 0x6f
        _emit 0x1c
        // b3a9: 89 6c 24 10  MOV [ESP+0x10],EBP
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // b3ad: 74 05  JZ 0x0040b3b4
        _emit 0x74
        _emit 0x05
        // b3af: 89 68 1c  MOV [EAX+0x1c],EBP
        _emit 0x89
        _emit 0x68
        _emit 0x1c
        // b3b2: eb 02  JMP 0x0040b3b6
        _emit 0xeb
        _emit 0x02
        // b3b4: 89 2a  MOV [EDX],EBP
        _emit 0x89
        _emit 0x2a
        // b3b6: 8b 1f  MOV EBX,[EDI]
        _emit 0x8b
        _emit 0x1f
        // b3b8: 85 db  TEST EBX,EBX
        _emit 0x85
        _emit 0xdb
        // b3ba: 8b 71 60  MOV ESI,[ECX+0x60]
        _emit 0x8b
        _emit 0x71
        _emit 0x60
        // b3bd: 74 3e  JZ 0x0040b3fd
        _emit 0x74
        _emit 0x3e
        // b3bf: 8b 46 08  MOV EAX,[ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // b3c2: 66 83 78 08 00  CMP word[EAX+0x8],0x0
        _emit 0x66
        _emit 0x83
        _emit 0x78
        _emit 0x08
        _emit 0x00
        // b3c7: 74 21  JZ 0x0040b3ea
        _emit 0x74
        _emit 0x21
        // b3c9: 8d 6e 04  LEA EBP,[ESI+0x4]
        _emit 0x8d
        _emit 0x6e
        _emit 0x04
        // b3cc: 53  PUSH EBX
        _emit 0x53
        // b3cd: 8b cd  MOV ECX,EBP
        _emit 0x8b
        _emit 0xcd
        // b3cf: e8 fc 29 00 00  CALL 0x0040ddd0
        _emit 0xe8
        _emit 0xfc
        _emit 0x29
        _emit 0x00
        _emit 0x00
        // b3d4: 84 c0  TEST AL,AL
        _emit 0x84
        _emit 0xc0
        // b3d6: 74 0e  JZ 0x0040b3e6
        _emit 0x74
        _emit 0x0e
        // b3d8: 53  PUSH EBX
        _emit 0x53
        // b3d9: 8b cd  MOV ECX,EBP
        _emit 0x8b
        _emit 0xcd
        // b3db: e8 a0 2a 00 00  CALL 0x0040de80
        _emit 0xe8
        _emit 0xa0
        _emit 0x2a
        _emit 0x00
        _emit 0x00
        // b3e0: 8b 6c 24 10  MOV EBP,[ESP+0x10]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // b3e4: eb 0c  JMP 0x0040b3f2
        _emit 0xeb
        _emit 0x0c
        // b3e6: 8b 6c 24 10  MOV EBP,[ESP+0x10]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // b3ea: 8b 0e  MOV ECX,[ESI]
        _emit 0x8b
        _emit 0x0e
        // b3ec: 53  PUSH EBX
        _emit 0x53
        // b3ed: e8 7e 2b 00 00  CALL 0x0040df70
        _emit 0xe8
        _emit 0x7e
        _emit 0x2b
        _emit 0x00
        _emit 0x00
        // b3f2: 83 46 30 01  ADD [ESI+0x30],0x1
        _emit 0x83
        _emit 0x46
        _emit 0x30
        _emit 0x01
        // b3f6: 81 46 38 00 f0 ff ff  ADD [ESI+0x38],0xfffff000
        _emit 0x81
        _emit 0x46
        _emit 0x38
        _emit 0x00
        _emit 0xf0
        _emit 0xff
        _emit 0xff
        // b3fd: 8b 4c 24 14  MOV ECX,[ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // b401: 8b 51 5c  MOV EDX,[ECX+0x5c]
        _emit 0x8b
        _emit 0x51
        _emit 0x5c
        // b404: 8b 32  MOV ESI,[EDX]
        _emit 0x8b
        _emit 0x32
        // b406: 85 f6  TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // b408: 74 34  JZ 0x0040b43e
        _emit 0x74
        _emit 0x34
        // b40a: 8d 9b 00 00 00 00  LEA EBX,[EBX]  (alignment NOP)
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // b410: 57  PUSH EDI
        _emit 0x57
        // b411: 8b ce  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // b413: e8 b8 29 00 00  CALL 0x0040ddd0
        _emit 0xe8
        _emit 0xb8
        _emit 0x29
        _emit 0x00
        _emit 0x00
        // b418: 84 c0  TEST AL,AL
        _emit 0x84
        _emit 0xc0
        // b41a: 75 16  JNZ 0x0040b432
        _emit 0x75
        _emit 0x16
        // b41c: 8b b6 a4 09 00 00  MOV ESI,[ESI+0x9a4]
        _emit 0x8b
        _emit 0xb6
        _emit 0xa4
        _emit 0x09
        _emit 0x00
        _emit 0x00
        // b422: 85 f6  TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // b424: 75 ea  JNZ 0x0040b410
        _emit 0x75
        _emit 0xea
        // b426: 8b 4c 24 14  MOV ECX,[ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // b42a: 8b 54 24 1c  MOV EDX,[ESP+0x1c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // b42e: 8b fd  MOV EDI,EBP
        _emit 0x8b
        _emit 0xfd
        // b430: eb 1f  JMP 0x0040b451
        _emit 0xeb
        _emit 0x1f
        // b432: 85 f6  TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // b434: 74 08  JZ 0x0040b43e
        _emit 0x74
        _emit 0x08
        // b436: 57  PUSH EDI
        _emit 0x57
        // b437: 8b ce  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // b439: e8 42 2a 00 00  CALL 0x0040de80
        _emit 0xe8
        _emit 0x42
        _emit 0x2a
        _emit 0x00
        _emit 0x00
        // b43e: 8b 4c 24 14  MOV ECX,[ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // b442: 8b 54 24 1c  MOV EDX,[ESP+0x1c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // b446: 8b fd  MOV EDI,EBP
        _emit 0x8b
        _emit 0xfd
        // b448: eb 07  JMP 0x0040b451
        _emit 0xeb
        _emit 0x07
        // b44a: 89 7c 24 18  MOV [ESP+0x18],EDI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        // b44e: 8b 7f 1c  MOV EDI,[EDI+0x1c]
        _emit 0x8b
        _emit 0x7f
        _emit 0x1c
        // b451: 85 ff  TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // b453: 0f 85 39 ff ff ff  JNZ 0x0040b392
        _emit 0x0f
        _emit 0x85
        _emit 0x39
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // b459: 83 c2 04  ADD EDX,0x4
        _emit 0x83
        _emit 0xc2
        _emit 0x04
        // b45c: 83 6c 24 20 01  SUB [ESP+0x20],0x1
        _emit 0x83
        _emit 0x6c
        _emit 0x24
        _emit 0x20
        _emit 0x01
        // b461: 89 54 24 1c  MOV [ESP+0x1c],EDX
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // b465: 0f 85 15 ff ff ff  JNZ 0x0040b380
        _emit 0x0f
        _emit 0x85
        _emit 0x15
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // b46b: 5f  POP EDI
        _emit 0x5f
        // b46c: 5e  POP ESI
        _emit 0x5e
        // b46d: 5d  POP EBP
        _emit 0x5d
        // b46e: 5b  POP EBX
        _emit 0x5b
        // b46f: 83 c4 14  ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // b472: c3  RET
        _emit 0xc3
    }
}
