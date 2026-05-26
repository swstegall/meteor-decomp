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
// FUNCTION: ffxivgame 0x0000ddd0 — range membership test (__thiscall, 52 B)
//
// __thiscall int FUN_0040ddd0(this, int param_1)
//   ECX : this  — pointer to outer struct (inner sub-struct pointer at +4)
//   [ESP+4] : param_1 — the value to test
//
// Returns 1 if param_1 lies within the range, 0 otherwise.
// The inner sub-struct at this->m_inner (offset +4 from this) holds:
//   [+0x00] int            base   — start of range
//   [+0x08] unsigned short count  — number of elements in range
//   [+0x0C] unsigned short stride — step size between elements
//
// Calling convention: __thiscall (ECX = this); RET 4 (one DWORD stack arg).
//
// Three-RET layout in orig binary:
//   RET1: param_1 == 0 → XOR AL,AL; RET 4   (early exit, short-form XOR)
//   RET2: in-range → MOV EAX,1; RET 4
//   RET3: out-of-range → XOR EAX,EAX; RET 4

struct ddd0_Inner {
    int            base;      // +0x00
    char           _pad4[4];
    unsigned short count;     // +0x08
    char           _padc[2];
    unsigned short stride;    // +0x0C
};

struct ddd0_Obj {
    char          _pad0[4];
    ddd0_Inner   *m_inner;    // +0x04

    int FUN_0040ddd0(int param_1);
};

int ddd0_Obj::FUN_0040ddd0(int param_1)
{
    if (param_1 == 0)
        return 0;
    ddd0_Inner *inner = this->m_inner;
    int idx = (param_1 - inner->base) / (int)inner->stride;
    if (idx >= 0 && idx < (int)inner->count)
        return 1;
    return 0;
}
