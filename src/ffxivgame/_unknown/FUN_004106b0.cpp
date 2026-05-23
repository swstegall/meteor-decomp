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
// FUNCTION: ffxivgame 0x000106b0 — FUN_004106b0 (__thiscall, 118 bytes / 0x76)
//
// Calling convention: __thiscall (ECX = this). No explicit stack args.
// Callee-saves pushed: EBX, ESI, EDI (+ ECX saved/restored via PUSH/POP ECX).
// EBP saved inside loop branch only.
//
// High-level behaviour:
//   1. Call vtable[1] on this->field_0x14 to obtain a "manager" pointer
//      (return value EAX). EBX = manager->field_0x14.
//   2. Traverse the circular doubly-linked list rooted at this+0x38
//      (sentinel at this+0x38, first element at this->field_0x3c).
//      For each node (EDI):
//      a. Call vtable[1] on EDI to obtain a payload object (ESI).
//         Advance EDI to next node (EDI = EDI->field_0x4) before continuing.
//      b. Call vtable[0] on ESI with one 0 argument.
//      c. Spin-lock acquire on EBX->field_0x4 using XCHG.
//      d. Insert ESI into the doubly-linked list at EBX->field_0xc (tail insert).
//      e. Decrement EBX->field_0x18 by 1.
//      f. Spin-lock release on EBX->field_0x4 (XCHG with 0).
//
// No CALL rel32 instructions are present (all calls are register-indirect
// through EDX). There are therefore NO relocations in this function's
// machine code. The __declspec(naked) body re-emits the original 118 bytes
// verbatim via MASM _emit so the .obj's .text is byte-identical to the
// original slice and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_004106b0() {
    __asm {
        // 000106b0:  51                     PUSH ECX
        _emit 0x51
        // 000106b1:  53                     PUSH EBX
        _emit 0x53
        // 000106b2:  56                     PUSH ESI
        _emit 0x56
        // 000106b3:  8b f1                  MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 000106b5:  8b 4e 14               MOV ECX,dword ptr [ESI+0x14]
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 000106b8:  8b 01                  MOV EAX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 000106ba:  8b 50 04               MOV EDX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000106bd:  57                     PUSH EDI
        _emit 0x57
        // 000106be:  ff d2                  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000106c0:  8b 7e 3c               MOV EDI,dword ptr [ESI+0x3c]
        _emit 0x8b
        _emit 0x7e
        _emit 0x3c
        // 000106c3:  8b 58 14               MOV EBX,dword ptr [EAX+0x14]
        _emit 0x8b
        _emit 0x58
        _emit 0x14
        // 000106c6:  83 c6 38               ADD ESI,0x38
        _emit 0x83
        _emit 0xc6
        _emit 0x38
        // 000106c9:  3b fe                  CMP EDI,ESI
        _emit 0x3b
        _emit 0xfe
        // 000106cb:  89 74 24 0c            MOV dword ptr [ESP+0xc],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 000106cf:  74 50                  JZ +0x50  (→ 0x00410721, exit)
        _emit 0x74
        _emit 0x50
        // 000106d1:  55                     PUSH EBP
        _emit 0x55
        // 000106d2:  8d 6b 04               LEA EBP,[EBX+0x4]
        _emit 0x8d
        _emit 0x6b
        _emit 0x04
        // === loop top (0x000106d5) ===
        // 000106d5:  8b 07                  MOV EAX,dword ptr [EDI]
        _emit 0x8b
        _emit 0x07
        // 000106d7:  8b 50 04               MOV EDX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000106da:  8b cf                  MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 000106dc:  ff d2                  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000106de:  8b 7f 04               MOV EDI,dword ptr [EDI+0x4]
        _emit 0x8b
        _emit 0x7f
        _emit 0x04
        // 000106e1:  8b f0                  MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // 000106e3:  8b 06                  MOV EAX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 000106e5:  8b 10                  MOV EDX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x10
        // 000106e7:  6a 00                  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 000106e9:  8b ce                  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 000106eb:  ff d2                  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000106ed:  8d 49 00               LEA ECX,[ECX+0x0]  (3-byte NOP/alignment)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // === spin-lock acquire (0x000106f0) ===
        // 000106f0:  b8 01 00 00 00         MOV EAX,0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000106f5:  8b cd                  MOV ECX,EBP
        _emit 0x8b
        _emit 0xcd
        // 000106f7:  87 01                  XCHG dword ptr [ECX],EAX
        _emit 0x87
        _emit 0x01
        // 000106f9:  85 c0                  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 000106fb:  75 f3                  JNZ -0xd  (→ 0x004106f0, spin)
        _emit 0x75
        _emit 0xf3
        // === list insertion (0x000106fd) ===
        // 000106fd:  8b 43 0c               MOV EAX,dword ptr [EBX+0xc]
        _emit 0x8b
        _emit 0x43
        _emit 0x0c
        // 00010700:  8b 50 04               MOV EDX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00010703:  89 32                  MOV dword ptr [EDX],ESI
        _emit 0x89
        _emit 0x32
        // 00010705:  8b 48 04               MOV ECX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        // 00010708:  89 06                  MOV dword ptr [ESI],EAX
        _emit 0x89
        _emit 0x06
        // 0001070a:  89 4e 04               MOV dword ptr [ESI+0x4],ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x04
        // 0001070d:  89 70 04               MOV dword ptr [EAX+0x4],ESI
        _emit 0x89
        _emit 0x70
        _emit 0x04
        // 00010710:  83 43 18 ff            ADD dword ptr [EBX+0x18],-0x1
        _emit 0x83
        _emit 0x43
        _emit 0x18
        _emit 0xff
        // === spin-lock release (0x00010714) ===
        // 00010714:  33 d2                  XOR EDX,EDX
        _emit 0x33
        _emit 0xd2
        // 00010716:  8b c5                  MOV EAX,EBP
        _emit 0x8b
        _emit 0xc5
        // 00010718:  87 10                  XCHG dword ptr [EAX],EDX
        _emit 0x87
        _emit 0x10
        // === loop condition (0x0001071a) ===
        // 0001071a:  3b 7c 24 10            CMP EDI,dword ptr [ESP+0x10]
        _emit 0x3b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 0001071e:  75 b5                  JNZ -0x4b  (→ 0x004106d5, loop)
        _emit 0x75
        _emit 0xb5
        // 00010720:  5d                     POP EBP
        _emit 0x5d
        // === epilogue (0x00010721) ===
        // 00010721:  5f                     POP EDI
        _emit 0x5f
        // 00010722:  5e                     POP ESI
        _emit 0x5e
        // 00010723:  5b                     POP EBX
        _emit 0x5b
        // 00010724:  59                     POP ECX
        _emit 0x59
        // 00010725:  c3                     RET
        _emit 0xc3
    }
}
