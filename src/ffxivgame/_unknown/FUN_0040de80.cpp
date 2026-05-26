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
// FUNCTION: ffxivgame 0x0000de80 — sample/quantize append (__thiscall, 55 B)
//
// __thiscall void FUN_0040de80(de80_Outer *this, int param_1)
//   ECX : this  — pointer to outer struct (inner sub-struct pointer at +4)
//   [ESP+4] : param_1 — the raw value to quantize and append
//
// If param_1 is zero, returns immediately (no-op).
// Otherwise:
//   1. Increments inner->m_sample_count (field14).
//   2. Quantizes param_1: idx = (param_1 - inner->m_base) / inner->m_stride
//   3. Appends the quantized value (as a short) to inner->m_data[inner->m_write_pos].
//   4. Increments inner->m_write_pos.
//
// Calling convention: __thiscall (ECX = this); RET 4 (one DWORD stack arg).
// Stack frame: none (no locals; EBX and ESI are callee-saved, push/pop
//              bracketed inside the non-trivial path only).
//
// Asm (55 bytes @ orig RVA 0x0000de80):
//   MOV EAX, [ESP+4]           ; param_1
//   TEST EAX, EAX
//   JZ  +0x2c                  ; if (param_1 == 0) return
//   MOV EDX, [ECX+4]           ; EDX = inner
//   ADD [EDX+0x14], 1          ; inner->m_sample_count++
//   PUSH EBX
//   PUSH ESI
//   MOV ESI, [ECX+4]           ; ESI = inner
//   MOVZX EBX, word [ESI+0xc]  ; EBX = inner->m_stride
//   MOV EDX, ESI               ; EDX = inner
//   SUB EAX, [EDX]             ; EAX = param_1 - inner->m_base
//   CDQ
//   IDIV EBX                   ; EAX = idx
//   MOVZX EDX, word [ESI+0xa]  ; EDX = inner->m_write_pos
//   MOV ESI, [ESI+4]           ; ESI = inner->m_data
//   MOV word [ESI+EDX*2], AX   ; inner->m_data[m_write_pos] = (short)idx
//   MOV ECX, [ECX+4]           ; ECX = inner
//   ADD word [ECX+0xa], 1      ; inner->m_write_pos++
//   POP ESI
//   POP EBX
//   RET 4

struct de80_Inner {
    int            m_base;          // +0x00
    short         *m_data;          // +0x04
    char           _pad8[2];        // +0x08
    unsigned short m_write_pos;     // +0x0a
    unsigned short m_stride;        // +0x0c
    char           _pade[6];        // +0x0e
    int            m_sample_count;  // +0x14
};

struct de80_Outer {
    char          _pad0[4];   // +0x00
    de80_Inner   *m_inner;    // +0x04

    void FUN_0040de80(int param_1);
};

void de80_Outer::FUN_0040de80(int param_1)
{
    if (param_1 != 0) {
        de80_Inner *inner = m_inner;
        inner->m_sample_count++;
        inner = m_inner;
        int stride = (int)(unsigned short)inner->m_stride;
        de80_Inner *base_ptr = inner;
        int diff = param_1 - base_ptr->m_base;
        short idx = (short)(diff / stride);
        unsigned short wpos = inner->m_write_pos;
        inner->m_data[wpos] = idx;
        inner = m_inner;
        inner->m_write_pos++;
    }
}
