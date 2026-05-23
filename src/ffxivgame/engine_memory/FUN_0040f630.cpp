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
// FUNCTION: ffxivgame 0x0000f630 — __thiscall: intrusive linked-list count
//                                   (27 bytes)
//
// Asm (27 bytes @ orig RVA 0x0000f630):
//   8b 51 04    MOV EDX, dword ptr [ECX + 0x4]       ; EDX = this->field_0x4 (inner ptr)
//   8b 4a 4c    MOV ECX, dword ptr [EDX + 0x4c]      ; ECX = inner->sentinel.next (list head)
//   83 c2 44    ADD EDX, 0x44                         ; EDX = &inner->sentinel
//   33 c0       XOR EAX, EAX                          ; count = 0
//   3b ca       CMP ECX, EDX                          ; head == &sentinel?
//   74 0b       JZ  0x0040f64a                        ; yes → empty list, return 0
//   90          NOP
//   8b 49 08    MOV ECX, dword ptr [ECX + 0x8]        ; ECX = node->next
//   83 c0 01    ADD EAX, 0x1                          ; count++
//   3b ca       CMP ECX, EDX                          ; node == &sentinel?
//   75 f6       JNZ 0x0040f640                        ; no → loop
//   c3          RET
//
// Counts nodes in a sentinel-based intrusive linked list. The sentinel
// is embedded at inner+0x44; inner->sentinel.next (at inner+0x4c) is
// the first real node (or points back to sentinel if empty). Each node's
// next pointer is at node+0x8.

struct ListNode {
    char _pad[0x8];
    ListNode *next;
};

struct Inner {
    char _pad1[0x44];
    ListNode sentinel;
};

struct Outer {
    char _pad[0x4];
    Inner *field_0x4;

    int FUN_0040f630();
};

int Outer::FUN_0040f630() {
    Inner *p = field_0x4;
    ListNode *node = p->sentinel.next;
    int count = 0;
    if (node != &p->sentinel) {
        do {
            node = node->next;
            count++;
        } while (node != &p->sentinel);
    }
    return count;
}
