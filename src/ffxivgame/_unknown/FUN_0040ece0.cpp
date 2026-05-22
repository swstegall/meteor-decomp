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
// FUNCTION: ffxivgame 0x0040ece0 — assert-failure handler (__cdecl, 166 B)
//
// Formats a human-readable assertion failure message into a 2048-byte
// stack buffer using _snprintf_s, then outputs it via a function-pointer
// stored in an IAT slot, then deliberately crashes by writing 0 to address 0.
//
// Signature: void FUN_0040ece0(const char *msg, const char *expr,
//                              const char *file, int line,
//                              const char *tag);
//
// Parameters (left-to-right __cdecl, from stack analysis):
//   param_1 = msg   — assertion failure message string
//   param_2 = expr  — expression/condition that failed (or second string)
//   param_3 = file  — source file name
//   param_4 = line  — source line number
//   param_5 = tag   — optional component tag (NULL if no tag)
//
// Two format paths:
//   tag != NULL: "%s(%d):[%s] <assert> (%s) %s\n"  (file, line, tag, expr, msg)
//   tag == NULL: "%s(%d): <assert> (%s) %s\n"       (file, line, expr, msg)
//
// After formatting, calls (*g_output_fn)(buf, 6) to output the message,
// then writes 0 to address 0 to force a crash.
//
// Globals:
//   g_output_fn @ 0x012651b4 — function pointer: void(__cdecl*)(const char*, int)
//
// Reloc-bearing sites (masked by compare.py):
//   format string push for non-zero branch at +0x2f: 0xf54d14
//   format string push for zero branch at +0x6d: 0xf54cf8
//   IAT indirect call at +0x95: [0x012651b4]

#include <stdio.h>

// Declared as __declspec(dllimport) to force the `ff 15 [IAT]` indirect-call
// encoding rather than the load-into-register + `ff d0` sequence.
__declspec(dllimport) void __cdecl g_output_fn(const char *, int);

extern "C" void __cdecl FUN_0040ece0(const char *param_1, const char *param_2,
                                      const char *param_3, int param_4,
                                      const char *param_5)
{
    char local_800[0x800];

    if (param_5 != 0) {
        _snprintf_s(local_800, 0x800, 0x7ff,
                    "%s(%d):[%s] <assert> (%s) %s\n",
                    param_3, param_4, param_5, param_1, param_2);
    } else {
        _snprintf_s(local_800, 0x800, 0x7ff,
                    "%s(%d): <assert> (%s) %s\n",
                    param_3, param_4, param_1, param_2);
    }

    g_output_fn(local_800, 6);
    *(int *)0 = 0;
}
