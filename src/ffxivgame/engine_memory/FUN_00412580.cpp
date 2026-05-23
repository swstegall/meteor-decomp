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
// FUNCTION: ffxivgame 0x00012580 — __thiscall: intrusive linked-list count
//                                   (27 bytes)
//
// Asm (27 bytes @ orig RVA 0x00012580):
//   8b 51 04    MOV EDX, dword ptr [ECX + 0x4]       ; EDX = this->field_0x4 (inner ptr)
//   8b 4a 48    MOV ECX, dword ptr [EDX + 0x48]      ; ECX = inner->sentinel.next (list head)
//   83 c2 40    ADD EDX, 0x40                         ; EDX = &inner->sentinel
//   33 c0       XOR EAX, EAX                          ; count = 0
//   3b ca       CMP ECX, EDX                          ; head == &sentinel?
//   74 0b       JZ  done                              ; yes → empty list, return 0
//   90          NOP                                   ; alignment
//   8b 49 08    MOV ECX, dword ptr [ECX + 0x8]        ; ECX = node->next
//   83 c0 01    ADD EAX, 0x1                          ; count++
//   3b ca       CMP ECX, EDX                          ; node == &sentinel?
//   75 f6       JNZ loop_body                         ; no → loop
//   c3          RET
//
// Counts nodes in a sentinel-based intrusive linked list. The sentinel
// is embedded at inner+0x40; inner->sentinel.next (at inner+0x48) is
// the first real node (or points back to sentinel if empty). Each node's
// next pointer is at node+0x8.
//
// Structurally identical to FUN_0040f630 (same 27-byte template, offsets
// 0x44/0x4c there vs 0x40/0x48 here).

struct ListNode_00412580 {
    char _pad[0x8];
    ListNode_00412580 *next;
};

struct Inner_00412580 {
    char _pad1[0x40];
    ListNode_00412580 sentinel;
};

struct Outer_00412580 {
    char _pad[0x4];
    Inner_00412580 *field_0x4;

    int FUN_00412580();
};

int Outer_00412580::FUN_00412580() {
    Inner_00412580 *p = field_0x4;
    ListNode_00412580 *node = p->sentinel.next;
    int count = 0;
    if (node != &p->sentinel) {
        do {
            node = node->next;
            count++;
        } while (node != &p->sentinel);
    }
    return count;
}
