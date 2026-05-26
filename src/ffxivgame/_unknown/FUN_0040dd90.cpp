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
// FUNCTION: ffxivgame 0x0000dd90 — FUN_0040dd90 (61 B / 0x3d)
//
// __thiscall write-fields method. ECX = this; callee cleans 0x10 bytes.
// Loads sub = this->sub (pointer at ECX+4), writes four parameters into
// the sub-object's fields, calls FUN_0040dcf0 (ECX = this, no args), then
// zeros three trailing DWORDs in the sub-object.
//
// Sub-object field layout (inferred from write offsets):
//   struct Sub {
//     unsigned int f0;     // +0x00  ← p1 (DWORD)
//     unsigned int f4;     // +0x04  ← p2 (DWORD)
//     unsigned short f8;   // +0x08  ← p4 (WORD) — 4th param, lower offset
//     unsigned short fa;   // +0x0a  ← 0  (zeroed explicitly)
//     unsigned short fc;   // +0x0c  ← p3 (WORD) — 3rd param, higher offset
//                          // +0x0e  2-byte implicit padding for alignment
//     unsigned int f10;    // +0x10  ← 0
//     unsigned int f14;    // +0x14  ← 0
//     unsigned int f18;    // +0x18  ← 0
//   };
//
// Sibling FUN_0040dd50 is the constructor for the enclosing object:
//   sets vtable at [this+0], stores &this[2] (i.e. this+8) into [this+4]
//   (self-referential), then zeroes all sub-fields.
//   FUN_0040dd90 overwrites those zeroes with the supplied values.
//
// Calling convention: __thiscall; callee cleans 0x10 bytes.
//   p1 / p2 occupy 4-byte DWORD slots; p3 / p4 are unsigned short but
//   also occupy DWORD-aligned 4-byte slots on the stack (upper 2 bytes
//   unused by callee — accessed with 16-bit MOV AX / MOV DX).
//
// MSVC /O2 codegen notes:
//   * p1 → EAX and p2 → EDX are preloaded BEFORE PUSH ESI/EDI to avoid
//     the larger [ESP+0xC]/[ESP+0x10] addressing mode; the writes to the
//     sub-object then reuse EAX/EDX for p4/p3 (register recycling).
//   * EDI = 0 (XOR EDI,EDI) is shared across the explicit word-zero at
//     [sub+0xa] and the three DWORD-zero stores at [sub+0x10..0x18].
//   * The rel32 CALL to FUN_0040dcf0 is the only relocation site;
//     compare.py masks it (IMAGE_REL_I386_REL32 → 4-byte wildcard).

struct Dd90Sub {
    unsigned int f0;
    unsigned int f4;
    unsigned short f8;
    unsigned short fa;
    unsigned short fc;
    unsigned int f10;
    unsigned int f14;
    unsigned int f18;
};

class Cls_0040dd90 {
    void *vtable;
    Dd90Sub *sub;
public:
    void FUN_0040dcf0();
    void FUN_0040dd90(unsigned int p1, unsigned int p2,
                      unsigned short p3, unsigned short p4);
};

void Cls_0040dd90::FUN_0040dd90(unsigned int p1, unsigned int p2,
                                unsigned short p3, unsigned short p4)
{
    Dd90Sub *s = sub;
    s->f0 = p1;
    s->f4 = p2;
    s->f8 = p4;
    s->fa = 0;
    s->fc = p3;
    FUN_0040dcf0();
    s->f10 = 0;
    s->f14 = 0;
    s->f18 = 0;
}
