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
// FUNCTION: ffxivgame 0x9f1773 — conditional byte-field copy (27 B)
//
// __thiscall char* C::copy_field(char* pOut, int flag)
//   ECX       : this
//   [ESP+4]   : pOut  — output pointer (checked for null)
//   [ESP+8]   : flag  — enable flag (checked for zero)
//
// If pOut is null OR flag is 0, returns null.
// Otherwise copies the byte at this+8 into *pOut and returns pOut.
//
// This is a neighbour of FUN_009f176f (the plain getter for the same
// byte field) in the MSVC internal name-demangling cluster.
// Class layout matches FUN_009f176f: char padding[8] + char field @ +8.

class C {
    char padding[8];
    char field;
public:
    char* copy_field(char* pOut, int flag);
};

#pragma optimize("s", on)
char* C::copy_field(char* pOut, int flag) {
    if (pOut && flag) {
        *pOut = field;
        return pOut;
    }
    return 0;
}
#pragma optimize("", on)
