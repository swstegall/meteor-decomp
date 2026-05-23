// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SPDX-License-Identifier: AGPL-3.0-or-later

// FUNCTION: ffxivgame 0x0040f650 — count items across an intrusive linked-list
// hierarchy.  The owner object holds a pointer (at +4) to a container that
// embeds two doubly-linked list sentinels.  The first sentinel (at +0x38,
// head at +0x40) holds "local" items; the second (at +0x44, head at +0x4c)
// holds sub-allocator nodes, each of which exposes a virtual method at vtable
// slot 1 that returns a second container whose inner list is also counted.
//
// Calling convention: __thiscall, no stack args.  Callee-saves: EBX, ESI, EDI.
// No stack frame.

// Node for the intrusive doubly-linked list (next pointer at offset +8).
struct f650_Node {
    int         m_field0;   // +0
    int         m_field4;   // +4
    f650_Node*  m_next;     // +8
};

// Forward declaration.
struct f650_Container;

// A sub-allocator node that lives in the outer list.
// Virtual destructor at vtable[0]; getContainer() at vtable[1].
class f650_SubAlloc {
public:
    virtual ~f650_SubAlloc();
    virtual f650_Container* getContainer();

    int             m_field4;   // +4 (after vptr)
    f650_SubAlloc*  m_next;     // +8
};

// The container object pointed to by owner+4.
// Two list sentinels embedded inside, each 12 bytes (f650_Node).
struct f650_Container {
    char        m_pad[0x38];        // bytes 0x00..0x37
    f650_Node   m_listSentinel;     // at +0x38: head pointer at +0x40
    f650_Node   m_outerSentinel;    // at +0x44: head pointer at +0x4c
};

// The owning class.
class f650_Owner {
public:
    int              m_field0;      // +0
    f650_Container*  m_pContainer;  // +4

    int FUN_0040f650();
};

int f650_Owner::FUN_0040f650()
{
    f650_Container* inner = m_pContainer;
    int count = 0;

    for (f650_Node* n = inner->m_listSentinel.m_next;
         n != &inner->m_listSentinel;
         n = n->m_next)
        count++;

    f650_SubAlloc* it =
        reinterpret_cast<f650_SubAlloc*>(inner->m_outerSentinel.m_next);
    if (it != reinterpret_cast<f650_SubAlloc*>(&inner->m_outerSentinel)) {
        do {
            f650_Container* sub = it->getContainer();
            for (f650_Node* n = sub->m_listSentinel.m_next;
                 n != &sub->m_listSentinel;
                 n = n->m_next)
                count++;
            it = it->m_next;
        } while (it != reinterpret_cast<f650_SubAlloc*>(
                            &m_pContainer->m_outerSentinel));
    }

    return count;
}
