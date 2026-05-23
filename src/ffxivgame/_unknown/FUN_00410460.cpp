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
// FUNCTION: ffxivgame 0x00010460 — linked-list iterator dispatching two
//                                   virtual calls per node (60 B / 0x3c)
//
// __thiscall void Container::FUN_00410460(void *param_1)
//   ECX        : this
//   [ESP+0x04] : param_1 — opaque argument forwarded to inner virtual call
//
// Object layout (inferred):
//   [this + 0x04]  void*      field_04    — passed by address as first arg to inner call
//   [this + 0x38]  Node       m_sentinel  — inline sentinel node (12-byte Node struct)
//     [this+0x38+0x00]  void** vftable
//     [this+0x38+0x04]  int    _pad04
//     [this+0x38+0x08]  Node*  next       — = [this+0x40]; points to first real node or self
//
// Node layout:
//   [node + 0x00]  void**     vftable     — vtable pointer
//   [node + 0x04]  int        _pad04      — unused in this function
//   [node + 0x08]  Node*      next        — forward link
//
// Vtable slot 0 of Node (offset +0):  destructor or pure virtual
// Vtable slot 1 of Node (offset +4):
//   __thiscall Result* Node::get_result()   (ECX = node, no stack args)
//   Returns a Result* in EAX.
//
// Result struct fields:
//   [result + 0x0c]  Obj*   obj   — object with its own vtable
//   [result + 0x10]  void*  arg   — forwarded as second stack arg
//
// Vtable slot 0 of Obj (offset +0):  destructor or pure virtual
// Vtable slot 1 of Obj (offset +4):
//   __thiscall void Obj::notify(void *arg_a, void *arg_b, void *param_1)
//   Called as: obj->notify(&this->field_04, result->arg, param_1)
//
// Behaviour:
//   Load head = m_sentinel.next (= [this+0x40]).  If head == &m_sentinel,
//   the list is empty; return immediately.  Otherwise iterate:
//     1. Call head->get_result() via vtable[1]  → result*
//     2. Call result->obj->notify() via vtable[1] with (&field_04, result->arg, param_1)
//     3. Advance head = head->next  ([head+8])
//   until head == &m_sentinel.
//
// Calling convention: __thiscall; callee pops 1 stack arg (RET 0x4).
//
// Register allocation (MSVC 2005 /O2):
//   ESI = node (the walking pointer)
//   EDI = &m_sentinel (sentinel / loop bound)
//   EBX = &this->field_04  (first arg to inner call)
//   EBP = param_1  (third arg to inner call)
//   EBX/EBP saves are deferred until after the empty-list check.

struct Result;

// Obj: virtual notify() at vtable slot 1
struct Obj {
    virtual ~Obj() {}
    virtual void notify(void *arg_a, void *arg_b, void *param_1);
};

// Result: fields at +0x0c and +0x10
struct Result {
    char _pad[0x0c];
    Obj *obj;      // +0x0c
    void *arg;     // +0x10
};

// Node: virtual get_result() at vtable slot 1; next ptr at offset +8
// Layout: [+0x00] vftable (implicit), [+0x04] m_pad04, [+0x08] next
struct Node {
    virtual ~Node() {}
    virtual Result *get_result();
    int   m_pad04;  // +0x04 — padding between vftable and next
    Node *next;     // +0x08
};

// Container: sentinel at +0x38; field_04 at +0x04
struct Container {
    int   _pad00;        // +0x00
    void *field_04;      // +0x04
    char  _pad08[0x30];  // +0x08..+0x37
    Node  m_sentinel;    // +0x38 (12 bytes: vftable@0, _pad04 implicit from C++ obj, next@8)
                         //   m_sentinel.next = [this+0x40]

    void FUN_00410460(void *param_1);
};

void Container::FUN_00410460(void *param_1)
{
    Node *node = m_sentinel.next;
    Node *sentinel = &m_sentinel;
    if (node == sentinel)
        return;
    void *arg_a = &field_04;
    do {
        Result *res = node->get_result();
        res->obj->notify(arg_a, res->arg, param_1);
        node = node->next;
    } while (node != sentinel);
}
