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
// FUNCTION: ffxivgame 0x0000d810 — walk linked list, accumulate stats
//                                   (__thiscall, 107 B / 0x6b)
//
// __thiscall void FUN_0040d810(NodeList3 *this, Stats *stats)
//   ECX        : this  — list head struct (head pointer at +0x00)
//   [ESP+0x04] : Stats *stats — accumulator struct (5 int fields at
//                [0],[4],[8],[c],[10])
//
// Walks the singly-linked list rooted at this->head (next ptr at node+0x9a4).
// For each node reads inner = *(int *)(node + 4), then:
//   - increments stats->total (always)
//   - if inner->a (short at +8) != inner->b (short at +0xa):
//       increments stats->with_diff
//       calls FUN_0040db50(node) [__thiscall]; if it returns 0, increments
//         stats->ret0_count
//       adds (unsigned short)(inner->a - inner->b) to stats->diff_sum
//   - adds (0x40 - (unsigned short)(inner->a - inner->b)) to
//     stats->capacity_rem
//
// Calling convention: __thiscall, callee cleans 1 stack arg (RET 0x4).
// Callee-saved: EDI (node), EBX (constant 1), ESI (stats).

struct Inner {
    char _pad[8];        // +0x00..+0x07
    short a;             // +0x08
    short b;             // +0x0a
};

struct Node3 {
    char _pad0[4];       // +0x00..+0x03
    Inner *inner;        // +0x04
    char _pad1[0x99a];   // +0x08..+0x9a3
    Node3 *next;         // +0x9a4
};

struct Stats {
    int total;           // +0x00
    int with_diff;       // +0x04
    int ret0_count;      // +0x08
    int diff_sum;        // +0x0c
    int capacity_rem;    // +0x10
};

class NodeList3 {
public:
    Node3 *head;
    void accumulate(Stats *stats);
};

// FUN_0040db50 — __thiscall bool check(this) on a Node3
class Node3Checker {
public:
    bool check();
};

void NodeList3::accumulate(Stats *stats)
{
    Node3 *cur = head;
    while (cur != 0) {
        stats->total += 1;
        if ((short)(cur->inner->a - cur->inner->b) != 0) {
            stats->with_diff += 1;
            bool r = ((Node3Checker *)cur)->check();
            if (!r) {
                stats->ret0_count += 1;
            }
            stats->diff_sum += (unsigned short)(cur->inner->a - cur->inner->b);
        }
        stats->capacity_rem += (int)(0x40 - (unsigned short)(cur->inner->a - cur->inner->b));
        cur = cur->next;
    }
}
