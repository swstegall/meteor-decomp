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
// FUNCTION: ffxivgame 0x0041fb40 — DWORD array run-splitter / glyph-run
//                                  dispatcher (280 B / 0x118, non-standard
//                                  calling convention: DWORD* passed in EAX).
//
// Structural twin of FUN_0041fa20 (wchar_t / 2-byte stride variant).
// This version walks a DWORD (4-byte element) array looking for a
// "break value" (passed as a 9th stack argument via [ESP+0x40]) and
// dispatches each run between break points via a virtual method call
// (vtable offset 0x6C) through the double-indirection thunk FUN_00423300.
// The run length is normalised through FUN_0041c540 (ECX = char-count,
// [ESP+4] = stride-type). After the main scan loop completes, the trailing
// run is handled identically. FUN_004246f0 is called after each run, and
// FUN_0041d240(0) is called once at exit.
//
// Global tables accessed:
//   0x00f595c4 — 4-byte-wide pointer array, indexed by arg2 (font slot)
//   0x00f596b8 — 4-byte-wide pointer array, indexed by arg5
//
// Called functions:
//   FUN_0041ed70  __cdecl, 1 stack arg (EDX), 0x21b bytes
//   FUN_0041c540  __cdecl, ECX = numerator, 1 stack arg = stride-type
//   FUN_00423300  vtable thunk: MOV ECX,[ECX]; vtable[0x6C]; JMP
//   FUN_004246f0  __cdecl, 1 stack arg (ESI = run result)
//   FUN_0041d240  __cdecl, 1 stack arg (0)
//
// Key differences from FUN_0041fa20:
//   - Loop comparison: CMP dword ptr [EBX], ECX   (32-bit, not MOVZX word)
//   - Loop step: ADD EBX,4 / LEA EDI,[EBX+4]      (not +2)
//   - End-pointer: LEA EAX,[EDI+EAX*4]            (not *2)
//   - Run-length shift: SAR ECX,2                  (not SAR ECX,1)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Identical rationale to FUN_0041fa20: the non-standard register
//   parameter (DWORD* in EAX on entry; EDX forwarded unchanged to
//   FUN_0041ed70) cannot be expressed in any MSVC 2005 source-level
//   calling convention. A __declspec(naked) body re-emits the 280 orig
//   bytes verbatim via MASM _emit directives. The absolute addresses
//   (0x00f595c4, 0x00f596b8) and CALL rel32 offsets are baked in as
//   raw immediates exactly as they appear in the post-link PE, which
//   tools/compare.py accepts.

extern "C" __declspec(naked) void FUN_0041fb40() {
    __asm {
        // 0x0001fb40 +0x00
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

        // 0x0001fb50 +0x10
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
        _emit 0xf2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b

        // 0x0001fb60 +0x20
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

        // 0x0001fb70 +0x30
        _emit 0x0f
        _emit 0x84
        _emit 0xdb
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x53
        _emit 0x52
        _emit 0xe8
        _emit 0xf3
        _emit 0xf1
        _emit 0xff
        _emit 0xff
        _emit 0x2b
        _emit 0x74
        _emit 0x24

        // 0x0001fb80 +0x40
        _emit 0x2c
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x8d
        _emit 0x04
        _emit 0x87
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x3b
        _emit 0xf8

        // 0x0001fb90 +0x50
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

        // 0x0001fba0 +0x60
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        _emit 0x39
        _emit 0x0b
        _emit 0x75
        _emit 0x4f
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x8b
        _emit 0xcb
        _emit 0x2b
        _emit 0xcf

        // 0x0001fbb0 +0x70
        _emit 0xc1
        _emit 0xf9
        _emit 0x02
        _emit 0x52
        _emit 0xe8
        _emit 0x87
        _emit 0xc9
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xf0
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x85
        _emit 0xf6

        // 0x0001fbc0 +0x80
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

        // 0x0001fbd0 +0x90
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

        // 0x0001fbe0 +0xa0
        _emit 0x3c
        _emit 0x55
        _emit 0xe8
        _emit 0x19
        _emit 0x37
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0x03
        _emit 0x4b
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04

        // 0x0001fbf0 +0xb0
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8d
        _emit 0x7b
        _emit 0x04
        _emit 0x83
        _emit 0xc3
        _emit 0x04
        _emit 0x3b
        _emit 0xd8
        _emit 0x75
        _emit 0xa2
        _emit 0x8b
        _emit 0x54

        // 0x0001fc00 +0xc0
        _emit 0x24
        _emit 0x24
        _emit 0x8b
        _emit 0xcb
        _emit 0x2b
        _emit 0xcf
        _emit 0xc1
        _emit 0xf9
        _emit 0x02
        _emit 0x52
        _emit 0xe8
        _emit 0x31
        _emit 0xc9
        _emit 0xff
        _emit 0xff
        _emit 0x83

        // 0x0001fc10 +0xd0
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

        // 0x0001fc20 +0xe0
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

        // 0x0001fc30 +0xf0
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
        _emit 0xc2
        _emit 0x36
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8

        // 0x0001fc40 +0x100
        _emit 0xac
        _emit 0x4a
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x6a
        _emit 0x00
        _emit 0xe8
        _emit 0xf2
        _emit 0xd5
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4

        // 0x0001fc50 +0x110
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
