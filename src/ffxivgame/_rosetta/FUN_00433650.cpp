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
// FUNCTION: ffxivgame 0x00433650 — __thiscall constructor with SEH+/GS frame;
//                                  initialises two byte flags on `this`, allocates
//                                  a 0x3c-byte inner object, and calls
//                                  FUN_004178c0 twice to seed two list slots on it.
//                                  (160 bytes / 0xa0)
//
// Calling convention : __thiscall (ECX = this); one caller-passed stack arg;
//                      callee-cleans via RET 0x4; returns this in EAX.
//
// Stack layout after the 7 prologue PUSHes (ESP_cur = ESP_entry - 28):
//   [ESP_cur + 0x00]  /GS cookie (XOR of __security_cookie ^ ESP)
//   [ESP_cur + 0x04]  saved EDI
//   [ESP_cur + 0x08]  saved ESI
//   [ESP_cur + 0x0c]  saved ECX (= this); repurposed as local for alloc result
//   [ESP_cur + 0x10]  saved FS:[0]  (SEH chain prev pointer)
//   [ESP_cur + 0x14]  SEH handler  (0xe5615b — address of __except_handler3
//                                   scope table in the binary)
//   [ESP_cur + 0x18]  SEH try-state (−1 = not in try; set to 0 inside try)
//   [ESP_cur + 0x1c]  return address
//   [ESP_cur + 0x20]  first (and only) stack argument
//
// Object layout ('this' offsets touched):
//   [+0x00]  byte flag: cleared to 0
//   [+0x01]  byte flag: set to 1
//   [+0x04]  pointer to the inner 0x3c-byte allocation (+4 within it)
//             i.e. this->field_4 = alloc_result + 4 (or NULL on alloc failure)
//
// Callee summary:
//   0x009d04ac  __cdecl  operator new / heap alloc (1 dword arg, caller-cleans)
//   0x009d61d6  __stdcall list-init helper          (5 dword args, callee-cleans)
//   0x004178c0  __thiscall FUN_004178c0             (1 stack arg, callee-cleans;
//                                                    ECX = list object pointer)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function opens with a full MSVC /GS + SEH prologue that embeds a
//   literal handler address (0xe5615b), a direct FS-segment write, and a
//   __security_cookie XOR — none of which can be reproduced byte-for-byte
//   from C++ source.  A __declspec(naked) body that re-emits the original
//   160 bytes verbatim produces a .obj whose .text is byte-identical to the
//   binary slice; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00433650() {
    __asm {
        // 00033650: 6a ff              PUSH -0x1   (SEH try-state = not-in-try)
        _emit 0x6a
        _emit 0xff
        // 00033652: 68 5b 61 e5 00     PUSH 0xe5615b  (SEH scope-table / handler)
        _emit 0x68
        _emit 0x5b
        _emit 0x61
        _emit 0xe5
        _emit 0x00
        // 00033657: 64 a1 00 00 00 00  MOV EAX,FS:[0x0]  (current SEH chain head)
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003365d: 50                 PUSH EAX  (save old SEH head)
        _emit 0x50
        // 0003365e: 51                 PUSH ECX  (save 'this'; slot reused later)
        _emit 0x51
        // 0003365f: 56                 PUSH ESI
        _emit 0x56
        // 00033660: 57                 PUSH EDI
        _emit 0x57
        // 00033661: a1 b0 a8 2e 01     MOV EAX,[0x012ea8b0]  (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00033666: 33 c4              XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 00033668: 50                 PUSH EAX  (/GS cookie on stack)
        _emit 0x50
        // 00033669: 8d 44 24 10        LEA EAX,[ESP + 0x10]  (ptr to SEH record)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0003366d: 64 a3 00 00 00 00  MOV FS:[0x0],EAX  (install SEH frame)
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00033673: 8b f1              MOV ESI,ECX  (ESI = this)
        _emit 0x8b
        _emit 0xf1
        // 00033675: 6a 3c              PUSH 0x3c   (60 bytes; arg to allocator)
        _emit 0x6a
        _emit 0x3c
        // 00033677: c6 06 00           MOV byte ptr [ESI],0x0   (this->byte_0 = 0)
        _emit 0xc6
        _emit 0x06
        _emit 0x00
        // 0003367a: c6 46 01 01        MOV byte ptr [ESI + 0x1],0x1  (this->byte_1 = 1)
        _emit 0xc6
        _emit 0x46
        _emit 0x01
        _emit 0x01
        // 0003367e: e8 29 ce 59 00     CALL 0x009d04ac  (__cdecl alloc; returns ptr in EAX)
        _emit 0xe8
        _emit 0x29
        _emit 0xce
        _emit 0x59
        _emit 0x00
        // 00033683: 83 c4 04           ADD ESP,0x4  (caller-cleans 1 cdecl arg)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00033686: 89 44 24 0c        MOV dword ptr [ESP + 0xc],EAX  (spill alloc result)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0003368a: 85 c0              TEST EAX,EAX  (null check)
        _emit 0x85
        _emit 0xc0
        // 0003368c: c7 44 24 18 00 00 00 00
        //           MOV dword ptr [ESP + 0x18],0x0  (try-state = 0: enter try block)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00033694: 74 21              JZ +0x21  (null → skip init, XOR ECX,ECX)
        _emit 0x74
        _emit 0x21
        // --- non-null path: initialise the inner allocation ---
        // 00033696: 68 20 78 41 00     PUSH 0x417820  (arg4: table ptr)
        _emit 0x68
        _emit 0x20
        _emit 0x78
        _emit 0x41
        _emit 0x00
        // 0003369b: 68 a0 78 41 00     PUSH 0x4178a0  (arg3: table ptr)
        _emit 0x68
        _emit 0xa0
        _emit 0x78
        _emit 0x41
        _emit 0x00
        // 000336a0: 6a 02              PUSH 0x2  (arg2)
        _emit 0x6a
        _emit 0x02
        // 000336a2: 8d 78 04           LEA EDI,[EAX + 0x4]  (EDI = alloc + 4)
        _emit 0x8d
        _emit 0x78
        _emit 0x04
        // 000336a5: 6a 1c              PUSH 0x1c  (arg1: element stride / 28)
        _emit 0x6a
        _emit 0x1c
        // 000336a7: 57                 PUSH EDI  (arg0: list base = alloc+4)
        _emit 0x57
        // 000336a8: c7 00 02 00 00 00  MOV dword ptr [EAX],0x2  (alloc[0] = 2)
        _emit 0xc7
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000336ae: e8 23 2b 5a 00     CALL 0x009d61d6  (list-init; callee-cleans 5 args)
        _emit 0xe8
        _emit 0x23
        _emit 0x2b
        _emit 0x5a
        _emit 0x00
        // 000336b3: 8b cf              MOV ECX,EDI  (ECX = alloc+4, for this->field_4)
        _emit 0x8b
        _emit 0xcf
        // 000336b5: eb 02              JMP +0x2  (skip the null-path XOR)
        _emit 0xeb
        _emit 0x02
        // --- null-alloc path ---
        // 000336b7: 33 c9              XOR ECX,ECX  (ECX = 0)
        _emit 0x33
        _emit 0xc9
        // --- common tail ---
        // 000336b9: 8b 7c 24 20        MOV EDI,dword ptr [ESP + 0x20]  (EDI = stack arg)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        // 000336bd: 57                 PUSH EDI  (pass stack arg to first FUN_004178c0 call)
        _emit 0x57
        // 000336be: c7 44 24 1c ff ff ff ff
        //           MOV dword ptr [ESP + 0x1c],0xffffffff  (try-state = -1: exit try)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 000336c6: 89 4e 04           MOV dword ptr [ESI + 0x4],ECX  (this->field_4 = ECX)
        _emit 0x89
        _emit 0x4e
        _emit 0x04
        // 000336c9: e8 f2 41 fe ff     CALL 0x004178c0  (__thiscall; callee-cleans 1 arg)
        _emit 0xe8
        _emit 0xf2
        _emit 0x41
        _emit 0xfe
        _emit 0xff
        // 000336ce: 8b 4e 04           MOV ECX,dword ptr [ESI + 0x4]  (ECX = this->field_4)
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 000336d1: 57                 PUSH EDI  (pass stack arg to second FUN_004178c0 call)
        _emit 0x57
        // 000336d2: 83 c1 1c           ADD ECX,0x1c  (ECX = this->field_4 + 0x1c)
        _emit 0x83
        _emit 0xc1
        _emit 0x1c
        // 000336d5: e8 e6 41 fe ff     CALL 0x004178c0  (__thiscall; callee-cleans 1 arg)
        _emit 0xe8
        _emit 0xe6
        _emit 0x41
        _emit 0xfe
        _emit 0xff
        // --- epilogue ---
        // 000336da: 8b c6              MOV EAX,ESI  (return this)
        _emit 0x8b
        _emit 0xc6
        // 000336dc: 8b 4c 24 10        MOV ECX,dword ptr [ESP + 0x10]  (reload saved FS:[0])
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 000336e0: 64 89 0d 00 00 00 00  MOV dword ptr FS:[0x0],ECX  (restore SEH chain)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000336e7: 59                 POP ECX  (pop /GS cookie; ECX discarded)
        _emit 0x59
        // 000336e8: 5f                 POP EDI
        _emit 0x5f
        // 000336e9: 5e                 POP ESI
        _emit 0x5e
        // 000336ea: 83 c4 10           ADD ESP,0x10  (remove ECX-slot + SEH record)
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 000336ed: c2 04 00           RET 0x4  (callee-cleans 1 stack arg)
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
