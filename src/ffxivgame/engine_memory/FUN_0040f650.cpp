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
// FUNCTION: ffxivgame 0x0000f650 — __thiscall: count linked-list nodes
//                                   across two circular linked lists
//                                   (90 bytes / 0x5a)
//
// Walks two circular intrusive linked lists (nodes linked via offset +8)
// embedded in this->m_inner.  For each node in the secondary list, calls
// vtable[1] on it (__thiscall, no stack args) to obtain an InnerObj
// pointer, then counts nodes in that object's primary list as well.
// Returns the total node count.
//
// Calling convention: __thiscall (member function); plain RET.
// Callee-saves pushed: EBX (this), ESI, EDI (count).

struct Link {
    int   _a;
    int   _b;
    Link *next;     // at +8
};

// sizeof(Link) == 12 == 0x0c

struct InnerObj {
    char _pad0[0x38];
    Link list1;     // primary-list sentinel   — at +0x38; .next at +0x40
    Link list2;     // secondary-list sentinel — at +0x44; .next at +0x4c
};

struct Item {
    virtual void       dummy() = 0;       // vtable[0]
    virtual InnerObj  *getInner() = 0;    // vtable[1]
    int   _pad;
    Item *next;                           // at +8
};

class OuterObj {
public:
    int       _pad0;
    InnerObj *m_inner;   // at +4

    int countNodes();
};

int OuterObj::countNodes()
{
    InnerObj *inner = this->m_inner;
    int count = 0;

    // --- Walk primary circular linked list ---
    Link *node1     = inner->list1.next;
    Link *sentinel1 = &inner->list1;
    while (node1 != sentinel1) {
        node1 = node1->next;
        count++;
    }

    // --- Walk secondary list; for each item count its inner list nodes ---
    Item *item = reinterpret_cast<Item *>(inner->list2.next);
    Item *end2 = reinterpret_cast<Item *>(&inner->list2);

    if (item != end2) {
        do {
            InnerObj *obj = item->getInner();

            // Count primary-list nodes of obj.
            Link *s = &obj->list1;
            Link *p = s->next;
            while (p != s) {
                p = p->next;
                count++;
            }

            // Advance; re-read sentinel from this->m_inner each iteration.
            item = item->next;
            end2 = reinterpret_cast<Item *>(&this->m_inner->list2);
        } while (item != end2);
    }

    return count;
}
