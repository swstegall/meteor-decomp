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
// FUNCTION: ffxivgame 0x0040d060 — thiscall method on outer object
//                                   (__thiscall, 77 B / 0x4d)
//
// int OuterObj_d060::FUN_0040d060(int param_1)
//   ECX        : this  — pointer to outer object
//   [ESP+0x04] : int   param_1
//
// Loads this->field_5c (a pointer to a sub-object), returns 0 if NULL.
// Calls FUN_0040da10 on the sub-object (thiscall), returns 0 if NULL result.
// Calls FUN_0040db50 then FUN_0040db60 on the result; if db60 returns
// non-zero, dispatches FUN_0040ba30(__cdecl) with this->field_60 and param_1.
// Returns the result of FUN_0040db60.
//
// Calling convention: __thiscall, callee cleans 1 stack arg (RET 0x4).

struct SubObj_da10 {
    SubObj_da10 *FUN_0040da10();
    void FUN_0040db50();
    int FUN_0040db60();
};

extern "C" int __cdecl FUN_0040ba30(int a, int b);

struct OuterObj_d060 {
    char pad[0x5c];
    SubObj_da10 *field_5c;  // +0x5c
    int field_60;            // +0x60

    int FUN_0040d060(int param_1);
};

int OuterObj_d060::FUN_0040d060(int param_1)
{
    SubObj_da10 *pSub = field_5c;
    if (pSub == 0) {
        return 0;
    }
    SubObj_da10 *pResult = pSub->FUN_0040da10();
    if (pResult == 0) {
        return 0;
    }
    pResult->FUN_0040db50();
    int iResult = pResult->FUN_0040db60();
    if (iResult != 0) {
        FUN_0040ba30(field_60, param_1);
    }
    return iResult;
}
