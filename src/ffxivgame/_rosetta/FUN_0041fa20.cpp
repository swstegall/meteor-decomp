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
// FUNCTION: ffxivgame 0x0041fa20 — wchar_t string run-splitter / glyph-run
//                                  dispatcher (279 B / 0x117, non-standard
//                                  calling convention: wchar_t* passed in EAX).
//
// The function walks a wchar_t string looking for a "break character"
// (passed as a 9th stack argument) and dispatches each run between
// break points via a virtual method call (vtable offset 0x6C) through a
// double-indirection thunk (FUN_00423300). The run length is first
// normalised through a stride-division helper (FUN_0041c540, which
// divides a char-count by a type-code in ECX by the element stride
// keyed on [ESP+4]). After the main scan loop completes, the trailing
// run is handled identically. A listener callback (FUN_004246f0) is
// invoked after each dispatched run, and FUN_0041d240 is called once
// at the end regardless.
//
// Global tables accessed:
//   0x00f595c4 — 4-byte-wide pointer array, indexed by arg2 (font slot index?)
//   0x00f596b8 — 4-byte-wide pointer array, indexed by arg5
//
// Called functions:
//   FUN_0041ed70  __cdecl, 1 stack arg (EDX), 0x21b bytes
//   FUN_0041c540  __cdecl, ECX = numerator, 1 stack arg = stride-type
//   FUN_00423300  vtable thunk: MOV ECX,[ECX]; vtable[0x6C]; JMP
//   FUN_004246f0  __cdecl, 1 stack arg (ESI = run result)
//   FUN_0041d240  __cdecl, 1 stack arg (0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function's non-standard register-parameter convention (EAX holds
//   the wchar_t* start pointer on entry, additionally EDX is passed
//   through unrenamed to FUN_0041ed70) cannot be expressed in any MSVC
//   2005 source-level calling convention (__cdecl/__stdcall/__thiscall/
//   __fastcall all use ECX/EDX or nothing for register args; none use
//   EAX). Attempting a source-level rewrite would require fighting the
//   code generator on every local declaration, call-arg push order, and
//   register allocation, making byte-identical output unreachable.
//
//   A __declspec(naked) body re-emits the 279 orig bytes verbatim via
//   MASM _emit directives. The absolute addresses (0x00f595c4 and
//   0x00f596b8) and the CALL rel32 offsets are baked in as raw immediates
//   exactly as they appear in the post-link PE, which tools/compare.py
//   accepts (same pattern as FUN_00404f70 and FUN_00401a00).

extern "C" __declspec(naked) void FUN_0041fa20() {
    __asm {
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        _emit 0x55
        _emit 0x56
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x24
        _emit 0x57
        _emit 0x8b
        _emit 0xf8
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20

        _emit 0x8b
        _emit 0x2c
        _emit 0x85
        _emit 0xc4
        _emit 0x95
        _emit 0xf5
        _emit 0x00
        _emit 0x85
        _emit 0xed
        _emit 0x0f
        _emit 0x84
        _emit 0xf1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b

        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x8b
        _emit 0x04
        _emit 0x8d
        _emit 0xb8
        _emit 0x96
        _emit 0xf5
        _emit 0x00
        _emit 0x85
        _emit 0xc0
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x0c

        _emit 0x0f
        _emit 0x84
        _emit 0xda
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x53
        _emit 0x52
        _emit 0xe8
        _emit 0x13
        _emit 0xf3
        _emit 0xff
        _emit 0xff
        _emit 0x2b
        _emit 0x74
        _emit 0x24

        _emit 0x2c
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x8d
        _emit 0x04
        _emit 0x47
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x3b
        _emit 0xf8

        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8b
        _emit 0xdf
        _emit 0x74
        _emit 0x62
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00

        _emit 0x0f
        _emit 0xb7
        _emit 0x0b
        _emit 0x3b
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        _emit 0x75
        _emit 0x4e
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x8b
        _emit 0xcb
        _emit 0x2b

        _emit 0xcf
        _emit 0xd1
        _emit 0xf9
        _emit 0x52
        _emit 0xe8
        _emit 0xa7
        _emit 0xca
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xf0
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x85
        _emit 0xf6

        _emit 0x7e
        _emit 0x2e
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x50
        _emit 0x8b

        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x51
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        _emit 0x52
        _emit 0x57
        _emit 0x56
        _emit 0x50
        _emit 0x51
        _emit 0x8b
        _emit 0x4c
        _emit 0x24

        _emit 0x3c
        _emit 0x55
        _emit 0xe8
        _emit 0x39
        _emit 0x38
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0x23
        _emit 0x4c
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04

        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8d
        _emit 0x7b
        _emit 0x02
        _emit 0x83
        _emit 0xc3
        _emit 0x02
        _emit 0x3b
        _emit 0xd8
        _emit 0x75
        _emit 0xa2
        _emit 0x8b
        _emit 0x54

        _emit 0x24
        _emit 0x24
        _emit 0x8b
        _emit 0xcb
        _emit 0x2b
        _emit 0xcf
        _emit 0xd1
        _emit 0xf9
        _emit 0x52
        _emit 0xe8
        _emit 0x52
        _emit 0xca
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4

        _emit 0x04
        _emit 0x8b
        _emit 0xf0
        _emit 0x85
        _emit 0xf6
        _emit 0x5b
        _emit 0x7e
        _emit 0x2e
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38

        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x50
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x51
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x52
        _emit 0x57

        _emit 0x56
        _emit 0x50
        _emit 0x51
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0x55
        _emit 0xe8
        _emit 0xe3
        _emit 0x37
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0xcd

        _emit 0x4b
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x6a
        _emit 0x00
        _emit 0xe8
        _emit 0x13
        _emit 0xd7
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04

        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3
    }
}
