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
// FUNCTION: ffxivgame 0x00024780 — linked-list virtual dispatch (36 B / 0x24)
//
// Iterates over a singly-linked list rooted at the global pointer
// g_24780_head (0x01329958). For each node, calls virtual method #11
// (vtable offset 0x2c) with this=node and param_1 as the sole stack arg.
//
// Calling convention: __cdecl (plain RET, 1 arg, void return)
// Frame: none (/Oy); callee-saves ESI (current node) and EDI (param cache).
//   MSVC 2005 /O2 shrink-wraps the PUSH EDI: it is deferred until AFTER the
//   null-guard on ESI so a NULL head takes the fast path without touching EDI.
//
// Object layout of ListNode_24780 (8 bytes):
//   offset 0x00 : vtable pointer  (implicit, 4 B)
//   offset 0x04 : next pointer    (4 B)
//
// Vtable layout (slot -> offset -> function):
//   slot  0 -> 0x00 : vfunc00
//   slot  1 -> 0x04 : vfunc01
//   ...
//   slot 10 -> 0x28 : vfunc10
//   slot 11 -> 0x2c : Notify   ← the dispatched method

struct ListNode_24780 {
    ListNode_24780 *next;       // object offset 4
    virtual void vfunc00(void *);
    virtual void vfunc01(void *);
    virtual void vfunc02(void *);
    virtual void vfunc03(void *);
    virtual void vfunc04(void *);
    virtual void vfunc05(void *);
    virtual void vfunc06(void *);
    virtual void vfunc07(void *);
    virtual void vfunc08(void *);
    virtual void vfunc09(void *);
    virtual void vfunc10(void *);
    virtual void Notify(void *param); // vtable slot 11, offset 0x2c
};

extern "C" ListNode_24780 *g_24780_head; // [0x01329958]

void __cdecl FUN_00424780(void *param_1)
{
    ListNode_24780 *pCur = g_24780_head;
    if (pCur) {
        do {
            pCur->Notify(param_1);
            pCur = pCur->next;
        } while (pCur);
    }
}
