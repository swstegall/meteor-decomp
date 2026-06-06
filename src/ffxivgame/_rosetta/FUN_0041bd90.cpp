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
// FUNCTION: ffxivgame 0x0001bd90 — __cdecl 4-arg rect-builder that packs
//                                  {x, y, w, h} into {x1,y1,x2,y2} on the
//                                  stack and dispatches to FUN_00423190 on
//                                  the global object at [0x0132987c] (57 B).
//
// Frame (no callee-saved registers; SUB ESP,0x10 only):
//   [ESP+0x00]  int x1  = arg1
//   [ESP+0x04]  int y1  = arg2
//   [ESP+0x08]  int x2  = arg1 + arg3
//   [ESP+0x0c]  int y2  = arg2 + arg4
//
// Calling conventions:
//   This function  : __cdecl (plain RET; 4 DWORD stack args, caller cleans)
//   FUN_00423190   : __thiscall (ECX = this; RET 4 cleans 1 pushed arg)
//   Global [0x0132987c] : pointer to the receiver object
//
// Both relocatable sites (abs32 MOV ECX,[global] + rel32 CALL) are masked
// by tools/compare.py, so the 4-byte addresses at those positions are
// treated as wildcards in the byte diff.

struct _Host_0132987c_1bd90 {
    void FUN_00423190(void* rect);
};

extern _Host_0132987c_1bd90* g_host_0132987c_1bd90;

struct _Rect4i_1bd90 { int x1, y1, x2, y2; };

extern "C" void __cdecl FUN_0041bd90(int x, int y, int w, int h)
{
    _Rect4i_1bd90 r;
    r.x1 = x;
    r.y1 = y;
    r.x2 = x + w;
    r.y2 = y + h;
    g_host_0132987c_1bd90->FUN_00423190(&r);
}
