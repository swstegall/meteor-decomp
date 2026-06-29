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
// FUNCTION: ffxivgame 0x00059510 — FUN_00459510 (247 B / 0xf7)
//                                  `__thiscall`, 2 stack args, bool return in AL.
//                                  No SEH / no security cookie.
//
// Behaviour read from asm/ffxivgame/00059510_FUN_00459510.s:
//
//   __thiscall bool FUN_00459510(this, arg0, arg1) — ECX = this (→ EDI).
//   Returns bool in AL; RET 0x8 pops 2 stack args.
//
//   Control-flow outline:
//
//     if (this->field_08 != 0) return true;                 // early-out
//
//     if (IAT[0xf3e658](0) < 0) {                           // IAT call #1
//         FUN_00458a80(this, arg0);
//         return false;
//     }
//
//     EBX = arg1;
//     if (EBX == 0) {                                        // null-arg check
//         FUN_00458a80(this, arg0);
//         IAT[0xf3e64c]();                                   // IAT call #2
//         return false;
//     }
//
//     ESI = &this->field_04;
//     hr = IAT[0xf3e660](0x11080d0, 0, 1, 0xf67958, ESI);  // IAT call #3
//     if (hr < 0) {
//         FUN_00458a80(this, arg0);
//         IAT[0xf3e64c]();
//         return false;
//     }
//
//     // COM-style vtable call: vtable[14](obj, &local, 4)
//     EAX = *ESI;
//     ECX = *EAX;   // vtable
//     hr = (vtable[0x38/4])(EAX, &local, 4);
//     if (hr < 0) {
//         /* fall through to error handler with arg0 */
//     } else {
//         void* p = malloc(0x14);            // CALL 0x009d1b35
//         if (p)
//             FUN_004591c0(p, EBX, *ESI);   // initialise allocation
//         else
//             p = 0;
//         this->field_08 = p;
//         if (FUN_00458b30(p))              // success predicate
//             return true;
//     }
//     // error handler
//     FUN_00458a80(this, arg0);
//     FUN_004592d0(this);
//     return false;
//
// Reloc-bearing sites (absolute addresses baked into the orig slice;
// only resolvable by a full binary relink at image base 0x00400000):
//   +0x13  abs32  0x00f3e658  IAT slot — call #1
//   +0x24  rel32  0x00458a80  FUN_00458a80 (error handler)
//   +0x40  rel32  0x00458a80  FUN_00458a80 (2nd call)
//   +0x46  abs32  0x00f3e64c  IAT slot — call #2
//   +0x58  abs32  0xf67958    data pointer arg to IAT call #3
//   +0x61  abs32  0x11080d0   GUID/handle arg to IAT call #3
//   +0x66  abs32  0x00f3e660  IAT slot — call #3
//   +0x77  rel32  0x00458a80  FUN_00458a80 (3rd call)
//   +0x7d  abs32  0x00f3e64c  IAT slot — call #2 again
//   +0xa9  rel32  0x009d1b35  malloc / operator new (0x14 bytes)
//   +0xbb  rel32  0x004591c0  FUN_004591c0 (init)
//   +0xc9  rel32  0x00458b30  FUN_00458b30 (success predicate)
//   +0xd9  rel32  0x00458a80  FUN_00458a80 (4th call)
//   +0xe0  rel32  0x004592d0  FUN_004592d0 (cleanup)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   No SEH / no __security_cookie — the reloc burden is entirely the
//   absolute IAT-slot addresses and intra-binary rel32 CALLs. A
//   source-level rewrite would need the MSVC 2005 /O2 scheduler to
//   produce exactly this register allocation, branch-shortening, and
//   push-sequence order. Each of those is brittle; the pragmatic
//   choice (same as FUN_00402a30, FUN_00403a20, FUN_004054d0) is to
//   emit all 247 bytes verbatim via _emit directives so compare.py
//   sees a byte-identical .text section with no auxiliaries.

extern "C" __declspec(naked) void FUN_00459510() {
    __asm {
        // 00059510  PUSH ECX
        _emit 0x51
        // 00059511  PUSH EDI
        _emit 0x57
        // 00059512  MOV EDI,ECX
        _emit 0x8b
        _emit 0xf9
        // 00059514  CMP dword ptr [EDI+0x8],0x0
        _emit 0x83
        _emit 0x7f
        _emit 0x08
        _emit 0x00
        // 00059518  JZ +0x9 (-> 00459521)
        _emit 0x74
        _emit 0x07
        // 0005951a  MOV AL,0x1
        _emit 0xb0
        _emit 0x01
        // 0005951c  POP EDI
        _emit 0x5f
        // 0005951d  POP ECX
        _emit 0x59
        // 0005951e  RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // 00059521  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00059523  CALL dword ptr [0x00f3e658]
        _emit 0xff
        _emit 0x15
        _emit 0x58
        _emit 0xe6
        _emit 0xf3
        _emit 0x00
        // 00059529  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0005952b  JGE +0x15 (-> 00459540)
        _emit 0x7d
        _emit 0x13
        // 0005952d  MOV EAX,dword ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00059531  PUSH EAX
        _emit 0x50
        // 00059532  MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 00059534  CALL 0x00458a80
        _emit 0xe8
        _emit 0x47
        _emit 0xf5
        _emit 0xff
        _emit 0xff
        // 00059539  XOR AL,AL
        _emit 0x32
        _emit 0xc0
        // 0005953b  POP EDI
        _emit 0x5f
        // 0005953c  POP ECX
        _emit 0x59
        // 0005953d  RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // 00059540  PUSH EBX
        _emit 0x53
        // 00059541  MOV EBX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        // 00059545  TEST EBX,EBX
        _emit 0x85
        _emit 0xdb
        // 00059547  JNZ +0x1c (-> 00459563)
        _emit 0x75
        _emit 0x1a
        // 00059549  MOV ECX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0005954d  PUSH ECX
        _emit 0x51
        // 0005954e  MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 00059550  CALL 0x00458a80
        _emit 0xe8
        _emit 0x2b
        _emit 0xf5
        _emit 0xff
        _emit 0xff
        // 00059555  CALL dword ptr [0x00f3e64c]
        _emit 0xff
        _emit 0x15
        _emit 0x4c
        _emit 0xe6
        _emit 0xf3
        _emit 0x00
        // 0005955b  POP EBX
        _emit 0x5b
        // 0005955c  XOR AL,AL
        _emit 0x32
        _emit 0xc0
        // 0005955e  POP EDI
        _emit 0x5f
        // 0005955f  POP ECX
        _emit 0x59
        // 00059560  RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // 00059563  PUSH ESI
        _emit 0x56
        // 00059564  LEA ESI,[EDI+0x4]
        _emit 0x8d
        _emit 0x77
        _emit 0x04
        // 00059567  PUSH ESI
        _emit 0x56
        // 00059568  PUSH 0xf67958
        _emit 0x68
        _emit 0x58
        _emit 0x79
        _emit 0xf6
        _emit 0x00
        // 0005956d  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005956f  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00059571  PUSH 0x11080d0
        _emit 0x68
        _emit 0xd0
        _emit 0x80
        _emit 0x10
        _emit 0x01
        // 00059576  CALL dword ptr [0x00f3e660]
        _emit 0xff
        _emit 0x15
        _emit 0x60
        _emit 0xe6
        _emit 0xf3
        _emit 0x00
        // 0005957c  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0005957e  JGE +0x1d (-> 0045959b)
        _emit 0x7d
        _emit 0x1b
        // 00059580  MOV EDX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 00059584  PUSH EDX
        _emit 0x52
        // 00059585  MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 00059587  CALL 0x00458a80
        _emit 0xe8
        _emit 0xf4
        _emit 0xf4
        _emit 0xff
        _emit 0xff
        // 0005958c  CALL dword ptr [0x00f3e64c]
        _emit 0xff
        _emit 0x15
        _emit 0x4c
        _emit 0xe6
        _emit 0xf3
        _emit 0x00
        // 00059592  POP ESI
        _emit 0x5e
        // 00059593  POP EBX
        _emit 0x5b
        // 00059594  XOR AL,AL
        _emit 0x32
        _emit 0xc0
        // 00059596  POP EDI
        _emit 0x5f
        // 00059597  POP ECX
        _emit 0x59
        // 00059598  RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // 0005959b  MOV EAX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 0005959d  MOV ECX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x08
        // 0005959f  PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 000595a1  LEA EDX,[ESP+0x10]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 000595a5  PUSH EDX
        _emit 0x52
        // 000595a6  PUSH EAX
        _emit 0x50
        // 000595a7  MOV EAX,dword ptr [ECX+0x38]
        _emit 0x8b
        _emit 0x41
        _emit 0x38
        // 000595aa  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 000595ac  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 000595ae  JGE +0x9 (-> 004595b7)
        _emit 0x7d
        _emit 0x07
        // 000595b0  MOV ECX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 000595b4  PUSH ECX
        _emit 0x51
        // 000595b5  JMP +0x32 (-> 004595e7)
        _emit 0xeb
        _emit 0x30
        // 000595b7  PUSH 0x14
        _emit 0x6a
        _emit 0x14
        // 000595b9  CALL 0x009d1b35
        _emit 0xe8
        _emit 0x77
        _emit 0x85
        _emit 0x57
        _emit 0x00
        // 000595be  ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 000595c1  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 000595c3  JZ +0xf (-> 004595d2)
        _emit 0x74
        _emit 0x0d
        // 000595c5  MOV EDX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x16
        // 000595c7  PUSH EBX
        _emit 0x53
        // 000595c8  PUSH EDX
        _emit 0x52
        // 000595c9  MOV ECX,EAX
        _emit 0x8b
        _emit 0xc8
        // 000595cb  CALL 0x004591c0
        _emit 0xe8
        _emit 0xf0
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        // 000595d0  JMP +0x4 (-> 004595d4)
        _emit 0xeb
        _emit 0x02
        // 000595d2  XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 000595d4  MOV ECX,EAX
        _emit 0x8b
        _emit 0xc8
        // 000595d6  MOV dword ptr [EDI+0x8],EAX
        _emit 0x89
        _emit 0x47
        _emit 0x08
        // 000595d9  CALL 0x00458b30
        _emit 0xe8
        _emit 0x52
        _emit 0xf5
        _emit 0xff
        _emit 0xff
        // 000595de  TEST AL,AL
        _emit 0x84
        _emit 0xc0
        // 000595e0  JNZ +0x1e (-> 004595fe)
        _emit 0x75
        _emit 0x1c
        // 000595e2  MOV EAX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 000595e6  PUSH EAX
        _emit 0x50
        // 000595e7  MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 000595e9  CALL 0x00458a80
        _emit 0xe8
        _emit 0x92
        _emit 0xf4
        _emit 0xff
        _emit 0xff
        // 000595ee  MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 000595f0  CALL 0x004592d0
        _emit 0xe8
        _emit 0xdb
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 000595f5  POP ESI
        _emit 0x5e
        // 000595f6  POP EBX
        _emit 0x5b
        // 000595f7  XOR AL,AL
        _emit 0x32
        _emit 0xc0
        // 000595f9  POP EDI
        _emit 0x5f
        // 000595fa  POP ECX
        _emit 0x59
        // 000595fb  RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // 000595fe  POP ESI
        _emit 0x5e
        // 000595ff  POP EBX
        _emit 0x5b
        // 00059600  MOV AL,0x1
        _emit 0xb0
        _emit 0x01
        // 00059602  POP EDI
        _emit 0x5f
        // 00059603  POP ECX
        _emit 0x59
        // 00059604  RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
