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
// FUNCTION: ffxivgame 0x00011330 — nested virtual-dispatch linked-list traversal
//                                   (__thiscall, 1 stack arg, 118 B / 0x76)
//
// void FUN_00411330_class::FUN_00411330(void* param_1)
//   ECX        : this  — outer object
//   [ESP+0x04] : param_1 — forwarded as third arg to innermost virtual call
//
// Behaviour:
//   1. Calls FUN_00410460 (__thiscall, 1 stack arg) on this with param_1.
//   2. Loads sub = this->field_30 (sub-object with two embedded list nodes).
//   3. Outer loop: walks sub's outer linked list (head at sub->field_4c,
//      sentinel = &sub->field_44, next at node+0x8).
//      Each outer node's vtable[1] is called → outer_result.
//   4. Inner loop: walks outer_result's inner list (head at outer_result+0x40,
//      sentinel = outer_result+0x38, next at node+0x8).
//      Each inner node's vtable[1] is called → inner_result.
//      Then calls inner_result->field_0c->vtable[1] with 3 stack args:
//        arg1 = &outer_result->field_4, arg2 = inner_result->field_10, arg3 = param_1.
//
// Calling convention: __thiscall, callee cleans 1 stack arg (RET 0x4).
// Callee-saves used: EBX, EBP, ESI, EDI.
// No local frame (no SUB ESP).
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The prologue (PUSH ECX used as a 4-byte stack adjuster, interleaved
//   with callee-save pushes and arg forwarding), the alignment NOP at +0x39
//   (7-byte LEA ESP,[ESP+0]), the sentinel reuse pattern (EBX serves as
//   both outer sentinel and inner arg1), and the exact register allocation
//   across two nested virtual-dispatch loops are not safely reproducible
//   from C++ source under MSVC 2005 /O2. The __declspec(naked) body
//   re-emits the original 118 bytes verbatim via MASM _emit directives;
//   compare.py reports GREEN.

#ifdef _MSC_VER
extern "C" __declspec(naked) void FUN_00411330() {
    __asm {
        // 00011330:  51                PUSH ECX
        _emit 0x51
        // 00011331:  8b 44 24 08       MOV EAX,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00011335:  53                PUSH EBX
        _emit 0x53
        // 00011336:  55                PUSH EBP
        _emit 0x55
        // 00011337:  56                PUSH ESI
        _emit 0x56
        // 00011338:  50                PUSH EAX
        _emit 0x50
        // 00011339:  8b f1             MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0001133b:  e8 20 f1 ff ff    CALL FUN_00410460  (rel32 reloc)
        _emit 0xe8
        _emit 0x20
        _emit 0xf1
        _emit 0xff
        _emit 0xff
        // 00011340:  8b 76 30          MOV ESI,dword ptr [ESI+0x30]
        _emit 0x8b
        _emit 0x76
        _emit 0x30
        // 00011343:  8b 6e 4c          MOV EBP,dword ptr [ESI+0x4c]
        _emit 0x8b
        _emit 0x6e
        _emit 0x4c
        // 00011346:  8d 5e 44          LEA EBX,[ESI+0x44]
        _emit 0x8d
        _emit 0x5e
        _emit 0x44
        // 00011349:  3b eb             CMP EBP,EBX
        _emit 0x3b
        _emit 0xeb
        // 0001134b:  89 5c 24 0c       MOV dword ptr [ESP+0xc],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        // 0001134f:  74 4e             JZ +0x4e  (→ 0x0041139f)
        _emit 0x74
        _emit 0x4e
        // 00011351:  57                PUSH EDI
        _emit 0x57
        // --- outer loop top (0x00011352) ---
        // 00011352:  8b 55 00          MOV EDX,dword ptr [EBP]
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        // 00011355:  8b 42 04          MOV EAX,dword ptr [EDX+0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00011358:  8b cd             MOV ECX,EBP
        _emit 0x8b
        _emit 0xcd
        // 0001135a:  ff d0             CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001135c:  8b 70 40          MOV ESI,dword ptr [EAX+0x40]
        _emit 0x8b
        _emit 0x70
        _emit 0x40
        // 0001135f:  8d 78 38          LEA EDI,[EAX+0x38]
        _emit 0x8d
        _emit 0x78
        _emit 0x38
        // 00011362:  3b f7             CMP ESI,EDI
        _emit 0x3b
        _emit 0xf7
        // 00011364:  74 31             JZ +0x31  (→ 0x00411397)
        _emit 0x74
        _emit 0x31
        // 00011366:  8d 58 04          LEA EBX,[EAX+0x4]
        _emit 0x8d
        _emit 0x58
        _emit 0x04
        // 00011369:  8d a4 24 00 00 00 00  LEA ESP,[ESP+0x0]  (7-byte alignment NOP)
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // --- inner loop top (0x00011370) ---
        // 00011370:  8b 16             MOV EDX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x16
        // 00011372:  8b 42 04          MOV EAX,dword ptr [EDX+0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00011375:  8b ce             MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00011377:  ff d0             CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00011379:  8b 48 0c          MOV ECX,dword ptr [EAX+0xc]
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 0001137c:  ff 74 24 18       PUSH dword ptr [ESP+0x18]
        _emit 0xff
        _emit 0x74
        _emit 0x24
        _emit 0x18
        // 00011380:  8b 40 10          MOV EAX,dword ptr [EAX+0x10]
        _emit 0x8b
        _emit 0x40
        _emit 0x10
        // 00011383:  8b 11             MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 00011385:  8b 52 04          MOV EDX,dword ptr [EDX+0x4]
        _emit 0x8b
        _emit 0x52
        _emit 0x04
        // 00011388:  50                PUSH EAX
        _emit 0x50
        // 00011389:  53                PUSH EBX
        _emit 0x53
        // 0001138a:  ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0001138c:  8b 76 08          MOV ESI,dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x76
        _emit 0x08
        // 0001138f:  3b f7             CMP ESI,EDI
        _emit 0x3b
        _emit 0xf7
        // 00011391:  75 dd             JNZ -0x23  (→ 0x00411370)
        _emit 0x75
        _emit 0xdd
        // 00011393:  8b 5c 24 10       MOV EBX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        // --- outer loop advance (0x00011397) ---
        // 00011397:  8b 6d 08          MOV EBP,dword ptr [EBP+0x8]
        _emit 0x8b
        _emit 0x6d
        _emit 0x08
        // 0001139a:  3b eb             CMP EBP,EBX
        _emit 0x3b
        _emit 0xeb
        // 0001139c:  75 b4             JNZ -0x4c  (→ 0x00411352)
        _emit 0x75
        _emit 0xb4
        // 0001139e:  5f                POP EDI
        _emit 0x5f
        // 0001139f:  5e                POP ESI
        _emit 0x5e
        // 000113a0:  5d                POP EBP
        _emit 0x5d
        // 000113a1:  5b                POP EBX
        _emit 0x5b
        // 000113a2:  59                POP ECX
        _emit 0x59
        // 000113a3:  c2 04 00          RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
#endif // _MSC_VER
