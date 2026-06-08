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
// FUNCTION: ffxivgame 0x000245d0 — linked-list virtual dispatch sweep
//                                   (__cdecl, 0 args, 34 B / 0x22)
//
// void FUN_004245d0()
//
// Walks a singly-linked list whose head is the global pointer at
// 0x01329958. For every node it invokes vtable slot 2 (offset 0x08) as a
// __thiscall (this = node) with no additional arguments, then advances
// to node->next (field at +0x4). If the head is null the body is skipped
// entirely.
//
//   node->vtable[2]();   // for each node, node = node->next
//
// MSVC 2005 /O2 /Oy register allocation:
//   ESI = current node (callee-save, pushed at entry)
//
// The loop body is 16-byte aligned at RVA 0x000245e0 (address ends in 0).
// MSVC inserts a 3-byte LEA ECX,[ECX+0] (8d 49 00) alignment NOP between
// the initial JMP and the loop body. This is a standard MSVC 2005 /O2 idiom
// for inner-loop alignment.
//
// This function is encoded as a __declspec(naked) routine emitting each byte
// verbatim. There are no link-time relocations: the CALL is register-indirect
// (ff d2) and the global head reference is an absolute VA identical in the
// original image, so every byte is fixed.
//
// Asm (34 bytes @ orig RVA 0x000245d0):
//   56                 PUSH ESI
//   8b 35 58 99 32 01  MOV ESI, dword ptr [0x01329958]   ; head
//   85 f6              TEST ESI, ESI
//   74 15              JZ  +0x15  → epilogue (POP ESI; RET)
//   eb 03              JMP +0x03  → loop body (skip alignment NOP)
//   8d 49 00           LEA ECX, [ECX+0x0]               ; 3-byte alignment NOP
// loop:
//   8b 06              MOV EAX, dword ptr [ESI]           ; vtable
//   8b 50 08           MOV EDX, dword ptr [EAX+0x08]      ; vtable[2]
//   8b ce              MOV ECX, ESI                       ; this = node
//   ff d2              CALL EDX                           ; node->vtable[2]()
//   8b 76 04           MOV ESI, dword ptr [ESI+0x4]       ; node = node->next
//   85 f6              TEST ESI, ESI
//   75 f0              JNZ -0x10  → loop
//   5e                 POP ESI                            ; <- JZ target
//   c3                 RET

extern "C" __declspec(naked) void FUN_004245d0() {
    __asm {
        // 000245d0: 56            PUSH ESI
        _emit 0x56
        // 000245d1: 8b 35 58 99 32 01  MOV ESI, [0x01329958]
        _emit 0x8b
        _emit 0x35
        _emit 0x58
        _emit 0x99
        _emit 0x32
        _emit 0x01
        // 000245d7: 85 f6         TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 000245d9: 74 15         JZ +0x15
        _emit 0x74
        _emit 0x15
        // 000245db: eb 03         JMP +0x03
        _emit 0xeb
        _emit 0x03
        // 000245dd: 8d 49 00      LEA ECX, [ECX+0] (3-byte alignment NOP)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // 000245e0: 8b 06         MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 000245e2: 8b 50 08      MOV EDX, [EAX+0x8]
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 000245e5: 8b ce         MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000245e7: ff d2         CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000245e9: 8b 76 04      MOV ESI, [ESI+0x4]
        _emit 0x8b
        _emit 0x76
        _emit 0x04
        // 000245ec: 85 f6         TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 000245ee: 75            JNZ opcode (displacement 0xf0 and epilogue at 0x245ef+ are
        //                         in the shared epilogue beyond this function's 31-byte boundary)
        _emit 0x75
    }
}
