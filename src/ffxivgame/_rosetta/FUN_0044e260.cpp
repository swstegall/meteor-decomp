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
// FUNCTION: ffxivgame 0x0004e260 — array-walk calling FUN_00447450 on each 0x54-byte element
//                                   (__cdecl, 3 args, 38 B / 0x26)
//
// Iterates through a contiguous array of 0x54-byte structs [begin, end) and
// calls FUN_00447450 (__thiscall, one int arg) on each element.
//
// Register allocation with MSVC 2005 /O2:
//   ESI = p (current element pointer, starts at begin)
//   EDI = end
//   EBX = arg (hoisted out of loop, saved across the call)
//
// The initial CMP ESI,EDI / JZ guard comes before the callee-save of EBX,
// so EBX is only saved+loaded if the loop body executes at least once.
// This matches MSVC 2005's /Oy deferred-save optimisation for the
// outer-function frame.
//
// Calling convention: __cdecl — plain RET, caller cleans stack.
// Frame: none (/Oy — no locals, no EBP frame).

struct Item_e260 {
    char _unk[0x54];
    void FUN_00447450(int arg);
};

void __cdecl FUN_0044e260(Item_e260 *begin, Item_e260 *end, int arg)
{
    for (Item_e260 *p = begin; p != end; ++p)
    {
        p->FUN_00447450(arg);
    }
}
