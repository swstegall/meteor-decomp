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
// FUNCTION: ffxivgame 0x00043ea0 — `__thiscall` grid-cell state accessor
//                                  (38 B / 0x26, 2 stack args, RET 0x8).
//
// Asm shape (read from orig RVA 0x00043ea0):
//
//   __thiscall int FUN_00443ea0(Owner *this,   // ECX
//                                int y,         // [esp+4]
//                                int x)         // [esp+8]
//   {
//       // Same 32-wide, 0xBC-byte-stride grid layout as the sibling
//       // mutator FUN_00443cf0 (this->cells lives at this+0x8): the
//       // cell index is (y << 5) + (x & 0x1f).
//       Cell *cell = (Cell *)((char *)this->cells
//                             + ((y << 5) + (x & 0x1f)) * 0xBC);
//
//       if (cell == 0)
//           return -1;
//       return cell->state;   // mov eax, [eax]
//   }
//
// Register notes: no callee-saves, no frame — a pure /Oy leaf function.
// EAX carries a1<<5, EDX carries a2&0x1f, the sum is scaled by IMUL and
// added to *[ecx+8] (this->cells) to land directly on the target cell's
// address; the null check happens to fall out of the ADD's flags (ZF)
// rather than a separate CMP/TEST.
//
// No externals touched — this is a pure leaf, source-level reproduction.

struct Cell {
    int state;   // offset 0x0
};

class Owner {
public:
    int GetCellState(int y, int x);

private:
    void *unk_0;
    void *unk_4;
    char *cells;   // offset 0x8 — base pointer, stride sizeof(Cell) == 0xBC
};

int Owner::GetCellState(int y, int x)
{
    Cell *cell = (Cell *)(cells + ((y << 5) + (x & 0x1f)) * 0xBC);

    if (cell == 0) {
        return -1;
    }

    return cell->state;
}
