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
// FUNCTION: ffxivgame 0x00013570 — count linked-list nodes across a primary
//           list and a secondary list-of-lists (93 B).
//
// Byte-for-byte duplicate of FUN_004114f0 (same source, second template
// instantiation by the build).  See engine_memory/FUN_004114f0.cpp for the
// full annotated commentary — every byte is identical, register allocation
// is identical, and the loop-head NOP-padding (`eb 03 8d 49 00`) is the
// same MSVC-emitted 16-byte alignment trick at the inner-inner loop head.
//
// __thiscall int FUN_00413570(this)
//   ECX : this — owner object; [this+4] points to the data block
//
// Behaviour:
//   1. Load data = [this+4].
//   2. Count nodes in data's primary linked list:
//        head     = data->inner_sentinel.next  [data+0x40]
//        sentinel = &data->inner_sentinel       data+0x38
//        next ptr = node->next                  [node+8]
//   3. Walk data's outer list (head = outer_sentinel.next at data+0x4c,
//      sentinel = &data->outer_sentinel at data+0x44).
//      For each outer node: call vtable slot 1 (vtable offset +4) to
//      obtain an inner DataBlock, then count its primary list the same way.
//   4. Return total count.
//
// Calling convention: __thiscall, no stack args (plain RET).
//
// Register allocation (MSVC 2005 /O2):
//   EBX = this        (callee-save; needed for outer-sentinel reload at loop tail)
//   EDI = count       (callee-save; accumulator)
//   ESI = outer node  (callee-save; loop cursor in outer do-while)
//   EDX = data        (= m_data pointer; preserved across first loop so outer
//                      loop head does not need to reload it from [EBX+4])
//   EAX, ECX = temporaries

struct ListNode {
    int       field0;   // [+0]
    int       field4;   // [+4]
    ListNode *next;     // [+8]
};

struct DataBlock {
    char      pad0[0x38];
    ListNode  inner_sentinel;  // [0x38..0x43]; .next at [0x40]
    ListNode  outer_sentinel;  // [0x44..0x4f]; .next at [0x4c]
};

struct IOuterNode {
    virtual ~IOuterNode() {}                // vtable slot 0 (vtable[+0])
    virtual DataBlock *GetInnerData() = 0; // vtable slot 1 (vtable[+4])
};

class CAlternativeManager {
public:
    int        field0;
    DataBlock *m_data;  // [+4]

    int FUN_00413570();
};

int CAlternativeManager::FUN_00413570()
{
    DataBlock *data = m_data;
    // Hoist head pointer to force EDX=data alive past first loop
    ListNode *node = data->inner_sentinel.next;
    int count = 0;

    // Count primary (inner) list nodes
    ListNode *sentinel = &data->inner_sentinel;
    while (node != sentinel) {
        node = node->next;
        count++;
    }

    // Walk outer list; outer sentinel reloaded from m_data each iteration
    ListNode *outerNode = data->outer_sentinel.next;
    ListNode *outerSentinel = reinterpret_cast<ListNode *>(&data->outer_sentinel);
    if (outerNode != outerSentinel) {
        do {
            IOuterNode *ref = reinterpret_cast<IOuterNode *>(outerNode);
            DataBlock *inner = ref->GetInnerData();
            ListNode *innerNode = inner->inner_sentinel.next;
            ListNode *innerSentinel = &inner->inner_sentinel;
            while (innerNode != innerSentinel) {
                innerNode = innerNode->next;
                count++;
            }
            outerNode = outerNode->next;
        } while (outerNode !=
                 reinterpret_cast<ListNode *>(&m_data->outer_sentinel));
    }

    return count;
}
