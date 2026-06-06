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
// FUNCTION: ffxivgame 0x0004d500 — FUN_0044d500 (__cdecl, 0xda bytes).
//
// Single pointer argument at [esp+0x18] (after the five callee-save
// pushes ECX/EBX/EBP/ESI/EDI). Returns a value in EAX via a plain `ret`
// (no callee stack cleanup → __cdecl).
//
// Body shape (read from orig/ffxivgame.exe @ file-offset 0x4d500,
// .text RVA == file-offset because the section's raw_pointer ==
// virtual_address == 0x1000):
//
//   * Walk i = 0..6 comparing the int arg against the table at
//     0x1266dc4 (stride 8). The eb 07 / `8d a4 24 00 00 00 00` pair is
//     the MSVC 7-byte LEA-ESP alignment NOP padding the loop head.
//   * When arg <= table[i], call the IAT thunk cached in EBP
//     (dword [0xf3e1a4]) twice against the two `.data` arg arrays at
//     0x132cf04 and 0x132cf20 (atoi-style parse), do CDQ/IDIV modular
//     arithmetic against the per-i divisor table at 0x1266dc8, and read
//     a flag out of 0x132cecc[...]; if non-zero, return that flag.
//   * Otherwise fall through to the helper at 0x009d04ac with
//     [esp+0x18]+4, then post-process its result (the
//     MOVZX/SHL-8/OR/store tail) and return.
//
// Reloc-bearing sites baked as raw immediates (compare.py masks them,
// the verbatim bytes match orig regardless):
//   +0x2a   DIR32 → 0x00f3e1a4  (IAT thunk loaded into EBP)
//   several DIR32 → .data tables (0x1266dc4/dc8, 0x132cf04/cf20/cecc)
//   +0xb1   REL32 → 0x009d04ac  (direct helper call)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough, the
// established local idiom (see FUN_00408910 / FUN_00404e40). The mix of
// an IAT call cached in EBP, two CDQ/IDIV reductions, the table-indexed
// flag fetch, and the MSVC loop-head alignment NOP make a source-level
// /O2 rewrite shift bytes; emitting the 0xda orig bytes verbatim yields
// a .text of exactly 218 bytes that tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0044d500() {
    __asm {
        _emit 0x51
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x57
        _emit 0x33
        _emit 0xf6
        _emit 0xeb
        _emit 0x07
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x3b
        _emit 0x3c
        _emit 0xf5
        _emit 0xc4
        _emit 0x6d
        _emit 0x26
        _emit 0x01
        _emit 0x7e
        _emit 0x0d
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        _emit 0x83
        _emit 0xfe
        _emit 0x07
        _emit 0x7c
        _emit 0xeb
        _emit 0xe9
        _emit 0x83
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x2d
        _emit 0xa4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0x8d
        _emit 0x1c
        _emit 0xb5
        _emit 0x04
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x53
        _emit 0xff
        _emit 0xd5
        _emit 0x8b
        _emit 0xf8
        _emit 0x6a
        _emit 0x00
        _emit 0x8d
        _emit 0x04
        _emit 0xb5
        _emit 0x20
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x50
        _emit 0xff
        _emit 0xd5
        _emit 0x8b
        _emit 0x0c
        _emit 0xf5
        _emit 0xc8
        _emit 0x6d
        _emit 0x26
        _emit 0x01
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0xc7
        _emit 0x99
        _emit 0xf7
        _emit 0xf9
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0xfa
        _emit 0x99
        _emit 0xf7
        _emit 0xf9
        _emit 0x3b
        _emit 0xfa
        _emit 0x7c
        _emit 0x06
        _emit 0x2b
        _emit 0xcf
        _emit 0x03
        _emit 0xca
        _emit 0xeb
        _emit 0x04
        _emit 0x2b
        _emit 0xd7
        _emit 0x8b
        _emit 0xca
        _emit 0x83
        _emit 0xf9
        _emit 0x64
        _emit 0x7c
        _emit 0x33
        _emit 0x6a
        _emit 0x01
        _emit 0x53
        _emit 0xff
        _emit 0xd5
        _emit 0x8b
        _emit 0xf8
        _emit 0x8b
        _emit 0x04
        _emit 0xf5
        _emit 0xc8
        _emit 0x6d
        _emit 0x26
        _emit 0x01
        _emit 0x8d
        _emit 0x0c
        _emit 0x00
        _emit 0x3b
        _emit 0xf9
        _emit 0x75
        _emit 0x06
        _emit 0xf7
        _emit 0xd8
        _emit 0x50
        _emit 0x53
        _emit 0xff
        _emit 0xd5
        _emit 0x8b
        _emit 0xc7
        _emit 0x99
        _emit 0xf7
        _emit 0x3c
        _emit 0xf5
        _emit 0xc8
        _emit 0x6d
        _emit 0x26
        _emit 0x01
        _emit 0x8b
        _emit 0x04
        _emit 0xb5
        _emit 0xcc
        _emit 0xce
        _emit 0x32
        _emit 0x01
        _emit 0x8b
        _emit 0x14
        _emit 0x90
        _emit 0x85
        _emit 0xd2
        _emit 0x75
        _emit 0x30
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x8d
        _emit 0x4f
        _emit 0x04
        _emit 0x51
        _emit 0xe8
        _emit 0xf6
        _emit 0x2e
        _emit 0x58
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x06
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x59
        _emit 0xc3
        _emit 0xc6
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0xb6
        _emit 0x10
        _emit 0xc1
        _emit 0xe7
        _emit 0x08
        _emit 0x0b
        _emit 0xd7
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x89
        _emit 0x10
        _emit 0x83
        _emit 0xc0
        _emit 0x04
        _emit 0x5b
        _emit 0x59
        _emit 0xc3
        _emit 0x5f
    }
}
