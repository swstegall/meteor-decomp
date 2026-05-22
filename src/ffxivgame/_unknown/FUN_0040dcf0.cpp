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
// FUNCTION: ffxivgame 0x0000dcf0 — __thiscall reverse-index initializer
//                                   (64 B / 0x40)
//
// __thiscall void FUN_0040dcf0(dcf0_Outer *this)
//   ECX : this
//   No stack args; RET 0.
//
// Initializes a reverse-index (descending) word array in a sub-object
// reachable via this->m_inner (offset +4).  The inner object layout:
//   inner + 0x04  : unsigned short *m_array   (allocated word buffer)
//   inner + 0x08  : unsigned short  m_count
//   inner + 0x0a  : unsigned short  m_used
//
// The function fills m_array[0..m_count-1] with m_count-1, m_count-2,
// ..., 0 (descending), then copies m_count to m_used.
//
// Asm (64 bytes, RVA 0xdcf0–0xdd2f):
//   dcf0: 8b 41 04              MOV EAX, [ECX+4]        ; EAX = inner
//   dcf3: 56                    PUSH ESI
//   dcf4: 57                    PUSH EDI
//   dcf5: 0f b7 78 08           MOVZX EDI, word [EAX+8] ; EDI = m_count
//   dcf9: 8d 57 ff              LEA EDX, [EDI-1]        ; EDX = m_count-1
//   dcfc: 0f b7 f2              MOVZX ESI, DX           ; ESI = (uint16)(m_count-1)
//   dcff: 33 d2                 XOR EDX, EDX            ; i = 0
//   dd01: 66 85 ff              TEST DI, DI
//   dd04: 76 1c                 JBE +0x1c  (skip loop)
//   dd06: 8b 40 04              MOV EAX, [EAX+4]        ; EAX = m_array
//   dd09: 0f b7 fa              MOVZX EDI, DX           ; EDI = (uint16)i
//   dd0c: 66 89 34 78           MOV [EAX+EDI*2], SI     ; m_array[i] = val
//   dd10: 8b 41 04              MOV EAX, [ECX+4]        ; reload inner
//   dd13: 83 c2 01              ADD EDX, 1              ; i++
//   dd16: 81 c6 ff ff 00 00     ADD ESI, 0xffff         ; val-- (16-bit decrement)
//   dd1c: 66 3b 50 08           CMP DX, [EAX+8]         ; i < m_count?
//   dd20: 72 e4                 JC  dd06                ; loop
//   dd22: 8b 49 04              MOV ECX, [ECX+4]        ; ECX = inner
//   dd25: 66 8b 51 08           MOV DX, [ECX+8]         ; DX = m_count
//   dd29: 5f                    POP EDI
//   dd2a: 66 89 51 0a           MOV [ECX+0xa], DX       ; m_used = m_count
//   dd2e: 5e                    POP ESI
//   dd2f: c3                    RET

struct dcf0_Inner {
    char          _pad0[4];        // +0x00
    unsigned short *m_array;       // +0x04
    unsigned short  m_count;       // +0x08
    unsigned short  m_used;        // +0x0a
};

struct dcf0_Outer {
    char          _pad0[4];        // +0x00
    dcf0_Inner   *m_inner;         // +0x04

    void FUN_0040dcf0();
};

void dcf0_Outer::FUN_0040dcf0()
{
    dcf0_Inner *inner = m_inner;
    unsigned short n = inner->m_count;
    unsigned short val = (unsigned short)(n - 1);
    unsigned short i = 0;
    if (i < n) {
        do {
            inner->m_array[i] = val;
            inner = m_inner;
            i++;
            val = (unsigned short)(val + (unsigned short)0xffff);
        } while (i < inner->m_count);
    }
    inner = m_inner;
    inner->m_used = inner->m_count;
}

// vim: ts=4 sts=4 sw=4 et
