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
// FUNCTION: ffxivgame 0x0004f9a0 — __cdecl memcmp(a, b, n) returning the
//                                  sign of the first differing byte
//                                  (142 B / 0x8e).
//
// __cdecl int memcmp(const void *a /*EDX, [esp+4]*/,
//                    const void *b /*ECX, [esp+8]*/,
//                    size_t n      /*EAX, [esp+0xc]*/):
//
//   Classic MSVC-2005 small-memcmp shape:
//     * a DWORD-at-a-time fast loop while n >= 4 (compares whole dwords,
//       bailing to the byte tail on the first mismatching dword),
//     * a 4x-unrolled byte tail subtracting unsigned bytes; the first
//       non-zero difference decides the result,
//     * result: 0 if equal, +1 if a-byte > b-byte (JG), -1 otherwise.
//
// No CALLs and no relocations — the 142-byte slice is position-independent,
// so this is a pure __declspec(naked) _emit byte passthrough whose .text is
// byte-identical to the original. The register-register SUB/CMP encodings
// (2b f7, 2b f0, ...) are pinned exactly rather than left to the MASM
// inline-assembler's encoding choice.

extern "C" __declspec(naked) void FUN_0044f9a0() {
    __asm {
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x83
        _emit 0xf8
        _emit 0x04
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x56
        _emit 0x57
        _emit 0x72
        _emit 0x14
        _emit 0x8b
        _emit 0x32
        _emit 0x3b
        _emit 0x31
        _emit 0x75
        _emit 0x12
        _emit 0x83
        _emit 0xe8
        _emit 0x04
        _emit 0x83
        _emit 0xc1
        _emit 0x04
        _emit 0x83
        _emit 0xc2
        _emit 0x04
        _emit 0x83
        _emit 0xf8
        _emit 0x04
        _emit 0x73
        _emit 0xec
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x5e
        _emit 0x0f
        _emit 0xb6
        _emit 0x32
        _emit 0x0f
        _emit 0xb6
        _emit 0x39
        _emit 0x2b
        _emit 0xf7
        _emit 0x75
        _emit 0x45
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        _emit 0x83
        _emit 0xc2
        _emit 0x01
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x47
        _emit 0x0f
        _emit 0xb6
        _emit 0x32
        _emit 0x0f
        _emit 0xb6
        _emit 0x39
        _emit 0x2b
        _emit 0xf7
        _emit 0x75
        _emit 0x2e
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        _emit 0x83
        _emit 0xc2
        _emit 0x01
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x30
        _emit 0x0f
        _emit 0xb6
        _emit 0x32
        _emit 0x0f
        _emit 0xb6
        _emit 0x39
        _emit 0x2b
        _emit 0xf7
        _emit 0x75
        _emit 0x17
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        _emit 0x83
        _emit 0xc2
        _emit 0x01
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x19
        _emit 0x0f
        _emit 0xb6
        _emit 0x32
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        _emit 0x2b
        _emit 0xf0
        _emit 0x74
        _emit 0x0f
        _emit 0x85
        _emit 0xf6
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x7f
        _emit 0x08
        _emit 0x5f
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        _emit 0x5e
        _emit 0xc3
        _emit 0x33
        _emit 0xc0
        _emit 0x5f
        _emit 0x5e
        _emit 0xc3
    }
}
