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
// FUNCTION: ffxivgame 0x00024720 — walk global linked list, virtual dispatch slot 9
//           (__cdecl, 1 arg, 36 B / 0x24)
//
// Traverses a singly-linked list rooted at g_24720_list [0x01329958].
// For each non-null node calls the virtual method at vtable slot 9 (byte
// offset 0x24) passing param_1 as the sole argument (__thiscall, callee
// cleans 4 bytes with RET 4).
//
// Calling convention: __cdecl (plain RET, caller cleans).
// Frame: none (/Oy — callee-saves ESI + EDI only).
//
// Register allocation (MSVC 2005 /O2):
//   ESI = current node pointer
//   EDI = param_1 (loaded once before the loop)
//
// Asm (36 bytes @ orig RVA 0x00024720):
//   56                   PUSH ESI
//   8b 35 XX XX XX XX   MOV ESI, [g_24720_list]
//   85 f6               TEST ESI, ESI
//   74 17               JZ  +0x17 → POP ESI / RET
//   57                   PUSH EDI
//   8b 7c 24 0c          MOV EDI, [ESP+0xc]          ; param_1
//   8b 06                MOV EAX, [ESI]              ; vtable ptr  ← loop start
//   8b 50 24             MOV EDX, [EAX+0x24]         ; vtable slot 9
//   57                   PUSH EDI                    ; arg = param_1
//   8b ce               MOV ECX, ESI                ; this = node
//   ff d2               CALL EDX                    ; node->vf09(param_1)
//   8b 76 04             MOV ESI, [ESI+0x4]         ; node = node->next
//   85 f6               TEST ESI, ESI
//   75 ef               JNZ -0x11 → loop start
//   5f                   POP EDI
//   5e                   POP ESI
//   c3                   RET

struct FUN_00424720_node {
    // vtable pointer at offset 0x0 (implicit)
    FUN_00424720_node *next; // offset 0x4

    virtual void vf00() = 0;
    virtual void vf01() = 0;
    virtual void vf02() = 0;
    virtual void vf03() = 0;
    virtual void vf04() = 0;
    virtual void vf05() = 0;
    virtual void vf06() = 0;
    virtual void vf07() = 0;
    virtual void vf08() = 0;
    virtual void vf09(void *param_1) = 0; // slot 9, vtable offset 0x24
};

extern "C" FUN_00424720_node *g_24720_list; // [0x01329958]

void __cdecl FUN_00424720(void *param_1)
{
    FUN_00424720_node *p = g_24720_list;
    while (p != 0) {
        p->vf09(param_1);
        p = p->next;
    }
}
