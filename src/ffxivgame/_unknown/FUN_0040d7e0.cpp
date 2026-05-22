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
// FUNCTION: ffxivgame 0x0040d7e0 — __thiscall linked-list drain.
//
// Walks a singly-linked node list rooted at this->m_head (offset 0),
// calling this->m_sub->FUN_0040df70(node) on each node, then clears
// m_head to null.
//
// ECX = this (d7e0_Obj*), no stack args, caller's RET (no stack cleanup).
//
// Asm (39 bytes, RVA 0xd7e0–0xd806):
//   d7e0: 57                    PUSH EDI
//   d7e1: 8b f9                 MOV EDI, ECX          ; EDI = this
//   d7e3: 8b 07                 MOV EAX, [EDI]        ; EAX = m_head
//   d7e5: 85 c0                 TEST EAX, EAX
//   d7e7: 74 1d                 JZ +0x1d (to d806)    ; if null skip loop
//   d7e9: 56                    PUSH ESI
//   d7ea: 8d 9b 00 00 00 00     LEA EBX, [EBX]        ; 6-byte NOP (loop align)
//   ; loop top at d7f0 (16-byte aligned):
//   d7f0: 8b 4f 04              MOV ECX, [EDI+4]      ; ECX = m_sub (reload)
//   d7f3: 8b b0 a4 09 00 00     MOV ESI, [EAX+0x9a4]  ; ESI = cur->m_next
//   d7f9: 50                    PUSH EAX              ; arg = cur
//   d7fa: e8 71 07 00 00        CALL FUN_0040df70      ; thiscall: ECX=m_sub
//   d7ff: 85 f6                 TEST ESI, ESI
//   d801: 8b c6                 MOV EAX, ESI           ; EAX = next
//   d803: 75 eb                 JNZ d7f0               ; loop if next != null
//   d805: 5e                    POP ESI
//   ; d806: first byte of next fn (MOV [EDI], 0) — NOT part of this function
//
// The 6-byte LEA EBX,[EBX+0] at d7ea is an alignment NOP emitted by
// MSVC 2005 /O2 to bring the loop top to the next 16-byte boundary
// (0xd7ea + 6 = 0xd7f0 = 0x10-aligned). This is automatic when the
// correct amount of pre-loop code precedes the loop.

struct d7e0_Node {
    char _pad[0x9a4];
    d7e0_Node* m_next;
};

struct d7e0_Sub {
    void FUN_0040df70(d7e0_Node* node);
};

struct d7e0_Obj {
    d7e0_Node* m_head;
    d7e0_Sub*  m_sub;

    void FUN_0040d7e0();
};

void d7e0_Obj::FUN_0040d7e0()
{
    d7e0_Node* cur = m_head;
    if (cur != 0) {
        do {
            d7e0_Node* next = cur->m_next;
            m_sub->FUN_0040df70(cur);
            cur = next;
        } while (cur != 0);
    }
    m_head = 0;
}

// vim: ts=4 sts=4 sw=4 et
