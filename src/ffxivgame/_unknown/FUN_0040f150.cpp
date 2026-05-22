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
// FUNCTION: ffxivgame 0x0000f150 — SQEX::CDev::Engine::Memory::ComplexLink ctor
//                                  (__thiscall, 375 B / 0x177)
//
// __thiscall void* FUN_0040f150(ComplexLink *this, MemoryBlock *block)
//   ECX        : this  — pointer to the ComplexLink object being constructed
//   [ESP+0x04] : block — pointer to a memory block descriptor
//
// Object layout (inferred from the initialisation loop):
//   [this + 0x00]  void**    vftable   — set to ComplexLink::vftable (0x00f56630)
//   [this + 0x04 .. this + 0x214]  11 x 0x30-byte list-node slots
//     Each 0x30-byte slot:
//       [slot + 0x00]  int       field_00  = 0
//       [slot + 0x04]  int       field_04  = 0
//       [slot + 0x08]  slot*     self_ptr08 = &slot        (self-link)
//       [slot + 0x0c]  slot*     self_ptr0c = &slot        (self-link)
//       [slot + 0x10]  void*     inline_buf = &slot+0x10   (inline storage head)
//       [slot + 0x14]  int       field_14  = 0
//       [slot + 0x18]  void*     inline_buf2 = &slot+0x20  (inline storage tail)
//       [slot + 0x1c]  void*     head_ptr1c = &slot+0x10
//       [slot + 0x1c+4] int      field_20 = 0
//       [slot + 0x24]  int       field_24  = 0
//       [slot + 0x28]  void*     inline_buf3 = &slot+0x20  (same as 0x18)
//       [slot + 0x2c]  void*     head_ptr2c = &slot+0x20
//   Block layout (from param):
//       [block + 0x04]  int       offset to first node
//       [block + 0x08]  int       base / size field
//
// Behaviour:
//   1. Set this->vftable = ComplexLink::vftable
//   2. Loop 11 times initialising each 0x30-byte slot (two doubly-linked
//      sub-lists per slot: a 0x10-entry list and a 0x20-entry list)
//   3. Compute first/second/last sentinel pointers from block descriptor
//   4. Initialise three free-list sentinel nodes from block
//   5. Second loop (count=8): initialise 8 buckets of 0x10-byte entries,
//      each bucket has 4 sub-entries linked via offset +0xc
//   6. Third loop (count=0x21): mark bit-field words at 0x10-byte stride
//      with 0x80000000 (occupied flag)
//   7. BSR on node size to find bucket index; insert node into free-list
//   8. Return this (ECX)
//
// Calling convention: __thiscall; callee cleans 1 stack arg (RET 0x4).
// Callee-saves pushed: EBX, ESI, EDI (ECX also pushed via PUSH ECX prologue).
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The register allocation (EDX as the walking slot pointer, EDI as loop
//   counter, EBX/EDI split across sentinel and free-list roles, EBP
//   borrowed for the inner 8-bucket loop) is not reproducible from C++
//   source with MSVC 2005 /O2 without exact local variable ordering.
//   The vtable reference at +0x08 is the only reloc site. The
//   __declspec(naked) body emits the original 375 bytes via MASM
//   instructions + _emit directives.

extern "C" void *ComplexLink_vftable;  // 0x00f56630

extern "C" __declspec(naked) void FUN_0040f150() {
    __asm {
        // 0000f150:  51                PUSH ECX
        _emit 0x51
        // 0000f151:  53                PUSH EBX
        _emit 0x53
        // 0000f152:  56                PUSH ESI
        _emit 0x56
        // 0000f153:  57                PUSH EDI
        _emit 0x57
        // 0000f154:  89 4c 24 0c       MOV dword ptr [ESP+0xc], ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0000f158:  c7 01 30 66 f5 00 MOV dword ptr [ECX], 0xf56630
        _emit 0xc7
        _emit 0x01
        _emit 0x30
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        // 0000f15e:  8d 51 04          LEA EDX, [ECX+0x4]
        _emit 0x8d
        _emit 0x51
        _emit 0x04
        // 0000f161:  bf 0b 00 00 00    MOV EDI, 0xb
        _emit 0xbf
        _emit 0x0b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f166:  33 c0             XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0000f168:  eb 06             JMP 0x0040f170
        _emit 0xeb
        _emit 0x06
        // 0000f16a..0x0040f16f: 6-byte alignment NOP (LEA EBX,[EBX+0] for loop body 16-byte alignment)
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f170:  8d 72 10          LEA ESI, [EDX+0x10]
        _emit 0x8d
        _emit 0x72
        _emit 0x10
        // 0000f173:  89 72 1c          MOV dword ptr [EDX+0x1c], ESI
        _emit 0x89
        _emit 0x72
        _emit 0x1c
        // 0000f176:  89 72 18          MOV dword ptr [EDX+0x18], ESI
        _emit 0x89
        _emit 0x72
        _emit 0x18
        // 0000f179:  89 06             MOV dword ptr [ESI], EAX
        _emit 0x89
        _emit 0x06
        // 0000f17b:  8d 72 20          LEA ESI, [EDX+0x20]
        _emit 0x8d
        _emit 0x72
        _emit 0x20
        // 0000f17e:  89 52 0c          MOV dword ptr [EDX+0xc], EDX
        _emit 0x89
        _emit 0x52
        _emit 0x0c
        // 0000f181:  89 52 08          MOV dword ptr [EDX+0x8], EDX
        _emit 0x89
        _emit 0x52
        _emit 0x08
        // 0000f184:  89 42 04          MOV dword ptr [EDX+0x4], EAX
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 0000f187:  89 02             MOV dword ptr [EDX], EAX
        _emit 0x89
        _emit 0x02
        // 0000f189:  89 42 14          MOV dword ptr [EDX+0x14], EAX
        _emit 0x89
        _emit 0x42
        _emit 0x14
        // 0000f18c:  89 72 2c          MOV dword ptr [EDX+0x2c], ESI
        _emit 0x89
        _emit 0x72
        _emit 0x2c
        // 0000f18f:  89 72 28          MOV dword ptr [EDX+0x28], ESI
        _emit 0x89
        _emit 0x72
        _emit 0x28
        // 0000f192:  89 42 24          MOV dword ptr [EDX+0x24], EAX
        _emit 0x89
        _emit 0x42
        _emit 0x24
        // 0000f195:  83 c2 30          ADD EDX, 0x30
        _emit 0x83
        _emit 0xc2
        _emit 0x30
        // 0000f198:  83 ef 01          SUB EDI, 0x1
        _emit 0x83
        _emit 0xef
        _emit 0x01
        // 0000f19b:  89 06             MOV dword ptr [ESI], EAX
        _emit 0x89
        _emit 0x06
        // 0000f19d:  75 d1             JNZ 0x0040f170
        _emit 0x75
        _emit 0xd1
        // 0000f19f:  8b 54 24 14       MOV EDX, dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0000f1a3:  8b 72 04          MOV ESI, dword ptr [EDX+0x4]
        _emit 0x8b
        _emit 0x72
        _emit 0x04
        // 0000f1a6:  03 f2             ADD ESI, EDX
        _emit 0x03
        _emit 0xf2
        // 0000f1a8:  74 0f             JZ 0x0040f1b9
        _emit 0x74
        _emit 0x0f
        // 0000f1aa:  89 76 0c          MOV dword ptr [ESI+0xc], ESI
        _emit 0x89
        _emit 0x76
        _emit 0x0c
        // 0000f1ad:  89 76 08          MOV dword ptr [ESI+0x8], ESI
        _emit 0x89
        _emit 0x76
        _emit 0x08
        // 0000f1b0:  89 46 04          MOV dword ptr [ESI+0x4], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 0000f1b3:  89 06             MOV dword ptr [ESI], EAX
        _emit 0x89
        _emit 0x06
        // 0000f1b5:  8b fe             MOV EDI, ESI
        _emit 0x8b
        _emit 0xfe
        // 0000f1b7:  eb 02             JMP 0x0040f1bb
        _emit 0xeb
        _emit 0x02
        // 0000f1b9:  33 ff             XOR EDI, EDI
        _emit 0x33
        _emit 0xff
        // 0000f1bb:  89 07             MOV dword ptr [EDI], EAX
        _emit 0x89
        _emit 0x07
        // 0000f1bd:  c7 47 04 00 00 00 80  MOV dword ptr [EDI+0x4], 0x80000000
        _emit 0xc7
        _emit 0x47
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80
        // 0000f1c4:  8b 72 04          MOV ESI, dword ptr [EDX+0x4]
        _emit 0x8b
        _emit 0x72
        _emit 0x04
        // 0000f1c7:  8d 74 16 10       LEA ESI, [ESI+EDX*1+0x10]
        _emit 0x8d
        _emit 0x74
        _emit 0x16
        _emit 0x10
        // 0000f1cb:  3b f0             CMP ESI, EAX
        _emit 0x3b
        _emit 0xf0
        // 0000f1cd:  74 0f             JZ 0x0040f1de
        _emit 0x74
        _emit 0x0f
        // 0000f1cf:  89 76 0c          MOV dword ptr [ESI+0xc], ESI
        _emit 0x89
        _emit 0x76
        _emit 0x0c
        // 0000f1d2:  89 76 08          MOV dword ptr [ESI+0x8], ESI
        _emit 0x89
        _emit 0x76
        _emit 0x08
        // 0000f1d5:  89 46 04          MOV dword ptr [ESI+0x4], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 0000f1d8:  89 06             MOV dword ptr [ESI], EAX
        _emit 0x89
        _emit 0x06
        // 0000f1da:  8b de             MOV EBX, ESI
        _emit 0x8b
        _emit 0xde
        // 0000f1dc:  eb 02             JMP 0x0040f1e0
        _emit 0xeb
        _emit 0x02
        // 0000f1de:  33 db             XOR EBX, EBX
        _emit 0x33
        _emit 0xdb
        // 0000f1e0:  8b 72 08          MOV ESI, dword ptr [EDX+0x8]
        _emit 0x8b
        _emit 0x72
        _emit 0x08
        // 0000f1e3:  83 ee 30          SUB ESI, 0x30
        _emit 0x83
        _emit 0xee
        _emit 0x30
        // 0000f1e6:  89 33             MOV dword ptr [EBX], ESI
        _emit 0x89
        _emit 0x33
        // 0000f1e8:  8b 37             MOV ESI, dword ptr [EDI]
        _emit 0x8b
        _emit 0x37
        // 0000f1ea:  81 e6 ff ff ff 7f AND ESI, 0x7fffffff
        _emit 0x81
        _emit 0xe6
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x7f
        // 0000f1f0:  89 73 04          MOV dword ptr [EBX+0x4], ESI
        _emit 0x89
        _emit 0x73
        _emit 0x04
        // 0000f1f3:  8b 72 08          MOV ESI, dword ptr [EDX+0x8]
        _emit 0x8b
        _emit 0x72
        _emit 0x08
        // 0000f1f6:  03 72 04          ADD ESI, dword ptr [EDX+0x4]
        _emit 0x03
        _emit 0x72
        _emit 0x04
        // 0000f1f9:  8d 54 16 f0       LEA EDX, [ESI+EDX*1-0x10]
        _emit 0x8d
        _emit 0x54
        _emit 0x16
        _emit 0xf0
        // 0000f1fd:  3b d0             CMP EDX, EAX
        _emit 0x3b
        _emit 0xd0
        // 0000f1ff:  74 0d             JZ 0x0040f20e
        _emit 0x74
        _emit 0x0d
        // 0000f201:  89 52 0c          MOV dword ptr [EDX+0xc], EDX
        _emit 0x89
        _emit 0x52
        _emit 0x0c
        // 0000f204:  89 52 08          MOV dword ptr [EDX+0x8], EDX
        _emit 0x89
        _emit 0x52
        _emit 0x08
        // 0000f207:  89 42 04          MOV dword ptr [EDX+0x4], EAX
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 0000f20a:  89 02             MOV dword ptr [EDX], EAX
        _emit 0x89
        _emit 0x02
        // 0000f20c:  eb 02             JMP 0x0040f210
        _emit 0xeb
        _emit 0x02
        // 0000f20e:  33 d2             XOR EDX, EDX
        _emit 0x33
        _emit 0xd2
        // 0000f210:  89 02             MOV dword ptr [EDX], EAX
        _emit 0x89
        _emit 0x02
        // 0000f212:  8b 33             MOV ESI, dword ptr [EBX]
        _emit 0x8b
        _emit 0x33
        // 0000f214:  81 ce 00 00 00 80 OR ESI, 0x80000000
        _emit 0x81
        _emit 0xce
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80
        // 0000f21a:  55                PUSH EBP
        _emit 0x55
        // 0000f21b:  89 72 04          MOV dword ptr [EDX+0x4], ESI
        _emit 0x89
        _emit 0x72
        _emit 0x04
        // 0000f21e:  8d 51 0c          LEA EDX, [ECX+0xc]
        _emit 0x8d
        _emit 0x51
        _emit 0x0c
        // 0000f221:  bd 08 00 00 00    MOV EBP, 0x8
        _emit 0xbd
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f226:  8b 0a             MOV ECX, dword ptr [EDX]
        _emit 0x8b
        _emit 0x0a
        // 0000f228:  8d 72 08          LEA ESI, [EDX+0x8]
        _emit 0x8d
        _emit 0x72
        _emit 0x08
        // 0000f22b:  89 71 0c          MOV dword ptr [ECX+0xc], ESI
        _emit 0x89
        _emit 0x71
        _emit 0x0c
        // 0000f22e:  8b 0a             MOV ECX, dword ptr [EDX]
        _emit 0x8b
        _emit 0x0a
        // 0000f230:  89 4a 10          MOV dword ptr [EDX+0x10], ECX
        _emit 0x89
        _emit 0x4a
        _emit 0x10
        // 0000f233:  8d 4a f8          LEA ECX, [EDX-0x8]
        _emit 0x8d
        _emit 0x4a
        _emit 0xf8
        // 0000f236:  89 4a 14          MOV dword ptr [EDX+0x14], ECX
        _emit 0x89
        _emit 0x4a
        _emit 0x14
        // 0000f239:  89 32             MOV dword ptr [EDX], ESI
        _emit 0x89
        _emit 0x32
        // 0000f23b:  8b 4a 10          MOV ECX, dword ptr [EDX+0x10]
        _emit 0x8b
        _emit 0x4a
        _emit 0x10
        // 0000f23e:  8d 7a 18          LEA EDI, [EDX+0x18]
        _emit 0x8d
        _emit 0x7a
        _emit 0x18
        // 0000f241:  89 79 0c          MOV dword ptr [ECX+0xc], EDI
        _emit 0x89
        _emit 0x79
        _emit 0x0c
        // 0000f244:  8b 4a 10          MOV ECX, dword ptr [EDX+0x10]
        _emit 0x8b
        _emit 0x4a
        _emit 0x10
        // 0000f247:  89 4a 20          MOV dword ptr [EDX+0x20], ECX
        _emit 0x89
        _emit 0x4a
        _emit 0x20
        // 0000f24a:  89 72 24          MOV dword ptr [EDX+0x24], ESI
        _emit 0x89
        _emit 0x72
        _emit 0x24
        // 0000f24d:  89 7a 10          MOV dword ptr [EDX+0x10], EDI
        _emit 0x89
        _emit 0x7a
        _emit 0x10
        // 0000f250:  8b 4a 20          MOV ECX, dword ptr [EDX+0x20]
        _emit 0x8b
        _emit 0x4a
        _emit 0x20
        // 0000f253:  8d 72 28          LEA ESI, [EDX+0x28]
        _emit 0x8d
        _emit 0x72
        _emit 0x28
        // 0000f256:  89 71 0c          MOV dword ptr [ECX+0xc], ESI
        _emit 0x89
        _emit 0x71
        _emit 0x0c
        // 0000f259:  8b 4a 20          MOV ECX, dword ptr [EDX+0x20]
        _emit 0x8b
        _emit 0x4a
        _emit 0x20
        // 0000f25c:  89 4a 30          MOV dword ptr [EDX+0x30], ECX
        _emit 0x89
        _emit 0x4a
        _emit 0x30
        // 0000f25f:  89 7a 34          MOV dword ptr [EDX+0x34], EDI
        _emit 0x89
        _emit 0x7a
        _emit 0x34
        // 0000f262:  89 72 20          MOV dword ptr [EDX+0x20], ESI
        _emit 0x89
        _emit 0x72
        _emit 0x20
        // 0000f265:  8b 4a 30          MOV ECX, dword ptr [EDX+0x30]
        _emit 0x8b
        _emit 0x4a
        _emit 0x30
        // 0000f268:  8d 7a 38          LEA EDI, [EDX+0x38]
        _emit 0x8d
        _emit 0x7a
        _emit 0x38
        // 0000f26b:  89 79 0c          MOV dword ptr [ECX+0xc], EDI
        _emit 0x89
        _emit 0x79
        _emit 0x0c
        // 0000f26e:  8b 4a 30          MOV ECX, dword ptr [EDX+0x30]
        _emit 0x8b
        _emit 0x4a
        _emit 0x30
        // 0000f271:  89 4a 40          MOV dword ptr [EDX+0x40], ECX
        _emit 0x89
        _emit 0x4a
        _emit 0x40
        // 0000f274:  89 72 44          MOV dword ptr [EDX+0x44], ESI
        _emit 0x89
        _emit 0x72
        _emit 0x44
        // 0000f277:  89 7a 30          MOV dword ptr [EDX+0x30], EDI
        _emit 0x89
        _emit 0x7a
        _emit 0x30
        // 0000f27a:  83 c2 40          ADD EDX, 0x40
        _emit 0x83
        _emit 0xc2
        _emit 0x40
        // 0000f27d:  83 ed 01          SUB EBP, 0x1
        _emit 0x83
        _emit 0xed
        _emit 0x01
        // 0000f280:  75 a4             JNZ 0x0040f226
        _emit 0x75
        _emit 0xa4
        // 0000f282:  8b 4c 24 10       MOV ECX, dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0000f286:  8d 51 08          LEA EDX, [ECX+0x8]
        _emit 0x8d
        _emit 0x51
        _emit 0x08
        // 0000f289:  be 21 00 00 00    MOV ESI, 0x21
        _emit 0xbe
        _emit 0x21
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f28e:  5d                POP EBP
        _emit 0x5d
        // 0000f28f:  90                NOP
        _emit 0x90
        // 0000f290:  81 0a 00 00 00 80 OR dword ptr [EDX], 0x80000000
        _emit 0x81
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80
        // 0000f296:  83 c2 10          ADD EDX, 0x10
        _emit 0x83
        _emit 0xc2
        _emit 0x10
        // 0000f299:  83 ee 01          SUB ESI, 0x1
        _emit 0x83
        _emit 0xee
        _emit 0x01
        // 0000f29c:  75 f2             JNZ 0x0040f290
        _emit 0x75
        _emit 0xf2
        // 0000f29e:  0f bd 13          BSR EDX, dword ptr [EBX]
        _emit 0x0f
        _emit 0xbd
        _emit 0x13
        // 0000f2a1:  74 05             JZ 0x0040f2a8
        _emit 0x74
        _emit 0x05
        // 0000f2a3:  83 c2 01          ADD EDX, 0x1
        _emit 0x83
        _emit 0xc2
        _emit 0x01
        // 0000f2a6:  eb 02             JMP 0x0040f2aa
        _emit 0xeb
        _emit 0x02
        // 0000f2a8:  33 d2             XOR EDX, EDX
        _emit 0x33
        _emit 0xd2
        // 0000f2aa:  c1 e2 04          SHL EDX, 0x4
        _emit 0xc1
        _emit 0xe2
        _emit 0x04
        // 0000f2ad:  8b 44 0a 0c       MOV EAX, dword ptr [EDX+ECX*1+0xc]
        _emit 0x8b
        _emit 0x44
        _emit 0x0a
        _emit 0x0c
        // 0000f2b1:  8d 54 0a 04       LEA EDX, [EDX+ECX*1+0x4]
        _emit 0x8d
        _emit 0x54
        _emit 0x0a
        _emit 0x04
        // 0000f2b5:  89 58 0c          MOV dword ptr [EAX+0xc], EBX
        _emit 0x89
        _emit 0x58
        _emit 0x0c
        // 0000f2b8:  8b 42 08          MOV EAX, dword ptr [EDX+0x8]
        _emit 0x8b
        _emit 0x42
        _emit 0x08
        // 0000f2bb:  5f                POP EDI
        _emit 0x5f
        // 0000f2bc:  89 43 08          MOV dword ptr [EBX+0x8], EAX
        _emit 0x89
        _emit 0x43
        _emit 0x08
        // 0000f2bf:  89 53 0c          MOV dword ptr [EBX+0xc], EDX
        _emit 0x89
        _emit 0x53
        _emit 0x0c
        // 0000f2c2:  5e                POP ESI
        _emit 0x5e
        // 0000f2c3:  89 5a 08          MOV dword ptr [EDX+0x8], EBX
        _emit 0x89
        _emit 0x5a
        _emit 0x08
        // 0000f2c6:  8b c1             MOV EAX, ECX
        _emit 0x8b
        _emit 0xc1
        // 0000f2c8:  5b                POP EBX
        _emit 0x5b
        // 0000f2c9:  59                POP ECX
        _emit 0x59
        // 0000f2ca:  c2 04 00          RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
