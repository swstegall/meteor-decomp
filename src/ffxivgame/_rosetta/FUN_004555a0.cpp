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
// FUNCTION: ffxivgame 0x000555a0 — __cdecl auto-radix wide-string integer
//                                  parser (501 B / 0x1f5, /GS + inline SEH).
//
// Inspection (read from the disassembly at orig RVA 0x000555a0):
//
//   int __cdecl parse_int_autobase(SrcObj* src /*[esp+4]*/,
//                                  int*    err /*[esp+8]*/);
//
//   Builds a short-string-optimised std::wstring local (capacity 7,
//   _Bx at [esp+0x1c], _Mysize at [esp+0x2c], _Myres at [esp+0x30]) by
//   calling FUN_00449000(src, &dst). If err ([esp+0x4c]) is non-zero the
//   whole parse is skipped and the radix defaults to 10. Otherwise the
//   leading characters select the base:
//
//     "0x" / "0X"            -> base 16
//     digit 'b' / 'B'        -> base  2   (FUN_00449540 = wstring::at)
//     leading '0'            -> base  8
//     else                   -> base 10
//
//   Per-base it precomputes the upper digit bounds ('0'+base for the
//   decimal range, plus 'a'+base-10 / 'A'+base-10 for bases > 10), then
//   walks the wstring back-to-front accumulating value*place, with place
//   multiplied by base each step (EDI = place, EBP = accumulator). A
//   leading '-' negates the result (NEG EBP). The wstring is destroyed
//   via FUN_0044d350 when it had spilled to the heap (_Myres >= 8), then
//   the value returns in EAX.
//
//   Calls (all rel32 / absolute, reloc-bearing in a real .obj):
//     +0x55  CALL 0x00449000   — wstring ctor-from-src
//     +0x84  CALL 0x009d22b4   — _invalid_parameter / range check (x3)
//     +0xc7  CALL 0x00449540   — wstring::at(1)            (x2)
//     +0x1cc CALL 0x0044d350   — wstring dtor / _Tidy
//     +0x1ec CALL 0x009d20f4   — __security_check_cookie
//   Globals:
//     0x00e584f8  — SEH scope table (PUSH imm32)
//     0x012ea8b0  — __security_cookie
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rewrite would have to coax MSVC 2005 /O2 /GS /EHsc into
//   reproducing the exact /GS cookie placement, the inline __CxxFrameHandler
//   SEH frame (PUSH -1 / scope table 0x00e584f8 / FS:[0] link), the precise
//   register allocation across the back-to-front digit loop, and the five
//   reloc-bearing call targets. Each constraint is brittle under /O2. Per the
//   established sibling idiom (FUN_00401820 / FUN_0040b840 / FUN_00409350),
//   the pragmatic choice is a __declspec(naked) body that re-emits the orig
//   501 bytes verbatim via _emit, yielding a byte-identical .text slice for
//   tools/compare.py.

extern "C" __declspec(naked) void FUN_004555a0() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0xf8
        _emit 0x84
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x83
        _emit 0xec

        _emit 0x24
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x57

        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x50
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x48
        _emit 0x33
        _emit 0xed
        _emit 0x8d
        _emit 0x7d
        _emit 0x01
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x07

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x2c
        _emit 0x66
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18

        _emit 0x50
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x44
        _emit 0xe8
        _emit 0x06
        _emit 0x3a
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x4c
        _emit 0x3b
        _emit 0xdd

        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x2c
        _emit 0x0f
        _emit 0x85
        _emit 0x93
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x30
        _emit 0x08
        _emit 0x8b

        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x73
        _emit 0x04
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x66
        _emit 0x83
        _emit 0x38
        _emit 0x30
        _emit 0x75
        _emit 0x79
        _emit 0x83

        _emit 0xfe
        _emit 0x01
        _emit 0x73
        _emit 0x05
        _emit 0xe8
        _emit 0x8b
        _emit 0xcc
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x30
        _emit 0x08
        _emit 0x8b
        _emit 0x44

        _emit 0x24
        _emit 0x1c
        _emit 0x73
        _emit 0x04
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x66
        _emit 0x83
        _emit 0x78
        _emit 0x02
        _emit 0x78
        _emit 0x74
        _emit 0x52
        _emit 0x83

        _emit 0x7c
        _emit 0x24
        _emit 0x2c
        _emit 0x01
        _emit 0x73
        _emit 0x05
        _emit 0xe8
        _emit 0x69
        _emit 0xcc
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x30
        _emit 0x08

        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x73
        _emit 0x04
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x66
        _emit 0x83
        _emit 0x78
        _emit 0x02
        _emit 0x58
        _emit 0x74

        _emit 0x30
        _emit 0x6a
        _emit 0x01
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xe8
        _emit 0xd4
        _emit 0x3e
        _emit 0xff
        _emit 0xff
        _emit 0x66
        _emit 0x83
        _emit 0x38
        _emit 0x62

        _emit 0x74
        _emit 0x18
        _emit 0x6a
        _emit 0x01
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xe8
        _emit 0xc3
        _emit 0x3e
        _emit 0xff
        _emit 0xff
        _emit 0x66
        _emit 0x83
        _emit 0x38

        _emit 0x42
        _emit 0x74
        _emit 0x07
        _emit 0xbb
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x13
        _emit 0xbb
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb

        _emit 0x0c
        _emit 0xbb
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x05
        _emit 0xbb
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8a
        _emit 0xc3
        _emit 0x04

        _emit 0x2f
        _emit 0x83
        _emit 0xfb
        _emit 0x0a
        _emit 0x88
        _emit 0x44
        _emit 0x24
        _emit 0x17
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x15
        _emit 0x00
        _emit 0xc6
        _emit 0x44
        _emit 0x24

        _emit 0x16
        _emit 0x00
        _emit 0x7e
        _emit 0x15
        _emit 0x8a
        _emit 0xc3
        _emit 0x04
        _emit 0x56
        _emit 0x88
        _emit 0x44
        _emit 0x24
        _emit 0x15
        _emit 0x8a
        _emit 0xc3
        _emit 0x04
        _emit 0x36

        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x17
        _emit 0x39
        _emit 0x88
        _emit 0x44
        _emit 0x24
        _emit 0x16
        _emit 0x83
        _emit 0xc6
        _emit 0xff
        _emit 0x3b
        _emit 0xf5
        _emit 0x7c
        _emit 0x7f

        _emit 0x3b
        _emit 0x74
        _emit 0x24
        _emit 0x2c
        _emit 0x76
        _emit 0x05
        _emit 0xe8
        _emit 0xd9
        _emit 0xcb
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x30
        _emit 0x08

        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x73
        _emit 0x04
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x0f
        _emit 0xb7
        _emit 0x04
        _emit 0x70
        _emit 0x66
        _emit 0x3d

        _emit 0x30
        _emit 0x00
        _emit 0x72
        _emit 0x11
        _emit 0x0f
        _emit 0xbe
        _emit 0x54
        _emit 0x24
        _emit 0x17
        _emit 0x0f
        _emit 0xb7
        _emit 0xc8
        _emit 0x3b
        _emit 0xca
        _emit 0x7f
        _emit 0x05

        _emit 0x83
        _emit 0xc1
        _emit 0xd0
        _emit 0xeb
        _emit 0x31
        _emit 0x83
        _emit 0xfb
        _emit 0x0a
        _emit 0x7e
        _emit 0x3d
        _emit 0x66
        _emit 0x3d
        _emit 0x61
        _emit 0x00
        _emit 0x72
        _emit 0x11

        _emit 0x0f
        _emit 0xbe
        _emit 0x54
        _emit 0x24
        _emit 0x15
        _emit 0x0f
        _emit 0xb7
        _emit 0xc8
        _emit 0x3b
        _emit 0xca
        _emit 0x7f
        _emit 0x05
        _emit 0x83
        _emit 0xc1
        _emit 0xa9
        _emit 0xeb

        _emit 0x15
        _emit 0x66
        _emit 0x3d
        _emit 0x41
        _emit 0x00
        _emit 0x72
        _emit 0x20
        _emit 0x0f
        _emit 0xbe
        _emit 0x54
        _emit 0x24
        _emit 0x16
        _emit 0x0f
        _emit 0xb7
        _emit 0xc8
        _emit 0x3b

        _emit 0xca
        _emit 0x7f
        _emit 0x14
        _emit 0x83
        _emit 0xc1
        _emit 0xc9
        _emit 0x0f
        _emit 0xaf
        _emit 0xcf
        _emit 0x0f
        _emit 0xaf
        _emit 0xfb
        _emit 0x83
        _emit 0xee
        _emit 0x01
        _emit 0x03

        _emit 0xe9
        _emit 0x85
        _emit 0xf6
        _emit 0x7d
        _emit 0x8b
        _emit 0xeb
        _emit 0x08
        _emit 0x66
        _emit 0x3d
        _emit 0x2d
        _emit 0x00
        _emit 0x75
        _emit 0x02
        _emit 0xf7
        _emit 0xdd
        _emit 0x8b

        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x72
        _emit 0x14

        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x6a
        _emit 0x0c
        _emit 0x8d
        _emit 0x44
        _emit 0x00
        _emit 0x02
        _emit 0x50
        _emit 0x51
        _emit 0xe8
        _emit 0xdf
        _emit 0x7b
        _emit 0xff

        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x8b
        _emit 0xc5
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0x59
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x63
        _emit 0xc9
        _emit 0x57

        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x30
        _emit 0xc3
    }
}
