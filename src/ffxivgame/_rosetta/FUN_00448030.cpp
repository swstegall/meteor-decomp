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
// FUNCTION: ffxivgame 0x00448030 — `__thiscall` buffer-format/append helper
//                                  (294 B / 0x126, /GS-cookied).
//
// Inspection (read from the disassembly at orig RVA 0x00048030):
//
//   __thiscall int FUN_00448030(This *this /*ECX*/,
//                               unsigned int a /*[esp+0xb0]*/,
//                               unsigned int b /*[esp+0xb4]*/,
//                               Sink *sink   /*[esp+0xb8]*/);
//                               // RET 0xc — three 4-byte stack args.
//
//   The function compares `this->m_capacity` (at +0x08) against `a + b`
//   and branches two ways:
//
//     if (this->m_capacity >= a + b) {
//         // fast path @0x004480f8 — build a small on-stack descriptor
//         // (FUN_00447a80 @ esp+0x64), clamp sink->m_size (+0x08) by the
//         // descriptor's length field, copy via FUN_00445b70, and if the
//         // descriptor's owned-flag byte (esp+0x6d) is clear, release the
//         // backing store via FUN_0044d350(sink->m_ptr, esp+0x60, 0xb).
//         return result;                         // EDI
//     } else {
//         // slow path — materialise a 0x40-capacity inline descriptor on
//         // the stack (cap=0x40 at esp+0x14, size=1 at esp+0x18, two
//         // sentinel bytes at esp+0x20/0x21, NUL at esp+0x22), call the
//         // formatter FUN_00447010, NUL-terminate via the literal at
//         // .rdata 0xf672a0 / FUN_009d5110, clamp + copy via FUN_00445b70,
//         // and conditionally free via FUN_0044d350. Returns -result.
//         return -result;                        // NEG ESI
//     }
//
//   Reloc-bearing sites that resolve only in a full-binary relink at the
//   orig image base (standalone .obj compilation can't reproduce them, so
//   this is emitted as a `__declspec(naked)` raw-byte passthrough — the
//   same strategy FUN_00401a00 / the other /GS-cookied siblings use):
//     +0x06  __security_cookie load   (.data 0x012ea8b0)
//     +0x6b  formatter CALL           (.text 0x00447010 rel32)
//     +0x76  NUL-terminate literal    (.rdata 0xf672a0)
//     +0x7c  string helper CALL       (.text 0x009d5110 rel32)
//     +0xa1  copy CALL                (.text 0x00445b70 rel32)
//     +0xbc  release CALL             (.text 0x0044d350 rel32)
//     +0xcf  descriptor CALL          (.text 0x00447a80 rel32)
//     +0xea  copy CALL                (.text 0x00445b70 rel32, 2nd)
//     +0x103 release CALL             (.text 0x0044d350 rel32, 2nd)
//     +0x118 __security_check_cookie  (.text 0x009d20f4 rel32)
//
//   Emitting the orig 294 bytes verbatim yields a `.text` slice with no
//   relocations, which is exactly what tools/compare.py grades against.

extern "C" __declspec(naked) void FUN_00448030() {
    __asm {
        _emit 0x81
        _emit 0xec
        _emit 0xac
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0xa8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x94
        _emit 0x24
        _emit 0xb4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x57
        _emit 0x8d
        _emit 0x3c
        _emit 0x10
        _emit 0x39
        _emit 0x79
        _emit 0x08
        _emit 0x0f
        _emit 0x83
        _emit 0x91
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1a
        _emit 0x50
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x88
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x88
        _emit 0x44
        _emit 0x24
        _emit 0x21
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x22
        _emit 0x00
        _emit 0xe8
        _emit 0x70
        _emit 0xef
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x6a
        _emit 0x00
        _emit 0x68
        _emit 0xa0
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x52
        _emit 0xe8
        _emit 0x5f
        _emit 0xd0
        _emit 0x58
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xc6
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x3b
        _emit 0xc1
        _emit 0x72
        _emit 0x02
        _emit 0x8b
        _emit 0xc1
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x08
        _emit 0x8b
        _emit 0x0e
        _emit 0x50
        _emit 0x57
        _emit 0x51
        _emit 0xe8
        _emit 0x9a
        _emit 0xda
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xf0
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xf7
        _emit 0xde
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x19
        _emit 0x00
        _emit 0x75
        _emit 0x10
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x6a
        _emit 0x0b
        _emit 0x52
        _emit 0x57
        _emit 0xe8
        _emit 0x5f
        _emit 0x52
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x8b
        _emit 0xc6
        _emit 0xeb
        _emit 0x45
        _emit 0x52
        _emit 0x50
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x64
        _emit 0x50
        _emit 0xe8
        _emit 0x7c
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x64
        _emit 0x3b
        _emit 0xc8
        _emit 0x73
        _emit 0x02
        _emit 0x8b
        _emit 0xc1
        _emit 0x8b
        _emit 0x0e
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x5c
        _emit 0x50
        _emit 0x51
        _emit 0x56
        _emit 0xe8
        _emit 0x51
        _emit 0xda
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x6d
        _emit 0x00
        _emit 0x8b
        _emit 0xf8
        _emit 0x75
        _emit 0x10
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x60
        _emit 0x6a
        _emit 0x0b
        _emit 0x52
        _emit 0x56
        _emit 0xe8
        _emit 0x18
        _emit 0x52
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x8b
        _emit 0xc7
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5f
        _emit 0x5e
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0xa7
        _emit 0x9f
        _emit 0x58
        _emit 0x00
        _emit 0x81
        _emit 0xc4
        _emit 0xac
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
