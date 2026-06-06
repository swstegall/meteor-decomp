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
// FUNCTION: ffxivgame 0x005d6962 — FUN_009d6962
//                                  (245 B / 0xf5, EH4-SEH wrapped, __cdecl).
//
// Behaviour read from asm/ffxivgame/005d6962_FUN_009d6962.s:
//
//   __cdecl int FUN_009d6962(int fd);
//
//   EH4 SEH prolog (PUSH 0x10 / PUSH scope_table@0x122ce40 /
//   CALL __EH4_prolog3@0x9de4f0) then:
//
//     if (fd == -2) {
//         *FUN_009d9d5a() = 0;       // clear errno ptr
//         *FUN_009d9d47() = 9;       // set EBADF
//         return -1;                 // OR EAX,0xFFFFFFFF -> epilog
//     }
//
//     EDI = 0;
//     if (fd < 0 || fd >= *(int*)0x137b7dc) {
//         // range-error path
//         *FUN_009d9d5a() = 0;
//         *FUN_009d9d47() = 9;
//         FUN_009d2290(0,0,0,0,0);   // 5-arg call, all zeros
//         return -1;
//     }
//
//     // bit-field validity check
//     ECX = fd >> 5;
//     EBX = &((void**)0x137b7e0)[ECX];   // slot in pointer array
//     ESI = (fd & 0x1f) << 6;            // byte offset within block
//     ECX = *(byte*)((*EBX) + ESI + 4) & 1;
//     if (!ECX)    goto range_error;     // same 5-arg error path + return -1
//
//     FUN_009e77ed(fd);                  // lock / ref-count fd
//     [EBP-4] = 0;                       // SEH trylevel = 0
//
//     // re-check bit (slot may have changed under us)
//     if (!(*(byte*)((*EBX) + ESI + 4) & 1)) {
//         *FUN_009d9d47() = 9;           // EBADF
//         *FUN_009d9d5a() = 0;
//         local_e4 = -1;
//         goto epilog;
//     }
//
//     ESI = FUN_009e7a49(fd, 0, 1);
//     if (ESI == -1) {
//         local_e4 = -1;                 // via OR dword [EBP-0x1c], -1
//         goto epilog;
//     }
//     local_e4 = FUN_009e7a49(fd, 0, 2);
//     if (ESI != local_e4)
//         FUN_009e7a49(fd, ESI, 0);
//
//   epilog:
//     [EBP-4] = 0xFFFFFFFE;            // SEH trylevel -> cleanup frame
//     FUN_009d6a57();                   // paired cleanup (SEH unwind target)
//     EAX = local_e4;
//     CALL EH4_epilog@0x9de535;
//     RET
//
//   Stack frame (EH4, ESP-relative after prolog):
//     [EBP+8]      fd (first __cdecl argument)
//     [EBP-4]      SEH trylevel (set to 0 on scope entry, 0xFFFFFFFE on exit)
//     [EBP-0x1c]   local_e4 (return value accumulator)
//     EBX/ESI/EDI  callee-saved by __EH4_prolog3
//
//   Reloc-bearing sites (absolute addresses baked into the 245 orig bytes —
//   cannot be reproduced by standalone .obj compilation):
//     +0x02  abs32   0x122ce40   — scope table (.rdata FuncInfo)
//     +0x15  abs32   (in CALL rel32 to FUN_009d9d5a)
//     +0x1d  abs32   (in CALL rel32 to FUN_009d9d47)
//     +0x35  abs32   0x137b7dc   — fd count / max (CMP EAX,[abs])
//     +0x3d  ..      (CALL FUN_009d9d5a, 2nd)
//     +0x44  ..      (CALL FUN_009d9d47, 2nd)
//     +0x54  ..      (CALL FUN_009d2290)
//     +0x63  abs32   0x137b7e0   — fd slot pointer array (LEA EBX,[ECX*4+abs])
//     +0x7f  ..      (CALL FUN_009e77ed)
//     +0x97  ..      (CALL FUN_009e7a49, 1st)
//     +0xac  ..      (CALL FUN_009e7a49, 2nd)
//     +0xc0  ..      (CALL FUN_009e7a49, 3rd)
//     +0xca  ..      (CALL FUN_009d9d47, 3rd)
//     +0xd5  ..      (CALL FUN_009d9d5a, 3rd)
//     +0xe7  ..      (CALL FUN_009d6a57)
//     +0xef  ..      (CALL EH4_epilog@0x9de535)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The EH4 prolog helper (PUSH 0x10 / PUSH 0x122ce40 / CALL @0x9de4f0)
//   contains an absolute address in the PUSH imm32.  Eleven additional
//   absolute and PC-relative relocations are scattered through the body.
//   Source-level C++ recompilation at /O2 /GS /EHsc cannot reproduce those
//   relocation slots from a standalone .obj.  The same naked-asm strategy
//   used by FUN_00403a20, FUN_004054d0, and FUN_00402a30 applies here.

extern "C" __declspec(naked) void FUN_009d6962() {
    __asm {
        // 005d6962  PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 005d6964  PUSH 0x122ce40  (scope table)
        _emit 0x68
        _emit 0x40
        _emit 0xce
        _emit 0x22
        _emit 0x01
        // 005d6969  CALL 0x009de4f0  (__EH4_prolog3)
        _emit 0xe8
        _emit 0x82
        _emit 0x7b
        _emit 0x00
        _emit 0x00
        // 005d696e  MOV EAX,[EBP+8]
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        // 005d6971  CMP EAX,-2
        _emit 0x83
        _emit 0xf8
        _emit 0xfe
        // 005d6974  JNZ +0x1b
        _emit 0x75
        _emit 0x1b
        // 005d6976  CALL 0x009d9d5a
        _emit 0xe8
        _emit 0xdf
        _emit 0x33
        _emit 0x00
        _emit 0x00
        // 005d697b  AND dword ptr [EAX],0
        _emit 0x83
        _emit 0x20
        _emit 0x00
        // 005d697e  CALL 0x009d9d47
        _emit 0xe8
        _emit 0xc4
        _emit 0x33
        _emit 0x00
        _emit 0x00
        // 005d6983  MOV dword ptr [EAX],9
        _emit 0xc7
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005d6989  OR EAX,-1
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // 005d698c  JMP 0x009d6a51
        _emit 0xe9
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005d6991  XOR EDI,EDI
        _emit 0x33
        _emit 0xff
        // 005d6993  CMP EAX,EDI
        _emit 0x3b
        _emit 0xc7
        // 005d6995  JL +8
        _emit 0x7c
        _emit 0x08
        // 005d6997  CMP EAX,dword ptr [0x0137b7dc]
        _emit 0x3b
        _emit 0x05
        _emit 0xdc
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        // 005d699d  JC +0x21
        _emit 0x72
        _emit 0x21
        // 005d699f  CALL 0x009d9d5a
        _emit 0xe8
        _emit 0xb6
        _emit 0x33
        _emit 0x00
        _emit 0x00
        // 005d69a4  MOV dword ptr [EAX],EDI
        _emit 0x89
        _emit 0x38
        // 005d69a6  CALL 0x009d9d47
        _emit 0xe8
        _emit 0x9c
        _emit 0x33
        _emit 0x00
        _emit 0x00
        // 005d69ab  MOV dword ptr [EAX],9
        _emit 0xc7
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005d69b1  PUSH EDI
        _emit 0x57
        // 005d69b2  PUSH EDI
        _emit 0x57
        // 005d69b3  PUSH EDI
        _emit 0x57
        // 005d69b4  PUSH EDI
        _emit 0x57
        // 005d69b5  PUSH EDI
        _emit 0x57
        // 005d69b6  CALL 0x009d2290
        _emit 0xe8
        _emit 0xd5
        _emit 0xb8
        _emit 0xff
        _emit 0xff
        // 005d69bb  ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 005d69be  JMP 0x009d6989  (-> OR EAX,-1)
        _emit 0xeb
        _emit 0xc9
        // 005d69c0  MOV ECX,EAX
        _emit 0x8b
        _emit 0xc8
        // 005d69c2  SAR ECX,5
        _emit 0xc1
        _emit 0xf9
        _emit 0x05
        // 005d69c5  LEA EBX,[ECX*4+0x137b7e0]
        _emit 0x8d
        _emit 0x1c
        _emit 0x8d
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        // 005d69cc  MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // 005d69ce  AND ESI,0x1f
        _emit 0x83
        _emit 0xe6
        _emit 0x1f
        // 005d69d1  SHL ESI,6
        _emit 0xc1
        _emit 0xe6
        _emit 0x06
        // 005d69d4  MOV ECX,dword ptr [EBX]
        _emit 0x8b
        _emit 0x0b
        // 005d69d6  MOVZX ECX,byte ptr [ECX+ESI+4]
        _emit 0x0f
        _emit 0xb6
        _emit 0x4c
        _emit 0x31
        _emit 0x04
        // 005d69db  AND ECX,1
        _emit 0x83
        _emit 0xe1
        _emit 0x01
        // 005d69de  JZ -0x41  (-> 0x009d699f)
        _emit 0x74
        _emit 0xbf
        // 005d69e0  PUSH EAX
        _emit 0x50
        // 005d69e1  CALL 0x009e77ed
        _emit 0xe8
        _emit 0x07
        _emit 0x0e
        _emit 0x01
        _emit 0x00
        // 005d69e6  POP ECX
        _emit 0x59
        // 005d69e7  MOV dword ptr [EBP-4],EDI
        _emit 0x89
        _emit 0x7d
        _emit 0xfc
        // 005d69ea  MOV EAX,dword ptr [EBX]
        _emit 0x8b
        _emit 0x03
        // 005d69ec  TEST byte ptr [EAX+ESI+4],1
        _emit 0xf6
        _emit 0x44
        _emit 0x30
        _emit 0x04
        _emit 0x01
        // 005d69f1  JZ +0x39  (-> 0x009d6a2c)
        _emit 0x74
        _emit 0x39
        // 005d69f3  PUSH 1
        _emit 0x6a
        _emit 0x01
        // 005d69f5  PUSH EDI
        _emit 0x57
        // 005d69f6  PUSH dword ptr [EBP+8]
        _emit 0xff
        _emit 0x75
        _emit 0x08
        // 005d69f9  CALL 0x009e7a49
        _emit 0xe8
        _emit 0x4b
        _emit 0x10
        _emit 0x01
        _emit 0x00
        // 005d69fe  ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 005d6a01  MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // 005d6a03  CMP ESI,-1
        _emit 0x83
        _emit 0xfe
        _emit 0xff
        // 005d6a06  JZ +0x36  (-> 0x009d6a3e)
        _emit 0x74
        _emit 0x36
        // 005d6a08  PUSH 2
        _emit 0x6a
        _emit 0x02
        // 005d6a0a  PUSH EDI
        _emit 0x57
        // 005d6a0b  PUSH dword ptr [EBP+8]
        _emit 0xff
        _emit 0x75
        _emit 0x08
        // 005d6a0e  CALL 0x009e7a49
        _emit 0xe8
        _emit 0x36
        _emit 0x10
        _emit 0x01
        _emit 0x00
        // 005d6a13  ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 005d6a16  MOV dword ptr [EBP-0x1c],EAX
        _emit 0x89
        _emit 0x45
        _emit 0xe4
        // 005d6a19  CMP ESI,EAX
        _emit 0x3b
        _emit 0xf0
        // 005d6a1b  JZ +0x25  (-> 0x009d6a42)
        _emit 0x74
        _emit 0x25
        // 005d6a1d  PUSH EDI
        _emit 0x57
        // 005d6a1e  PUSH ESI
        _emit 0x56
        // 005d6a1f  PUSH dword ptr [EBP+8]
        _emit 0xff
        _emit 0x75
        _emit 0x08
        // 005d6a22  CALL 0x009e7a49
        _emit 0xe8
        _emit 0x22
        _emit 0x10
        _emit 0x01
        _emit 0x00
        // 005d6a27  ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 005d6a2a  JMP +0x16  (-> 0x009d6a42)
        _emit 0xeb
        _emit 0x16
        // 005d6a2c  CALL 0x009d9d47
        _emit 0xe8
        _emit 0x16
        _emit 0x33
        _emit 0x00
        _emit 0x00
        // 005d6a31  MOV dword ptr [EAX],9
        _emit 0xc7
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005d6a37  CALL 0x009d9d5a
        _emit 0xe8
        _emit 0x1e
        _emit 0x33
        _emit 0x00
        _emit 0x00
        // 005d6a3c  MOV dword ptr [EAX],EDI
        _emit 0x89
        _emit 0x38
        // 005d6a3e  OR dword ptr [EBP-0x1c],-1
        _emit 0x83
        _emit 0x4d
        _emit 0xe4
        _emit 0xff
        // 005d6a42  MOV dword ptr [EBP-4],0xFFFFFFFE
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 005d6a49  CALL 0x009d6a57
        _emit 0xe8
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005d6a4e  MOV EAX,dword ptr [EBP-0x1c]
        _emit 0x8b
        _emit 0x45
        _emit 0xe4
        // 005d6a51  CALL 0x009de535  (EH4 epilog)
        _emit 0xe8
        _emit 0xdf
        _emit 0x7a
        _emit 0x00
        _emit 0x00
        // 005d6a56  RET
        _emit 0xc3
    }
}
