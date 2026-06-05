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
// FUNCTION: ffxivgame 0x0005e6c0 — __cdecl bounds-checked table lookup (20 B / 0x14).
//
// Behaviour read from the disassembly at orig RVA 0x0005e6c0:
//
//   __cdecl int FUN_0045e6c0(unsigned int index)
//
//   Loads `index` from [ESP+4] into EAX, compares against 0x1e (30
//   decimal) as an unsigned quantity, and if unsigned-above jumps to a
//   zero-return (XOR EAX,EAX / RET). Otherwise it reads a DWORD from the
//   static table at absolute VA 0x00f69280 (RVA 0x00B69280) indexed by
//   EAX*4 via the SIB form [EAX*4 + disp32], and returns that value.
//
//   Calling convention: __cdecl (plain RET c3 epilogue; caller cleans
//   the single 4-byte argument slot).
//   Return type: int (table DWORD on hit, 0 on out-of-range).
//   No prologue, no callee-saved registers, no stack frame.
//
//   Asm shape (20 bytes total):
//
//     8b 44 24 04              MOV  EAX, dword ptr [ESP + 0x4]
//     83 f8 1e                 CMP  EAX, 0x1e
//     77 08                    JA   +8  (→ XOR EAX,EAX / RET)
//     8b 04 85 <disp32>        MOV  EAX, dword ptr [EAX*4 + g_table_f69280]
//     c3                       RET
//     33 c0                    XOR  EAX, EAX
//     c3                       RET
//
//   Reloc-bearing site: the disp32 in the SIB MOV (bytes +9 .. +12 of
//   the function body) is an absolute VA; tools/compare.py masks those
//   4 bytes during the byte diff.

extern "C" int g_table_f69280[];

extern "C" int __cdecl FUN_0045e6c0(unsigned int index)
{
    if (index <= 0x1e)
        return g_table_f69280[index];
    return 0;
}
