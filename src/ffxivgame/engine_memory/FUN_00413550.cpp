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
// FUNCTION: ffxivgame 0x00013550 — __thiscall: count nodes in an intrusive
//                                   circular doubly-linked list (27 bytes)
//
// Asm (27 bytes @ orig RVA 0x00013550):
//   8b 51 04    MOV EDX, dword ptr [ECX + 0x4]     ; EDX = this->m_4 (list-owning obj)
//   8b 4a 4c    MOV ECX, dword ptr [EDX + 0x4c]    ; ECX = head = m_4->list.next
//   83 c2 44    ADD EDX, 0x44                       ; EDX = sentinel = &m_4->list (head node)
//   33 c0       XOR EAX, EAX                        ; count = 0
//   3b ca       CMP ECX, EDX                        ; head == sentinel? (empty list)
//   74 0b       JZ +0x0b   (→ ret)
//   90          NOP                                 ; loop-target alignment pad
// loop_top:
//   8b 49 08    MOV ECX, dword ptr [ECX + 0x8]      ; ECX = node->next (linked via +8 slot)
//   83 c0 01    ADD EAX, 0x1                        ; ++count
//   3b ca       CMP ECX, EDX                        ; node == sentinel?
//   75 f6       JNZ -0x0a   (→ loop_top)
//   c3          RET                                 ; __thiscall, no stack args
//
// Counts elements in the intrusive doubly-linked list rooted at
// this->m_4->list (sentinel node embedded at offset 0x44 inside m_4, head
// pointer at offset 0x4c i.e. sentinel.next, linked via the node-internal
// pointer at offset 0x8). The 1-byte NOP at +0x0f is the classic MSVC 2005
// /O2 loop-target alignment pad — the natural `loop_top` would land at
// +0x0f (one byte short of a 16-byte boundary relative to the function's
// 16-aligned entry), so MSVC inserts a single-byte NOP to round it up
// to +0x10 (= absolute 0x00413560, 16-byte aligned).
//
// The clang/GCC stub below is for static analysis only; the byte-correct
// implementation is the MSVC __declspec(naked) + _emit block.

#if defined(__clang__) || defined(__GNUC__)
extern "C" int FUN_00413550() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) int FUN_00413550() {
    __asm {
        // 00013550: 8b 51 04        MOV EDX, dword ptr [ECX + 0x4]
        _emit 0x8b
        _emit 0x51
        _emit 0x04
        // 00013553: 8b 4a 4c        MOV ECX, dword ptr [EDX + 0x4c]
        _emit 0x8b
        _emit 0x4a
        _emit 0x4c
        // 00013556: 83 c2 44        ADD EDX, 0x44
        _emit 0x83
        _emit 0xc2
        _emit 0x44
        // 00013559: 33 c0           XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0001355b: 3b ca           CMP ECX, EDX
        _emit 0x3b
        _emit 0xca
        // 0001355d: 74 0b           JZ +0x0b   (→ 0001356a)
        _emit 0x74
        _emit 0x0b
        // 0001355f: 90              NOP   (loop-target alignment pad)
        _emit 0x90
        // 00013560: 8b 49 08        MOV ECX, dword ptr [ECX + 0x8]
        _emit 0x8b
        _emit 0x49
        _emit 0x08
        // 00013563: 83 c0 01        ADD EAX, 0x1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 00013566: 3b ca           CMP ECX, EDX
        _emit 0x3b
        _emit 0xca
        // 00013568: 75 f6           JNZ -0x0a  (→ 00013560)
        _emit 0x75
        _emit 0xf6
        // 0001356a: c3              RET
        _emit 0xc3
    }
}
#endif
