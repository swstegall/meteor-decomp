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
// FUNCTION: ffxivgame 0x0000edd0 — __thiscall setter: store 3 function-pointer
//                                   fields with per-field NULL defaults
//                                   (51 B / 0x33)
//
// void FUN_0040edd0_class::FUN_0040edd0(void *p1, void *p2, void *p3)
//   ECX        : this  — object with pointer fields at +0x18, +0x1c, +0x20
//   [ESP+0x04] : void* p1 — new value for field_18; if NULL → &FUN_0040ee10
//   [ESP+0x08] : void* p2 — new value for field_1c; if NULL → &FUN_0040ee20
//   [ESP+0x0c] : void* p3 — new value for field_20; if NULL → &FUN_0040ee30
//
// Behaviour:
//   Each of the three pairs follows the same pattern:
//     load param → if NULL substitute default address → store to member field.
//   No stack frame; no callee-saved registers used.
//
// Calling convention: __thiscall, callee cleans 3 stack args (RET 0xc).
// Frame: none (no PUSH EBP, no locals).
//
// Default addresses (reloc targets masked by compare.py):
//   0x40ee10, 0x40ee20, 0x40ee30 — default callback implementations;
//   not known as named functions in this binary's work-pool.

extern "C" void FUN_0040ee10(void);
extern "C" void FUN_0040ee20(void);
extern "C" void FUN_0040ee30(void);

struct FUN_0040edd0_class {
    char pad[0x18];
    void *field_18;  // +0x18
    void *field_1c;  // +0x1c
    void *field_20;  // +0x20

    void FUN_0040edd0(void *p1, void *p2, void *p3);
};

void FUN_0040edd0_class::FUN_0040edd0(void *p1, void *p2, void *p3)
{
    if (!p1) p1 = (void *)FUN_0040ee10;
    field_18 = p1;
    if (!p2) p2 = (void *)FUN_0040ee20;
    field_1c = p2;
    if (!p3) p3 = (void *)FUN_0040ee30;
    field_20 = p3;
}
