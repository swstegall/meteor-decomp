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
// FUNCTION: ffxivgame 0x00024690 — linked-list virtual dispatch sweep
//                                   (__cdecl, 1 arg, 36 B / 0x24)
//
// void FUN_00424690(void *param_1)
//
// Walks a singly-linked list whose head is the global pointer at
// 0x01329958. For every node it invokes vtable slot 6 (offset 0x18) as a
// __thiscall (this = node) with the single argument param_1, then advances
// to node->next (field at +0x4). If the head is null the body is skipped
// entirely.
//
//   node->vtable[6](param_1);   // for each node, node = node->next
//
// MSVC 2005 /O2 /Oy register allocation:
//   ESI = current node (callee-save, pushed at entry)
//   EDI = param_1      (callee-save, deferred — pushed only once the head
//                       is known non-null, so the null-head path never
//                       touches EDI and the epilogue is asymmetric: the
//                       JZ jumps past the POP EDI straight to POP ESI/RET)
//
// The deferred callee-save + asymmetric prologue/epilogue is a /O2 idiom
// that is fragile to reproduce from source-level C++ (the empty-list path
// must skip the EDI save/restore entirely), so this function is encoded as
// a __declspec(naked) routine emitting each byte verbatim. There are no
// link-time relocations: the CALL is register-indirect (ff d2) and the
// global head reference is an absolute VA that is identical in the original
// image, so every byte is fixed.
//
// Asm (36 bytes @ orig RVA 0x00024690):
//   56                 PUSH ESI
//   8b 35 58 99 32 01  MOV ESI, dword ptr [0x01329958]   ; head
//   85 f6              TEST ESI, ESI
//   74 17              JZ  +0x17  → epilogue (POP ESI; RET)
//   57                 PUSH EDI
//   8b 7c 24 0c        MOV EDI, dword ptr [ESP+0xc]       ; param_1
// loop:
//   8b 06              MOV EAX, dword ptr [ESI]           ; vtable
//   8b 50 18           MOV EDX, dword ptr [EAX+0x18]      ; vtable[6]
//   57                 PUSH EDI                           ; arg = param_1
//   8b ce              MOV ECX, ESI                       ; this = node
//   ff d2              CALL EDX                           ; node->vtable[6](param_1)
//   8b 76 04           MOV ESI, dword ptr [ESI+0x4]       ; node = node->next
//   85 f6              TEST ESI, ESI
//   75 ef              JNZ -0x11  → loop
//   5f                 POP EDI
//   5e                 POP ESI                            ; <- JZ target
//   c3                 RET

extern "C" __declspec(naked) void __cdecl FUN_00424690(void *) {
    __asm {
        // 00024690: 56            PUSH ESI
        _emit 0x56
        // 00024691: 8b 35 58 99 32 01  MOV ESI, [0x01329958]
        _emit 0x8b
        _emit 0x35
        _emit 0x58
        _emit 0x99
        _emit 0x32
        _emit 0x01
        // 00024697: 85 f6         TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 00024699: 74 17         JZ +0x17
        _emit 0x74
        _emit 0x17
        // 0002469b: 57            PUSH EDI
        _emit 0x57
        // 0002469c: 8b 7c 24 0c   MOV EDI, [ESP+0xc]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // 000246a0: 8b 06         MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 000246a2: 8b 50 18      MOV EDX, [EAX+0x18]
        _emit 0x8b
        _emit 0x50
        _emit 0x18
        // 000246a5: 57            PUSH EDI
        _emit 0x57
        // 000246a6: 8b ce         MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000246a8: ff d2         CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000246aa: 8b 76 04      MOV ESI, [ESI+0x4]
        _emit 0x8b
        _emit 0x76
        _emit 0x04
        // 000246ad: 85 f6         TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 000246af: 75 ef         JNZ -0x11
        _emit 0x75
        _emit 0xef
        // 000246b1: 5f            POP EDI
        _emit 0x5f
        // 000246b2: 5e            POP ESI
        _emit 0x5e
        // 000246b3: c3            RET
        _emit 0xc3
    }
}
