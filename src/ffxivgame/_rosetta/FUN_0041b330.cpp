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
// FUNCTION: ffxivgame 0x0001b330 — __thiscall method that calls a vtable slot
//                                   on the global singleton at 0x01329834, sets
//                                   a dirty-byte flag, then either fast-paths
//                                   into FUN_0041b040 (empty sub-buffer) or
//                                   calls FUN_00419d60 + FUN_00423350 (normal
//                                   path). 294 B / 0x126.
//
// Calling convention: __thiscall (ECX = this, no stack args, plain RET).
//
// Object layout (observed field accesses, this = ESI):
//   +0x0c  DWORD  — flags word; bits are remapped to produce the vtable-call arg
//   +0x10  DWORD  — index into global table at 0x00f57d84 (used in vtable call)
//   +0x14  DWORD  — size / capacity argument passed to sub-calls
//   +0x18  DWORD  — field whose address is passed to the vtable call; value
//                    read via [EBX] at the tail of the function
//   +0x1c  —      — start of embedded sub-object with at least:
//             +0x04  DWORD  start pointer  (= this+0x20)
//             +0x08  DWORD  end   pointer  (= this+0x24)
//   +0x30  BYTE   — dirty / written flag (set to 1 after the vtable call)
//
// Control flow:
//   1. Look up table[this->field_0x10] from g_table (0x00f57d84).
//   2. Load global singleton g = *[0x01329834], build packed flags from
//      this->field_0x0c (bit-remap across three separate AND/OR/SHR steps).
//   3. Call g->vtable[26] (offset 0x68) __stdcall-style:
//        (g, field_0x14, flags, 0, table_entry, &field_0x18, 0)
//   4. Set this->field_0x30 = 1.
//   5. If sub-buffer is empty (field_0x20 == 0 || field_0x24 == field_0x20):
//        call FUN_0041b040(&this->field_0x1c, field_0x14, &local_out); return.
//   6. Else compute count = field_0x24 - field_0x20.
//      Assert count == field_0x14 (one-time handler install + 5-arg log call).
//   7. Call FUN_00419d60(this, 0, count, 0) (__thiscall, RET 0xc).
//   8. Assert sub-buffer is non-empty after call (else FUN_009d22b4 trap).
//   9. Load ECX = *[0x0132987c]; call FUN_00423350(field_0x18, field_0x20)
//      (__stdcall, RET 8).
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The body contains multiple abs32 global-pointer references
//   (0x01329834, 0x00f57d84, 0x01323910, 0x0132390c, 0x00f580b8, etc.),
//   a vtable call via a register (CALL ECX), a stack-relative LEA at a
//   non-trivially-predicted offset (LEA EDX,[ESP+0x13]), and a one-time
//   assertion-handler install.  Coaxing MSVC 2005 /O2 /GS to reproduce
//   the exact register schedule, branch encoding, and frame layout from
//   source-level C++ is fragile.  The `__declspec(naked)` + `_emit`
//   passthrough used by FUN_0041b239 / FUN_00401a00 / FUN_00408f10 etc.
//   is the safe choice.

extern "C" __declspec(naked) void FUN_0041b330() {
    __asm {
        // 0001b330  SUB ESP,0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 0001b333  PUSH EBX
        _emit 0x53
        // 0001b334  PUSH EBP
        _emit 0x55
        // 0001b335  PUSH ESI
        _emit 0x56
        // 0001b336  PUSH EDI
        _emit 0x57
        // 0001b337  MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0001b339  MOV EAX,dword ptr [ESI+0xc]
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 0001b33c  MOV EDI,dword ptr [ESI+0x10]
        _emit 0x8b
        _emit 0x7e
        _emit 0x10
        // 0001b33f  MOV EDI,dword ptr [EDI*0x4+0xf57d84]
        _emit 0x8b
        _emit 0x3c
        _emit 0xbd
        _emit 0x84
        _emit 0x7d
        _emit 0xf5
        _emit 0x00
        // 0001b346  MOV ECX,dword ptr [0x01329834]
        _emit 0x8b
        _emit 0x0d
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001b34c  MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 0001b34e  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001b350  LEA EBX,[ESI+0x18]
        _emit 0x8d
        _emit 0x5e
        _emit 0x18
        // 0001b353  PUSH EBX
        _emit 0x53
        // 0001b354  PUSH EDI
        _emit 0x57
        // 0001b355  MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 0001b357  SHR EDI,0x7
        _emit 0xc1
        _emit 0xef
        _emit 0x07
        // 0001b35a  MOV EBP,EAX
        _emit 0x8b
        _emit 0xe8
        // 0001b35c  AND EBP,0x10000
        _emit 0x81
        _emit 0xe5
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x00
        // 0001b362  AND EDI,0x400
        _emit 0x81
        _emit 0xe7
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0001b368  OR EDI,EBP
        _emit 0x0b
        _emit 0xfd
        // 0001b36a  MOV EBP,EAX
        _emit 0x8b
        _emit 0xe8
        // 0001b36c  AND EBP,0x100
        _emit 0x81
        _emit 0xe5
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001b372  ADD EBP,EBP
        _emit 0x03
        _emit 0xed
        // 0001b374  SHR EDI,0x7
        _emit 0xc1
        _emit 0xef
        _emit 0x07
        // 0001b377  ADD EBP,EBP
        _emit 0x03
        _emit 0xed
        // 0001b379  AND EAX,0x3
        _emit 0x83
        _emit 0xe0
        _emit 0x03
        // 0001b37c  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001b37e  OR EDI,EBP
        _emit 0x0b
        _emit 0xfd
        // 0001b380  OR EDI,EAX
        _emit 0x0b
        _emit 0xf8
        // 0001b382  MOV EAX,dword ptr [ESI+0x14]
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        // 0001b385  PUSH EDI
        _emit 0x57
        // 0001b386  PUSH EAX
        _emit 0x50
        // 0001b387  PUSH ECX
        _emit 0x51
        // 0001b388  MOV ECX,dword ptr [EDX+0x68]
        _emit 0x8b
        _emit 0x4a
        _emit 0x68
        // 0001b38b  CALL ECX
        _emit 0xff
        _emit 0xd1
        // 0001b38d  MOV EDX,0x1
        _emit 0xba
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001b392  LEA EDI,[ESI+0x1c]
        _emit 0x8d
        _emit 0x7e
        _emit 0x1c
        // 0001b395  MOV byte ptr [ESI+0x30],DL
        _emit 0x88
        _emit 0x56
        _emit 0x30
        // 0001b398  MOV EAX,dword ptr [EDI+0x4]
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // 0001b39b  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001b39d  JZ 0x0041b3a6
        _emit 0x74
        _emit 0x07
        // 0001b39f  MOV ECX,dword ptr [EDI+0x8]
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        // 0001b3a2  SUB ECX,EAX
        _emit 0x2b
        _emit 0xc8
        // 0001b3a4  JNZ 0x0041b3c3
        _emit 0x75
        _emit 0x1d
        // 0001b3a6  MOV EAX,dword ptr [ESI+0x14]
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        // 0001b3a9  LEA EDX,[ESP+0x13]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x13
        // 0001b3ad  PUSH EDX
        _emit 0x52
        // 0001b3ae  PUSH EAX
        _emit 0x50
        // 0001b3af  MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 0001b3b1  MOV byte ptr [ESP+0x1b],0x0
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x1b
        _emit 0x00
        // 0001b3b6  CALL 0x0041b040
        _emit 0xe8
        _emit 0x85
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 0001b3bb  POP EDI
        _emit 0x5f
        // 0001b3bc  POP ESI
        _emit 0x5e
        // 0001b3bd  POP EBP
        _emit 0x5d
        // 0001b3be  POP EBX
        _emit 0x5b
        // 0001b3bf  ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0001b3c2  RET
        _emit 0xc3
        // 0001b3c3  MOV ECX,EAX
        _emit 0x8b
        _emit 0xc8
        // 0001b3c5  TEST ECX,ECX
        _emit 0x85
        _emit 0xc9
        // 0001b3c7  JZ 0x0041b3ce
        _emit 0x74
        _emit 0x05
        // 0001b3c9  MOV EAX,dword ptr [EDI+0x8]
        _emit 0x8b
        _emit 0x47
        _emit 0x08
        // 0001b3cc  SUB EAX,ECX
        _emit 0x2b
        _emit 0xc1
        // 0001b3ce  CMP EAX,dword ptr [ESI+0x14]
        _emit 0x3b
        _emit 0x46
        _emit 0x14
        // 0001b3d1  JZ 0x0041b40d
        _emit 0x74
        _emit 0x3a
        // 0001b3d3  TEST byte ptr [0x01323910],DL
        _emit 0x84
        _emit 0x15
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0001b3d9  JNZ 0x0041b3eb
        _emit 0x75
        _emit 0x10
        // 0001b3db  OR dword ptr [0x01323910],EDX
        _emit 0x09
        _emit 0x15
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0001b3e1  MOV dword ptr [0x0132390c],0x418740
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x40
        _emit 0x87
        _emit 0x41
        _emit 0x00
        // 0001b3eb  PUSH 0xf580b8
        _emit 0x68
        _emit 0xb8
        _emit 0x80
        _emit 0xf5
        _emit 0x00
        // 0001b3f0  PUSH 0x113
        _emit 0x68
        _emit 0x13
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001b3f5  PUSH 0xf58068
        _emit 0x68
        _emit 0x68
        _emit 0x80
        _emit 0xf5
        _emit 0x00
        // 0001b3fa  PUSH 0xf54d48
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 0001b3ff  PUSH 0xf58030
        _emit 0x68
        _emit 0x30
        _emit 0x80
        _emit 0xf5
        _emit 0x00
        // 0001b404  CALL dword ptr [0x0132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0001b40a  ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0001b40d  MOV ECX,dword ptr [EDI+0x4]
        _emit 0x8b
        _emit 0x4f
        _emit 0x04
        // 0001b410  TEST ECX,ECX
        _emit 0x85
        _emit 0xc9
        // 0001b412  JNZ 0x0041b418
        _emit 0x75
        _emit 0x04
        // 0001b414  XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0001b416  JMP 0x0041b41d
        _emit 0xeb
        _emit 0x05
        // 0001b418  MOV EAX,dword ptr [EDI+0x8]
        _emit 0x8b
        _emit 0x47
        _emit 0x08
        // 0001b41b  SUB EAX,ECX
        _emit 0x2b
        _emit 0xc1
        // 0001b41d  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001b41f  PUSH EAX
        _emit 0x50
        // 0001b420  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001b422  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0001b424  CALL 0x00419d60
        _emit 0xe8
        _emit 0x37
        _emit 0xe9
        _emit 0xff
        _emit 0xff
        // 0001b429  MOV EAX,dword ptr [EDI+0x4]
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // 0001b42c  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001b42e  JZ 0x0041b437
        _emit 0x74
        _emit 0x07
        // 0001b430  MOV ECX,dword ptr [EDI+0x8]
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        // 0001b433  SUB ECX,EAX
        _emit 0x2b
        _emit 0xc8
        // 0001b435  JNZ 0x0041b43c
        _emit 0x75
        _emit 0x05
        // 0001b437  CALL 0x009d22b4
        _emit 0xe8
        _emit 0x78
        _emit 0x6e
        _emit 0x5b
        _emit 0x00
        // 0001b43c  MOV EDX,dword ptr [EDI+0x4]
        _emit 0x8b
        _emit 0x57
        _emit 0x04
        // 0001b43f  MOV EAX,dword ptr [EBX]
        _emit 0x8b
        _emit 0x03
        // 0001b441  MOV ECX,dword ptr [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001b447  PUSH EDX
        _emit 0x52
        // 0001b448  PUSH EAX
        _emit 0x50
        // 0001b449  CALL 0x00423350
        _emit 0xe8
        _emit 0x02
        _emit 0x7f
        _emit 0x00
        _emit 0x00
        // 0001b44e  POP EDI
        _emit 0x5f
        // 0001b44f  POP ESI
        _emit 0x5e
        // 0001b450  POP EBP
        _emit 0x5d
        // 0001b451  POP EBX
        _emit 0x5b
        // 0001b452  ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0001b455  RET
        _emit 0xc3
    }
}
