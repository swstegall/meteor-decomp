// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SPDX-License-Identifier: AGPL-3.0-or-later

// FUNCTION: ffxivgame 0x00412600 — __thiscall, no stack args, 41 bytes
//
// Checks whether the intrusive list inside the container (pointed to by
// this->m_container at +4) is non-empty.  The sentinel node is embedded
// in the container at byte offset +0x20; its m_head field (the first
// element pointer) lives at +0x28 (= sentinel offset 0x20 + 8).
// When the list is non-empty, calls the first element's virtual method
// at vtable slot 1 and caches the int result in this->m_result (+0xc).
// Returns &this->m_field8 (+8) if the result is non-zero, else 0.
//
// Register layout (MSVC 2005, /O2):
//   ESI = this (callee-save; ECX clobbered by the virtual call)
//   EAX = container ptr, then sentinel addr (ADD EAX, 0x20)
//   ECX = head element ptr (sentinel.m_head), stays live through CALL
//   EDX = result (XOR EDX,EDX before branch; MOV EDX,EAX after call)
//
// Intrusive list pattern: identical to the outer-sentinel idiom in
// FUN_004125a0 — sentinel embedded in the container; iterator equality
// tested against &sentinel (not against a stored end pointer).

// Forward declaration.
struct f12600_SentNode;

// Element type stored in the intrusive list.  vtable at +0 (standard
// MSVC single-inheritance layout).  Destructor at slot 0; getValue()
// at slot 1 → compiled to CALL [EAX+4].
class f12600_Node {
public:
    virtual ~f12600_Node();
    virtual int getValue() = 0;
};

// Plain sentinel node embedded inside the container.  No vtable.
// Three 4-byte fields → 12 bytes total, so m_head falls at +8.
struct f12600_SentNode {
    int           m_a;     // +0  (container+0x20)
    int           m_b;     // +4  (container+0x24)
    f12600_Node*  m_head;  // +8  (container+0x28) — first element / end sentinel
};

// Container object pointed to by the outer object's field at +4.
struct f12600_Container {
    char            m_pad[0x20];   // bytes 0x00..0x1f
    f12600_SentNode m_sentinel;    // at +0x20 (12 bytes)
};

// The outer object whose method we are matching.
class f12600_Outer {
public:
    int                m_field0;      // +0
    f12600_Container*  m_container;   // +4
    int                m_field8;      // +8  (address returned on success)
    int                m_result;      // +0xc (cached call result)

    int FUN_00412600();
};

int f12600_Outer::FUN_00412600()
{
    f12600_Container* c = m_container;
    f12600_Node* head = c->m_sentinel.m_head;
    int result = 0;
    if (head != reinterpret_cast<f12600_Node*>(&c->m_sentinel)) {
        result = head->getValue();
    }
    m_result = result;
    if (result != 0) {
        return (int)&m_field8;
    }
    return 0;
}
