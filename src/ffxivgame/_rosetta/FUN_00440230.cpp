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
// FUNCTION: ffxivgame 0x00040230 — __cdecl setter that stores a scalar
//           and five consecutive DWORD struct fields into six adjacent
//           globals at 0x0132ca24..0x0132ca38 (57 bytes / 0x39).
//
// Called by FUN_00401000, which forwards its own arg1 (scalar) and arg2
// (pointer to the 5-DWORD struct) verbatim.
//
// Frame: none — pure leaf; /Oy elides the EBP frame completely.
//        Parameters live at [ESP+4] (param1) and [ESP+8] (param2).
// CC:    __cdecl — bare RET, caller cleans the stack.
//
// Register allocation (MSVC 2005 /O2 pattern):
//   EAX   used for param1 (load + A3-form store to g_ca24), then reloaded
//         as the base pointer for param2, then reused for the final member
//         load + A3-form store to g_ca38.
//   ECX   scratch for param2->a and param2->c (89 0D store form).
//   EDX   scratch for param2->b and param2->d (89 15 store form).
//
// All six 4-byte global addresses are COFF DIR32 relocations in the .obj;
// compare.py masks them out of the byte diff so only the non-reloc opcode
// and register bytes need to match.

struct SomeFiveWordStruct {
    unsigned int a;   // offset 0x00
    unsigned int b;   // offset 0x04
    unsigned int c;   // offset 0x08
    unsigned int d;   // offset 0x0c
    unsigned int e;   // offset 0x10
};

extern "C" unsigned int DAT_0132ca24;
extern "C" unsigned int DAT_0132ca28;
extern "C" unsigned int DAT_0132ca2c;
extern "C" unsigned int DAT_0132ca30;
extern "C" unsigned int DAT_0132ca34;
extern "C" unsigned int DAT_0132ca38;

extern "C" void FUN_00440230(unsigned int param1, SomeFiveWordStruct* param2)
{
    DAT_0132ca24 = param1;
    DAT_0132ca28 = param2->a;
    DAT_0132ca2c = param2->b;
    DAT_0132ca30 = param2->c;
    DAT_0132ca34 = param2->d;
    DAT_0132ca38 = param2->e;
}
