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
// FUNCTION: ffxivgame 0x0000d600 — linked-list search by predicate
//                                   (__thiscall, 58 B / 0x3a)
//
// __thiscall Node *find_node(this, Node *target)
//   ECX        : this  — pointer to the head pointer (Node **)
//   [ESP+0x04] : Node *target — node to search for
//
// Walks the singly-linked list rooted at this->head (offset +0x9a4 next).
// For each node, calls node->matches(target) (FUN_0040ddd0 — __thiscall,
// one stack arg). Returns the first matching node, or NULL if none found.
// Also returns NULL immediately if target is NULL.
//
// Calling convention: __thiscall, callee cleans 1 stack arg (RET 0x4).
// Callee-saved: EDI, ESI.

struct Node {
    char _pad[0x9a4];
    Node *next;
};

class NodeList {
public:
    Node *head;
    Node *find_node(Node *target);
};

// FUN_0040ddd0 — __thiscall bool matches(this, Node *target)
class NodeMatcher {
public:
    bool matches(Node *target);
};

Node *NodeList::find_node(Node *target)
{
    if (target == 0)
        return 0;
    Node *cur = head;
    while (cur != 0) {
        if (((NodeMatcher *)cur)->matches(target))
            return cur;
        cur = cur->next;
    }
    return 0;
}
