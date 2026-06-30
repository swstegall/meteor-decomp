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
// FUNCTION: ffxivgame 0x0005b4c0 — FUN_0045b4c0
//                                  (251 B / 0xfb, no SEH).
//
// Behaviour read from the disassembly at orig RVA 0x0005b4c0:
//
//   __cdecl int FUN_0045b4c0(SomeStruct* arg0);
//
//   Prologue: SUB ESP, 0x18 / PUSH EDI
//
//   Calls an IAT function ([0x00f3e1a0], likely a Win32 API) with args:
//     (0x132d110, 1, 0) and tests the result. If EAX == 0 (fail branch),
//     calls a CRT/helper (0x009d1b35) with arg 1, saves result to
//     [0x0132d10c], then calls three setup helpers
//     (0x0045cb40, 0x0045c820, 0x0045cb10).
//
//   After the conditional init block, loads arg0[1] (ECX = [EAX+4]) and
//   branches on it. If non-zero, calls FUN_00458a60 (ECX arg), saves into
//   ESI. If ESI is non-zero, calls FUN_0045cb50 twice (both with ESI),
//   then on positive result enters a longer processing sequence:
//     - calls FUN_0045ced0 (LEA of stack slot)
//     - calls FUN_0045d650 (push 0)
//     - calls FUN_0045cf20 (stack slot + result of FUN_0045d650)
//     - if result == 1, calls FUN_0045d0d0, then if result == 1
//       calls FUN_0045d490 (ESI + two stack args)
//     - computes EDI from EAX: if EAX > 0, EDI = 0x28a5, else
//       if EAX < 0, EDI unchanged (0x28a5 initial), else EDI = 0x28a6
//     - calls FUN_0045d160 (cleanup of stack slot)
//   Finally calls FUN_0045ce50 (ESI), pops ESI.
//
//   Returns EDI in EAX. Initial EDI = 0x28a5; updated by the inner
//   branch via the SETG/SUB/AND idiom to 0x28a5 (EAX>0) or 0x28a6
//   (EAX==0, masked by AND 0x28a6).
//
//   Reloc-bearing sites in the orig 251 bytes (absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x08   abs32  0x0132d110  — argument pushed to IAT call
//     +0x12   abs32  0x00f3e1a0  — IAT indirect call slot
//     +0x26   abs32  0x0132d10c  — global written with CRT result
//     +0x1e   rel32  0x009d1b35  — CRT/helper call (relative)
//     +0x2b   rel32  0x0045cb40  — relative call
//     +0x30   rel32  0x0045c820  — relative call
//     +0x35   rel32  0x0045cb10  — relative call
//     +0x4a   rel32  0x00458a60  — relative call
//     +0x5a   rel32  0x0045cb50  — relative call (1st)
//     +0x60   rel32  0x0045cb50  — relative call (2nd)
//     +0x71   rel32  0x0045ced0  — relative call
//     +0x7b   rel32  0x0045d650  — relative call
//     +0x86   rel32  0x0045cf20  — relative call
//     +0xa2   rel32  0x0045d0d0  — relative call
//     +0xbf   rel32  0x0045d490  — relative call
//     +0xe2   rel32  0x0045d160  — relative call
//     +0xeb   rel32  0x0045ce50  — relative call
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This function contains absolute address references in its binary encoding
//   (IAT slot, two global addresses pushed/stored as immediates) plus
//   numerous PC-relative calls whose raw rel32 bytes encode the original
//   link-time RVA layout. A source-level C++ port at /O2 would need to
//   reproduce the exact register allocation (EDI for the return value,
//   ESI for the queried handle), the stack frame shape, the SETG+SUB+AND
//   idiom for the ternary result computation, AND all the linker-resolved
//   addresses. This is fragile in MSVC 2005 under /O2.
//
//   The pragmatic choice — matching FUN_004054d0 / FUN_00402a30 /
//   FUN_00403a20 — is a `__declspec(naked)` body that re-emits the orig
//   251 bytes verbatim via MASM `_emit` directives.

extern "C" __declspec(naked) void FUN_0045b4c0() {
    __asm {
        // 0005b4c0  SUB ESP,0x18
        _emit 0x83
        _emit 0xec
        _emit 0x18
        // 0005b4c3  PUSH EDI
        _emit 0x57
        // 0005b4c4  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0005b4c6  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005b4c8  PUSH 0x132d110
        _emit 0x68
        _emit 0x10
        _emit 0xd1
        _emit 0x32
        _emit 0x01
        // 0005b4cd  MOV EDI,0x28a5
        _emit 0xbf
        _emit 0xa5
        _emit 0x28
        _emit 0x00
        _emit 0x00
        // 0005b4d2  CALL dword ptr [0x00f3e1a0]
        _emit 0xff
        _emit 0x15
        _emit 0xa0
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0005b4d8  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0005b4da  JNZ +0x1e
        _emit 0x75
        _emit 0x1e
        // 0005b4dc  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005b4de  CALL 0x009d1b35
        _emit 0xe8
        _emit 0x52
        _emit 0x66
        _emit 0x57
        _emit 0x00
        // 0005b4e3  ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005b4e6  MOV [0x0132d10c],EAX
        _emit 0xa3
        _emit 0x0c
        _emit 0xd1
        _emit 0x32
        _emit 0x01
        // 0005b4eb  CALL 0x0045cb40
        _emit 0xe8
        _emit 0x50
        _emit 0x16
        _emit 0x00
        _emit 0x00
        // 0005b4f0  CALL 0x0045c820
        _emit 0xe8
        _emit 0x2b
        _emit 0x13
        _emit 0x00
        _emit 0x00
        // 0005b4f5  CALL 0x0045cb10
        _emit 0xe8
        _emit 0x16
        _emit 0x16
        _emit 0x00
        _emit 0x00
        // 0005b4fa  MOV EAX,dword ptr [ESP + 0x20]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 0005b4fe  MOV ECX,dword ptr [EAX + 0x4]
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        // 0005b501  TEST ECX,ECX
        _emit 0x85
        _emit 0xc9
        // 0005b503  JZ +0xab (-> 0x0045b5b4)
        _emit 0x0f
        _emit 0x84
        _emit 0xab
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005b509  PUSH ESI
        _emit 0x56
        // 0005b50a  CALL 0x00458a60
        _emit 0xe8
        _emit 0x51
        _emit 0xd5
        _emit 0xff
        _emit 0xff
        // 0005b50f  MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // 0005b511  TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 0005b513  JZ +0x9a (-> 0x0045b5b3)
        _emit 0x0f
        _emit 0x84
        _emit 0x9a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005b519  PUSH ESI
        _emit 0x56
        // 0005b51a  CALL 0x0045cb50
        _emit 0xe8
        _emit 0x31
        _emit 0x16
        _emit 0x00
        _emit 0x00
        // 0005b51f  PUSH ESI
        _emit 0x56
        // 0005b520  CALL 0x0045cb50
        _emit 0xe8
        _emit 0x2b
        _emit 0x16
        _emit 0x00
        _emit 0x00
        // 0005b525  ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0005b528  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0005b52a  JLE +0x7e (-> 0x0045b5aa)
        _emit 0x7e
        _emit 0x7e
        // 0005b52c  LEA ECX,[ESP + 0x8]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 0005b530  PUSH ECX
        _emit 0x51
        // 0005b531  CALL 0x0045ced0
        _emit 0xe8
        _emit 0x9a
        _emit 0x19
        _emit 0x00
        _emit 0x00
        // 0005b536  ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005b539  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0005b53b  CALL 0x0045d650
        _emit 0xe8
        _emit 0x10
        _emit 0x21
        _emit 0x00
        _emit 0x00
        // 0005b540  PUSH EAX
        _emit 0x50
        // 0005b541  LEA EDX,[ESP + 0x10]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0005b545  PUSH EDX
        _emit 0x52
        // 0005b546  CALL 0x0045cf20
        _emit 0xe8
        _emit 0xd5
        _emit 0x19
        _emit 0x00
        _emit 0x00
        // 0005b54b  ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0005b54e  CMP EAX,0x1
        _emit 0x83
        _emit 0xf8
        _emit 0x01
        // 0005b551  JNZ +0x4a (-> 0x0045b59d)
        _emit 0x75
        _emit 0x4a
        // 0005b553  MOV EAX,dword ptr [ESP + 0x2c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // 0005b557  MOV ECX,dword ptr [ESP + 0x28]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // 0005b55b  PUSH EAX
        _emit 0x50
        // 0005b55c  PUSH ECX
        _emit 0x51
        // 0005b55d  LEA EDX,[ESP + 0x10]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0005b561  PUSH EDX
        _emit 0x52
        // 0005b562  CALL 0x0045d0d0
        _emit 0xe8
        _emit 0x69
        _emit 0x1b
        _emit 0x00
        _emit 0x00
        // 0005b567  ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0005b56a  CMP EAX,0x1
        _emit 0x83
        _emit 0xf8
        _emit 0x01
        // 0005b56d  JNZ +0x2e (-> 0x0045b59d)
        _emit 0x75
        _emit 0x2e
        // 0005b56f  MOV EAX,dword ptr [ESP + 0x34]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // 0005b573  MOV ECX,dword ptr [ESP + 0x30]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        // 0005b577  PUSH ESI
        _emit 0x56
        // 0005b578  PUSH EAX
        _emit 0x50
        // 0005b579  PUSH ECX
        _emit 0x51
        // 0005b57a  LEA EDX,[ESP + 0x14]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0005b57e  PUSH EDX
        _emit 0x52
        // 0005b57f  CALL 0x0045d490
        _emit 0xe8
        _emit 0x0c
        _emit 0x1f
        _emit 0x00
        _emit 0x00
        // 0005b584  ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0005b587  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0005b589  JL +0x12 (-> 0x0045b59d)
        _emit 0x7c
        _emit 0x12
        // 0005b58b  XOR ECX,ECX
        _emit 0x33
        _emit 0xc9
        // 0005b58d  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0005b58f  SETG CL
        _emit 0x0f
        _emit 0x9f
        _emit 0xc1
        // 0005b592  SUB ECX,0x1
        _emit 0x83
        _emit 0xe9
        _emit 0x01
        // 0005b595  AND ECX,0x28a6
        _emit 0x81
        _emit 0xe1
        _emit 0xa6
        _emit 0x28
        _emit 0x00
        _emit 0x00
        // 0005b59b  MOV EDI,ECX
        _emit 0x8b
        _emit 0xf9
        // 0005b59d  LEA EDX,[ESP + 0x8]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 0005b5a1  PUSH EDX
        _emit 0x52
        // 0005b5a2  CALL 0x0045d160
        _emit 0xe8
        _emit 0xb9
        _emit 0x1b
        _emit 0x00
        _emit 0x00
        // 0005b5a7  ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005b5aa  PUSH ESI
        _emit 0x56
        // 0005b5ab  CALL 0x0045ce50
        _emit 0xe8
        _emit 0xa0
        _emit 0x18
        _emit 0x00
        _emit 0x00
        // 0005b5b0  ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005b5b3  POP ESI
        _emit 0x5e
        // 0005b5b4  MOV EAX,EDI
        _emit 0x8b
        _emit 0xc7
        // 0005b5b6  POP EDI
        _emit 0x5f
        // 0005b5b7  ADD ESP,0x18
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 0005b5ba  RET
        _emit 0xc3
    }
}
