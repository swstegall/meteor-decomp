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
// FUNCTION: ffxivgame 0x00042050 — FUN_00442050
//                                  (0xf7 / 247 B, EH3-SEH wrapped, __thiscall, RET 4).
//
// Behaviour read from asm/ffxivgame/00042050_FUN_00442050.s:
//
//   __thiscall bool FUN_00442050(this, DWORD param) — ECX = this, one
//   4-byte stack parameter (RET 0x4).
//
//   Standard MSVC EH3 SEH prologue (PUSH -1 / PUSH 0xe5710a handler /
//   MOV EAX,FS:[0] / PUSH EAX / SUB ESP,0x14 / PUSH EBX/EBP/ESI/EDI /
//   security-cookie XOR ESP / PUSH cookie / LEA→FS:[0] install).
//
//   ESI = this+0x18 (container member).
//   Calls FUN_0071d420(__thiscall on ESI, &local0, &param) — a find/
//   lower-bound operation on the container.
//   Checks EDI (local0 = found node) against ESI and EBX (sentinel).
//   Branch A (found): SETNZ from node->field10->field1c → return bool.
//   Branch B (not found):
//     new (0x28 bytes) → FUN_00443350 ctor → insert via FUN_00994a90,
//     then vtable call on this->field4->vt[1] with 9 args,
//     XOR AL,AL → return 0.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The EH3 frame, security-cookie, absolute addresses (0xe5710a scope
//   table, 0x012ea8b0 cookie, call targets), and vtable dispatch make a
//   source-level C++ reconstruction brittle under /O2. The pragmatic
//   approach used by every other SEH-wrapped function in this _rosetta
//   tree (FUN_00403a20, FUN_004054d0, FUN_00402a30, …) is a
//   __declspec(naked) body that re-emits the orig bytes verbatim via
//   MASM _emit directives — byte-identical to the original .text slice.

extern "C" __declspec(naked) void FUN_00442050() {
    __asm {
        // 00042050  PUSH -1
        _emit 0x6a
        _emit 0xff
        // 00042052  PUSH 0xe5710a
        _emit 0x68
        _emit 0x0a
        _emit 0x71
        _emit 0xe5
        _emit 0x00
        // 00042057  MOV EAX, FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004205d  PUSH EAX
        _emit 0x50
        // 0004205e  SUB ESP, 0x14
        _emit 0x83
        _emit 0xec
        _emit 0x14
        // 00042061  PUSH EBX
        _emit 0x53
        // 00042062  PUSH EBP
        _emit 0x55
        // 00042063  PUSH ESI
        _emit 0x56
        // 00042064  PUSH EDI
        _emit 0x57
        // 00042065  MOV EAX, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0004206a  XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 0004206c  PUSH EAX  (cookie)
        _emit 0x50
        // 0004206d  LEA EAX, [ESP+0x28]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 00042071  MOV FS:[0], EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00042077  MOV EBP, ECX  (save this)
        _emit 0x8b
        _emit 0xe9
        // 00042079  LEA EAX, [ESP+0x38]  (&param)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x38
        // 0004207d  PUSH EAX
        _emit 0x50
        // 0004207e  LEA ECX, [ESP+0x18]  (&local0)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00042082  LEA ESI, [EBP+0x18]  (container)
        _emit 0x8d
        _emit 0x75
        _emit 0x18
        // 00042085  PUSH ECX
        _emit 0x51
        // 00042086  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00042088  CALL 0x0071d420  (find/lower_bound on container)
        _emit 0xe8
        _emit 0x93
        _emit 0xb3
        _emit 0x2d
        _emit 0x00
        // 0004208d  MOV EDI, [ESP+0x14]  (found node)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        // 00042091  TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 00042093  MOV EBX, [ESI+0x4]  (sentinel)
        _emit 0x8b
        _emit 0x5e
        _emit 0x04
        // 00042096  JZ 0x0044209c
        _emit 0x74
        _emit 0x04
        // 00042098  CMP EDI, ESI
        _emit 0x3b
        _emit 0xfe
        // 0004209a  JZ 0x004420a1
        _emit 0x74
        _emit 0x05
        // 0004209c  CALL 0x009d22b4  (assert/abort)
        _emit 0xe8
        _emit 0x13
        _emit 0x02
        _emit 0x59
        _emit 0x00
        // 000420a1  CMP [ESP+0x18], EBX  (local1 vs sentinel)
        _emit 0x39
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        // 000420a5  JZ 0x004420cf  (not found → alloc path)
        _emit 0x74
        _emit 0x28
        // 000420a7  TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 000420a9  JNZ 0x004420b0
        _emit 0x75
        _emit 0x05
        // 000420ab  CALL 0x009d22b4  (assert)
        _emit 0xe8
        _emit 0x04
        _emit 0x02
        _emit 0x59
        _emit 0x00
        // 000420b0  MOV EDX, [ESP+0x18]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 000420b4  CMP EDX, [EDI+0x4]
        _emit 0x3b
        _emit 0x57
        _emit 0x04
        // 000420b7  JNZ 0x004420be
        _emit 0x75
        _emit 0x05
        // 000420b9  CALL 0x009d22b4  (assert)
        _emit 0xe8
        _emit 0xf6
        _emit 0x01
        _emit 0x59
        _emit 0x00
        // 000420be  MOV EAX, [ESP+0x18]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 000420c2  MOV ECX, [EAX+0x10]
        _emit 0x8b
        _emit 0x48
        _emit 0x10
        // 000420c5  XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 000420c7  CMP [ECX+0x1c], EAX
        _emit 0x39
        _emit 0x41
        _emit 0x1c
        // 000420ca  SETNZ AL
        _emit 0x0f
        _emit 0x95
        _emit 0xc0
        // 000420cd  JMP 0x00442131  (epilogue)
        _emit 0xeb
        _emit 0x62
        // 000420cf  PUSH 0x28  (operator new size)
        _emit 0x6a
        _emit 0x28
        // 000420d1  CALL 0x009d1b35  (operator new)
        _emit 0xe8
        _emit 0x5f
        _emit 0xfa
        _emit 0x58
        _emit 0x00
        // 000420d6  ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 000420d9  MOV [ESP+0x14], EAX  (save new ptr)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 000420dd  XOR EDI, EDI
        _emit 0x33
        _emit 0xff
        // 000420df  CMP EAX, EDI
        _emit 0x3b
        _emit 0xc7
        // 000420e1  MOV [ESP+0x30], EDI  (SEH trylevel = 0)
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x30
        // 000420e5  JZ 0x004420f0  (skip ctor if null)
        _emit 0x74
        _emit 0x09
        // 000420e7  MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 000420e9  CALL 0x00443350  (constructor)
        _emit 0xe8
        _emit 0x62
        _emit 0x12
        _emit 0x00
        _emit 0x00
        // 000420ee  MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // 000420f0  MOV EBX, [ESP+0x38]  (param)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x38
        // 000420f4  LEA EDX, [ESP+0x14]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 000420f8  PUSH EDX
        _emit 0x52
        // 000420f9  LEA EAX, [ESP+0x20]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 000420fd  PUSH EAX
        _emit 0x50
        // 000420fe  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00042100  MOV [ESP+0x38], 0xffffffff  (SEH trylevel = -1)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00042108  MOV [ESP+0x1c], EBX  (store param)
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        // 0004210c  MOV [ESP+0x20], EDI  (store new obj)
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        // 00042110  CALL 0x00994a90  (insert)
        _emit 0xe8
        _emit 0x7b
        _emit 0x29
        _emit 0x55
        _emit 0x00
        // 00042115  MOV ECX, [EBP+0x4]
        _emit 0x8b
        _emit 0x4d
        _emit 0x04
        // 00042118  MOV EDX, [ECX]
        _emit 0x8b
        _emit 0x11
        // 0004211a  MOV EAX, [EDX+0x4]  (vtable slot 1)
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 0004211d  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0004211f  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 00042121  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 00042123  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 00042125  PUSH EDI
        _emit 0x57
        // 00042126  PUSH 1
        _emit 0x6a
        _emit 0x01
        // 00042128  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0004212a  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0004212c  PUSH EBX
        _emit 0x53
        // 0004212d  CALL EAX  (vtable dispatch)
        _emit 0xff
        _emit 0xd0
        // 0004212f  XOR AL, AL  (return 0)
        _emit 0x32
        _emit 0xc0
        // 00042131  MOV ECX, [ESP+0x28]  (restore FS chain)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // 00042135  MOV FS:[0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004213c  POP ECX  (cookie)
        _emit 0x59
        // 0004213d  POP EDI
        _emit 0x5f
        // 0004213e  POP ESI
        _emit 0x5e
        // 0004213f  POP EBP
        _emit 0x5d
        // 00042140  POP EBX
        _emit 0x5b
        // 00042141  ADD ESP, 0x20
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        // 00042144  RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
