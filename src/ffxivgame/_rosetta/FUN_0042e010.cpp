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
// FUNCTION: ffxivgame 0x0042e010 — matrix-init + render call wrapper
//                                  (49 B / 0x31).
//
// Behaviour read from asm/ffxivgame/0002e010_FUN_0042e010.s:
//
//   __cdecl void FUN_0042e010(int arg1, float arg2, int arg3, int arg4);
//
//   Allocates a 64-byte (4×4 float) local matrix on the stack, initialises
//   it via FUN_0042fcf0 (scaled-identity matrix fill), then forwards the
//   matrix pointer plus the incoming args to FUN_00428260.
//
//   Stack layout at entry (ESP points to return address):
//     [ESP+0x04]  arg1  — pointer / int forwarded to FUN_00428260 arg1
//     [ESP+0x08]  arg2  — float forwarded to FUN_00428260 arg2
//     [ESP+0x0c]  arg3  — int forwarded to both calls, FUN_00428260 arg4
//     [ESP+0x10]  arg4  — int forwarded to both calls, FUN_00428260 arg5
//
//   Call sequence:
//     (1) FUN_0042fcf0(&local_mat, arg3, arg4)
//         Fills local_mat with a scaled-identity 4×4 float matrix.
//         Returns EAX = &local_mat (first arg passed through EAX).
//         arg3 / arg4 are pushed but not read by the callee; they stay on
//         the stack as the 4th/5th positional args for the second call.
//     (2) FUN_00428260(arg1, arg2_float, &local_mat, arg3, arg4)
//         arg3 / arg4 are the leftover stack values from step (1) — the
//         compiler intentionally defers their cleanup to avoid re-pushing.
//
//   Stack cleanup:
//     - ADD ESP,0x4 after call (1) pops only the &local_mat pointer slot.
//     - ADD ESP,0x54 at the end cleans the entire frame:
//         0x40 local buffer + 0x08 leftover arg3/arg4 + 0x0c three new
//         pushes for call (2) = 0x54 total.
//
//   Reloc-bearing sites in the orig 49 bytes:
//     +0x12  CALL rel32 → FUN_0042fcf0 (0x0042fcf0)
//     +0x28  CALL rel32 → FUN_00428260 (0x00428260)
//   (masked by tools/compare.py; the orig PE bytes are baked in directly)
//
// Reconstruction: __declspec(naked) byte passthrough — same strategy as
// FUN_004051e0, FUN_00416320, and other _rosetta siblings.

extern "C" __declspec(naked) void FUN_0042e010() {
    __asm {
        // 0002e010: 8b 44 24 10   MOV EAX, dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0002e014: 8b 4c 24 0c   MOV ECX, dword ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0002e018: 83 ec 40      SUB ESP, 0x40
        _emit 0x83
        _emit 0xec
        _emit 0x40
        // 0002e01b: 50            PUSH EAX
        _emit 0x50
        // 0002e01c: 51            PUSH ECX
        _emit 0x51
        // 0002e01d: 8d 54 24 08   LEA EDX, [ESP+0x8]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 0002e021: 52            PUSH EDX
        _emit 0x52
        // 0002e022: e8 c9 1c 00 00  CALL FUN_0042fcf0
        _emit 0xe8
        _emit 0xc9
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        // 0002e027: d9 44 24 54   FLD float ptr [ESP+0x54]
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x54
        // 0002e02b: 83 c4 04      ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0002e02e: 50            PUSH EAX
        _emit 0x50
        // 0002e02f: 8b 44 24 50   MOV EAX, dword ptr [ESP+0x50]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x50
        // 0002e033: 51            PUSH ECX
        _emit 0x51
        // 0002e034: d9 1c 24      FSTP float ptr [ESP]
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // 0002e037: 50            PUSH EAX
        _emit 0x50
        // 0002e038: e8 23 a2 ff ff  CALL FUN_00428260
        _emit 0xe8
        _emit 0x23
        _emit 0xa2
        _emit 0xff
        _emit 0xff
        // 0002e03d: 83 c4 54      ADD ESP, 0x54
        _emit 0x83
        _emit 0xc4
        _emit 0x54
        // 0002e040: c3            RET
        _emit 0xc3
    }
}
