// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SPDX-License-Identifier: AGPL-3.0-or-later

// FUNCTION: ffxivgame 0x004125a0 — count items across an intrusive linked-list
// hierarchy.  The owner object holds a pointer (at +4) to a container that
// embeds two doubly-linked list sentinels.  The first sentinel (at +0x34,
// head at +0x3c) holds "local" items; the second (at +0x40, head at +0x48)
// holds sub-allocator nodes, each of which exposes a virtual method at vtable
// slot 1 that returns a second container whose inner list is also counted.
//
// Calling convention: __thiscall, no stack args.  Callee-saves: EBX, ESI, EDI.
// Size note: Ghidra reports 0x5a (90) bytes but the actual function including
// its epilogue (POP ESI; POP EBX; RET) is 0x5d (93) bytes; the YAML size has
// been hand-corrected to 0x5d.

// Node for the intrusive doubly-linked list (next pointer at offset +8).
struct f125a0_Node {
    int         m_field0;   // +0
    int         m_field4;   // +4
    f125a0_Node*  m_next;   // +8
};

// Forward declaration.
struct f125a0_Container;

// A sub-allocator node that lives in the outer list.
// Virtual destructor at vtable[0]; getContainer() at vtable[1].
class f125a0_SubAlloc {
public:
    virtual ~f125a0_SubAlloc();
    virtual f125a0_Container* getContainer();

    int             m_field4;   // +4 (after vptr)
    f125a0_SubAlloc*  m_next;   // +8
};

// The container object pointed to by owner+4.
// Two list sentinels embedded inside, each 12 bytes (f125a0_Node).
struct f125a0_Container {
    char        m_pad[0x34];        // bytes 0x00..0x33
    f125a0_Node   m_listSentinel;   // at +0x34: head pointer at +0x3c
    f125a0_Node   m_outerSentinel;  // at +0x40: head pointer at +0x48
};

// The owning class.
class f125a0_Owner {
public:
    int              m_field0;      // +0
    f125a0_Container*  m_pContainer;  // +4

    int FUN_004125a0();
};

int f125a0_Owner::FUN_004125a0()
{
    f125a0_Container* inner = m_pContainer;
    int count = 0;

    for (f125a0_Node* n = inner->m_listSentinel.m_next;
         n != &inner->m_listSentinel;
         n = n->m_next)
        count++;

    f125a0_SubAlloc* it =
        reinterpret_cast<f125a0_SubAlloc*>(inner->m_outerSentinel.m_next);
    if (it != reinterpret_cast<f125a0_SubAlloc*>(&inner->m_outerSentinel)) {
        do {
            f125a0_Container* sub = it->getContainer();
            for (f125a0_Node* n = sub->m_listSentinel.m_next;
                 n != &sub->m_listSentinel;
                 n = n->m_next)
                count++;
            it = it->m_next;
        } while (it != reinterpret_cast<f125a0_SubAlloc*>(
                            &m_pContainer->m_outerSentinel));
    }

    return count;
}
