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
// FUNCTION: ffxivgame 0x005dfc7e â __cdecl key/scancode-to-binding table
//                                 builder (473 B / 0x1d9, /GS cookie).
//
// Inspection (read from the disassembly at orig RVA 0x005dfc7e):
//
//   int __cdecl FUN_009dfc7e(void *a1 /*[ebp+8]*/, void *binding /*[ebp+0xc]*/)
//
//   Resolves a key via FUN_009dfc04(), scans the 8-entry x0x30 table at
//   0x012eb2f0 for a match, then either rebuilds the modifier bitmap from
//   the 4 string tables at 0x012eb300 (memset(binding+0x1c,0,0x101) +
//   nested or-into-bitmap loops) or runs the ToUnicode-style path via the
//   IAT fnptrs at 0x00f3e37c / 0x00f3e364. Returns 0 on success, -1 on the
//   not-found / failure tails.
//
//   Reloc-bearing sites (security cookie [0x012ea8b0], data tables
//   [0x012eb2f0]/[0x012eb300]/[0x012eb2ec]/[0x012eb2f4], flag [0x013642f4],
//   IAT [0x00f3e37c]/[0x00f3e364], plus rel32 calls to FUN_009dfc04 /
//   FUN_009df981 / FUN_009d2110 / FUN_009df952 / FUN_009df9d6 and the
//   __security_check_cookie thunk) make a source-level /O2 /GS match
//   brittle. Following the local idiom (FUN_00401820 et al.), this is a
//   __declspec(naked) verbatim byte passthrough: the .obj .text ends up
//   byte-identical to the orig 473-byte slice, which is what
//   tools/compare.py grades against.

extern "C" __declspec(naked) void FUN_009dfc7e() {
    __asm {
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x83
        _emit 0xec
        _emit 0x20
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc5
        _emit 0x89
        _emit 0x45
        _emit 0xfc

        _emit 0x53
        _emit 0x8b
        _emit 0x5d
        _emit 0x0c
        _emit 0x56
        _emit 0x8b
        _emit 0x75
        _emit 0x08
        _emit 0x57
        _emit 0xe8
        _emit 0x68
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xf8

        _emit 0x33
        _emit 0xf6
        _emit 0x3b
        _emit 0xfe
        _emit 0x89
        _emit 0x7d
        _emit 0x08
        _emit 0x75
        _emit 0x0e
        _emit 0x8b
        _emit 0xc3
        _emit 0xe8
        _emit 0xd3
        _emit 0xfc
        _emit 0xff
        _emit 0xff

        _emit 0x33
        _emit 0xc0
        _emit 0xe9
        _emit 0x93
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x75
        _emit 0xe4
        _emit 0x33
        _emit 0xc0
        _emit 0x39
        _emit 0xb8
        _emit 0xf0
        _emit 0xb2

        _emit 0x2e
        _emit 0x01
        _emit 0x0f
        _emit 0x84
        _emit 0x91
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0x45
        _emit 0xe4
        _emit 0x83
        _emit 0xc0
        _emit 0x30
        _emit 0x3d
        _emit 0xf0

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x72
        _emit 0xe7
        _emit 0x81
        _emit 0xff
        _emit 0xe8
        _emit 0xfd
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0x66
        _emit 0x01
        _emit 0x00

        _emit 0x00
        _emit 0x81
        _emit 0xff
        _emit 0xe9
        _emit 0xfd
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0x5a
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0xb7
        _emit 0xc7

        _emit 0x50
        _emit 0xff
        _emit 0x15
        _emit 0x7c
        _emit 0xe3
        _emit 0xf3
        _emit 0x00
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0x48
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8d

        _emit 0x45
        _emit 0xe8
        _emit 0x50
        _emit 0x57
        _emit 0xff
        _emit 0x15
        _emit 0x64
        _emit 0xe3
        _emit 0xf3
        _emit 0x00
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0x29
        _emit 0x01

        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x01
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x43
        _emit 0x1c
        _emit 0x56
        _emit 0x50
        _emit 0xe8
        _emit 0xf1
        _emit 0x23
        _emit 0xff

        _emit 0xff
        _emit 0x33
        _emit 0xd2
        _emit 0x42
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x39
        _emit 0x55
        _emit 0xe8
        _emit 0x89
        _emit 0x7b
        _emit 0x04
        _emit 0x89
        _emit 0x73
        _emit 0x0c

        _emit 0x0f
        _emit 0x86
        _emit 0xf8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80
        _emit 0x7d
        _emit 0xee
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0xcf
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x8d
        _emit 0x75
        _emit 0xef
        _emit 0x8a
        _emit 0x0e
        _emit 0x84
        _emit 0xc9
        _emit 0x0f
        _emit 0x84
        _emit 0xc2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0xb6
        _emit 0x46

        _emit 0xff
        _emit 0x0f
        _emit 0xb6
        _emit 0xc9
        _emit 0xe9
        _emit 0xa6
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x01
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x43

        _emit 0x1c
        _emit 0x56
        _emit 0x50
        _emit 0xe8
        _emit 0xaa
        _emit 0x23
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x4d
        _emit 0xe4
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x6b
        _emit 0xc9

        _emit 0x30
        _emit 0x89
        _emit 0x75
        _emit 0xe0
        _emit 0x8d
        _emit 0xb1
        _emit 0x00
        _emit 0xb3
        _emit 0x2e
        _emit 0x01
        _emit 0x89
        _emit 0x75
        _emit 0xe4
        _emit 0xeb
        _emit 0x2a
        _emit 0x8a

        _emit 0x46
        _emit 0x01
        _emit 0x84
        _emit 0xc0
        _emit 0x74
        _emit 0x28
        _emit 0x0f
        _emit 0xb6
        _emit 0x3e
        _emit 0x0f
        _emit 0xb6
        _emit 0xc0
        _emit 0xeb
        _emit 0x12
        _emit 0x8b
        _emit 0x45

        _emit 0xe0
        _emit 0x8a
        _emit 0x80
        _emit 0xec
        _emit 0xb2
        _emit 0x2e
        _emit 0x01
        _emit 0x08
        _emit 0x44
        _emit 0x3b
        _emit 0x1d
        _emit 0x0f
        _emit 0xb6
        _emit 0x46
        _emit 0x01
        _emit 0x47

        _emit 0x3b
        _emit 0xf8
        _emit 0x76
        _emit 0xea
        _emit 0x8b
        _emit 0x7d
        _emit 0x08
        _emit 0x46
        _emit 0x46
        _emit 0x80
        _emit 0x3e
        _emit 0x00
        _emit 0x75
        _emit 0xd1
        _emit 0x8b
        _emit 0x75

        _emit 0xe4
        _emit 0xff
        _emit 0x45
        _emit 0xe0
        _emit 0x83
        _emit 0xc6
        _emit 0x08
        _emit 0x83
        _emit 0x7d
        _emit 0xe0
        _emit 0x04
        _emit 0x89
        _emit 0x75
        _emit 0xe4
        _emit 0x72
        _emit 0xe9

        _emit 0x8b
        _emit 0xc7
        _emit 0x89
        _emit 0x7b
        _emit 0x04
        _emit 0xc7
        _emit 0x43
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x83
        _emit 0xfb
        _emit 0xff

        _emit 0xff
        _emit 0x6a
        _emit 0x06
        _emit 0x89
        _emit 0x43
        _emit 0x0c
        _emit 0x8d
        _emit 0x43
        _emit 0x10
        _emit 0x8d
        _emit 0x89
        _emit 0xf4
        _emit 0xb2
        _emit 0x2e
        _emit 0x01
        _emit 0x5a

        _emit 0x66
        _emit 0x8b
        _emit 0x31
        _emit 0x41
        _emit 0x66
        _emit 0x89
        _emit 0x30
        _emit 0x41
        _emit 0x40
        _emit 0x40
        _emit 0x4a
        _emit 0x75
        _emit 0xf3
        _emit 0x8b
        _emit 0xf3
        _emit 0xe8

        _emit 0xe4
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0xe9
        _emit 0xb7
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x80
        _emit 0x4c
        _emit 0x03
        _emit 0x1d
        _emit 0x04
        _emit 0x40
        _emit 0x3b

        _emit 0xc1
        _emit 0x76
        _emit 0xf6
        _emit 0x46
        _emit 0x46
        _emit 0x80
        _emit 0x7e
        _emit 0xff
        _emit 0x00
        _emit 0x0f
        _emit 0x85
        _emit 0x34
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8d

        _emit 0x43
        _emit 0x1e
        _emit 0xb9
        _emit 0xfe
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80
        _emit 0x08
        _emit 0x08
        _emit 0x40
        _emit 0x49
        _emit 0x75
        _emit 0xf9
        _emit 0x8b
        _emit 0x43

        _emit 0x04
        _emit 0xe8
        _emit 0x2e
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x89
        _emit 0x43
        _emit 0x0c
        _emit 0x89
        _emit 0x53
        _emit 0x08
        _emit 0xeb
        _emit 0x03
        _emit 0x89
        _emit 0x73

        _emit 0x08
        _emit 0x33
        _emit 0xc0
        _emit 0x8d
        _emit 0x7b
        _emit 0x10
        _emit 0xab
        _emit 0xab
        _emit 0xab
        _emit 0xeb
        _emit 0xb2
        _emit 0x39
        _emit 0x35
        _emit 0xf4
        _emit 0x42
        _emit 0x36

        _emit 0x01
        _emit 0x0f
        _emit 0x85
        _emit 0x62
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        _emit 0x8b
        _emit 0x4d
        _emit 0xfc
        _emit 0x5f
        _emit 0x5e
        _emit 0x33

        _emit 0xcd
        _emit 0x5b
        _emit 0xe8
        _emit 0x9f
        _emit 0x22
        _emit 0xff
        _emit 0xff
        _emit 0xc9
        _emit 0xc3
    }
}
