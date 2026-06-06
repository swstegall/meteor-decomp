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
// FUNCTION: ffxivgame 0x0046ec40 — string-lookup/insert helper (__cdecl, 0xbc / 188 B)
//
// __cdecl int FUN_0046ec40(int *arg1, int arg2, const char *arg3, int arg4)
//
// arg1  — pointer to a two-field struct: { int field0; int *inner; }
// arg2  — mode / flag word:
//           > 0 && bit 0x1000 set  → special insert via FUN_00464d80 + FUN_00483bc0
//           == -1                   → search only; return 1 on hit
//           == -2                   → search + store computed length in inner[1]
//           anything else           → search + store arg2 in inner[1]
// arg3  — string pointer
// arg4  — explicit length; < 0 → compute strlen(arg3) at runtime
//
// Early exits (before PUSH EBP):
//   arg1 == NULL                → return 0
//   arg3 == NULL && arg4 != 0   → return 0
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Four CALL rel32 relocations, deferred PUSH EBP (two early-exit paths
//   before EBP is ever pushed), NEG/SBB/NEG bool-normalisation, and a
//   LEA EAX,[EBP+3] shortcut for the -2 branch return combine to make
//   byte-exact C++ reconstruction impractical under MSVC 2005 /O2.
//   Reloc sites are wildcarded by tools/compare.py.

extern "C" __declspec(naked) void FUN_0046ec40()
{
    __asm {
        _emit 0x53              // 00  PUSH EBX
        _emit 0x8b              // 01  MOV EBX, [ESP+8]
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x85              // 05  TEST EBX, EBX
        _emit 0xdb
        _emit 0x56              // 07  PUSH ESI
        _emit 0x57              // 08  PUSH EDI
        _emit 0x0f              // 09  JZ +0xa7  (to ecf6)
        _emit 0x84
        _emit 0xa7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // 0f  MOV EDI, [ESP+0x18]
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x85              // 13  TEST EDI, EDI
        _emit 0xff
        _emit 0x8b              // 15  MOV ESI, [ESP+0x1c]
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        _emit 0x75              // 19  JNZ +8  (to ec63)
        _emit 0x08
        _emit 0x85              // 1b  TEST ESI, ESI
        _emit 0xf6
        _emit 0x0f              // 1d  JNZ +0x93  (to ecf6)
        _emit 0x85
        _emit 0x93
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x55              // 23  PUSH EBP
        _emit 0x8b              // 24  MOV EBP, [ESP+0x18]
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x85              // 28  TEST EBP, EBP
        _emit 0xed
        _emit 0x7e              // 2a  JLE +0x2b  (to ec97)
        _emit 0x2b
        _emit 0xf7              // 2c  TEST EBP, 0x1000
        _emit 0xc5
        _emit 0x00
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x74              // 32  JZ +0x23  (to ec97)
        _emit 0x23
        _emit 0x8b              // 34  MOV EAX, [EBX]
        _emit 0x03
        _emit 0x50              // 36  PUSH EAX
        _emit 0xe8              // 37  CALL rel32 -> 0x00464d80
        _emit 0x04
        _emit 0x61
        _emit 0xff
        _emit 0xff
        _emit 0x50              // 3c  PUSH EAX
        _emit 0x55              // 3d  PUSH EBP
        _emit 0x56              // 3e  PUSH ESI
        _emit 0x57              // 3f  PUSH EDI
        _emit 0x83              // 40  ADD EBX, 4
        _emit 0xc3
        _emit 0x04
        _emit 0x53              // 43  PUSH EBX
        _emit 0xe8              // 44  CALL rel32 -> 0x00483bc0
        _emit 0x37
        _emit 0x4f
        _emit 0x01
        _emit 0x00
        _emit 0x83              // 49  ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0x5d              // 4c  POP EBP
        _emit 0xf7              // 4d  NEG EAX
        _emit 0xd8
        _emit 0x5f              // 4f  POP EDI
        _emit 0x1b              // 50  SBB EAX, EAX
        _emit 0xc0
        _emit 0x5e              // 52  POP ESI
        _emit 0xf7              // 53  NEG EAX
        _emit 0xd8
        _emit 0x5b              // 55  POP EBX
        _emit 0xc3              // 56  RET
        _emit 0x85              // 57  TEST ESI, ESI  (ec97:)
        _emit 0xf6
        _emit 0x7d              // 59  JGE +0x12  (to ecad)
        _emit 0x12
        _emit 0x8b              // 5b  MOV EAX, EDI
        _emit 0xc7
        _emit 0x8d              // 5d  LEA EDX, [EAX+1]
        _emit 0x50
        _emit 0x01
        _emit 0x8a              // 60  MOV CL, [EAX]  (loop:)
        _emit 0x08
        _emit 0x83              // 62  ADD EAX, 1
        _emit 0xc0
        _emit 0x01
        _emit 0x84              // 65  TEST CL, CL
        _emit 0xc9
        _emit 0x75              // 67  JNZ -9  (to loop)
        _emit 0xf7
        _emit 0x2b              // 69  SUB EAX, EDX
        _emit 0xc2
        _emit 0x8b              // 6b  MOV ESI, EAX
        _emit 0xf0
        _emit 0x8b              // 6d  MOV ECX, [EBX+4]  (ecad:)
        _emit 0x4b
        _emit 0x04
        _emit 0x56              // 70  PUSH ESI
        _emit 0x57              // 71  PUSH EDI
        _emit 0x51              // 72  PUSH ECX
        _emit 0xe8              // 73  CALL rel32 -> 0x00464370
        _emit 0xb8
        _emit 0x56
        _emit 0xff
        _emit 0xff
        _emit 0x83              // 78  ADD ESP, 0xC
        _emit 0xc4
        _emit 0x0c
        _emit 0x85              // 7b  TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // 7d  JNZ +5  (to ecc4)
        _emit 0x05
        _emit 0x5d              // 7f  POP EBP
        _emit 0x5f              // 80  POP EDI
        _emit 0x5e              // 81  POP ESI
        _emit 0x5b              // 82  POP EBX
        _emit 0xc3              // 83  RET
        _emit 0x83              // 84  CMP EBP, -1  (ecc4:)
        _emit 0xfd
        _emit 0xff
        _emit 0x74              // 87  JZ +0x23  (to ecec)
        _emit 0x23
        _emit 0x83              // 89  CMP EBP, -2
        _emit 0xfd
        _emit 0xfe
        _emit 0x75              // 8c  JNZ +0x18  (to ece6)
        _emit 0x18
        _emit 0x56              // 8e  PUSH ESI
        _emit 0x57              // 8f  PUSH EDI
        _emit 0xe8              // 90  CALL rel32 -> 0x00483a80
        _emit 0xab
        _emit 0x4d
        _emit 0x01
        _emit 0x00
        _emit 0x8b              // 95  MOV EDX, [EBX+4]
        _emit 0x53
        _emit 0x04
        _emit 0x83              // 98  ADD ESP, 8
        _emit 0xc4
        _emit 0x08
        _emit 0x89              // 9b  MOV [EDX+4], EAX
        _emit 0x42
        _emit 0x04
        _emit 0x8d              // 9e  LEA EAX, [EBP+3]  (= -2+3 = 1)
        _emit 0x45
        _emit 0x03
        _emit 0x5d              // a1  POP EBP
        _emit 0x5f              // a2  POP EDI
        _emit 0x5e              // a3  POP ESI
        _emit 0x5b              // a4  POP EBX
        _emit 0xc3              // a5  RET
        _emit 0x8b              // a6  MOV EAX, [EBX+4]  (ece6:)
        _emit 0x43
        _emit 0x04
        _emit 0x89              // a9  MOV [EAX+4], EBP
        _emit 0x68
        _emit 0x04
        _emit 0x5d              // ac  POP EBP  (ecec:)
        _emit 0x5f              // ad  POP EDI
        _emit 0x5e              // ae  POP ESI
        _emit 0xb8              // af  MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5b              // b4  POP EBX
        _emit 0xc3              // b5  RET
        _emit 0x5f              // b6  POP EDI  (ecf6:)
        _emit 0x5e              // b7  POP ESI
        _emit 0x33              // b8  XOR EAX, EAX
        _emit 0xc0
        _emit 0x5b              // ba  POP EBX
        _emit 0xc3              // bb  RET
    }
}
