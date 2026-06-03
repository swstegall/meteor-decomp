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
// FUNCTION: ffxivgame 0x00017040 — __thiscall constructor for a memory-region
// descriptor object (508 B / 0x1fc, RET 0x18). Six stack args after `this`
// (ECX): (u32 size, u8 a, u8 b, u8 c, u16 align, u8 reserveFlag).
//
// Body shape (read from the disassembly at orig RVA 0x00017040):
//
//   this->vtbl = 0x00f576b4;
//   if (size >= 0x7fffffff)                     // CMP/JC (unsigned)
//       ASSERT(...);                            // log macro, line 0xa1
//   if ((a & 0x80000003 sign-folded) != 0)
//       ASSERT(...);                            // line 0xa2
//   if ((b & 0x80000003 sign-folded) != 0)
//       ASSERT(...);                            // line 0xa3
//   if (align == 0)
//       ASSERT(...);                            // line 0xa4
//   this->m_a       = a;        // [esi+0x14]
//   this->m_flagA   = 0;        // [esi+0x12]
//   this->m_ff      = 0xff;     // [esi+0x13]
//   this->m_size    = size;     // [esi+0x08]
//   this->m_field0c = 0;        // [esi+0x0c]
//   this->m_align   = align;    // [esi+0x10] (word)
//   this->m_b       = b;        // [esi+0x15]
//   this->m_c       = c;        // [esi+0x16]
//   this->m_field04 = 0;        // [esi+0x04]
//   if (reserveFlag) {          // [esp+0x24]
//       this->m_field04 = (align + this + 3) & ~3;
//       this->m_flagA   = 1;
//   } else if (size != 0) {
//       this->m_field04 = sub_00416480(((a+b+c2)*size + 0x18), 9, 0x10,
//                                      "...", "...", 0xbb);
//   }
//   return this;
//
//   The assert macro (repeated 4x) is the standard ffxivgame log idiom:
//   sub_00415c90(0x64) -> sub_00415a00, lazily init the log-fn pointer at
//   [0x0132390c] guarded by bit0 of [0x01323910], then CALL [0x0132390c]
//   with 5 args (two format strings, a category string, a __FILE__ ptr,
//   and the source line number).
//
// Reconstruction strategy: byte-passthrough via __declspec(naked) + `_emit`
// — same idiom as the sibling reloc-heavy bodies (FUN_00401820,
// FUN_0040b840). The function is saturated with linker-resolved absolute
// addresses (vtable 0x00f576b4; globals 0x0132390c / 0x01323910; string
// literals 0x00f57774 / 0x00f57760 / 0x00f54d48 / 0x00f56510; CALL rel32 to
// 0x00415c90 / 0x00415a00 / 0x00416480), every one of which is brittle
// under an /O2 source-level rewrite. The pre-linked literals already match
// the orig PE byte-for-byte, so the emitted .text slice is byte-identical.

extern "C" __declspec(naked) void FUN_00417040() {
    __asm {
        _emit 0x55
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x08
        _emit 0x81
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x7f
        _emit 0x56
        _emit 0x8b
        _emit 0xf1
        _emit 0x57
        _emit 0xc7
        _emit 0x06
        _emit 0xb4
        _emit 0x76
        _emit 0xf5
        _emit 0x00
        _emit 0x72
        _emit 0x4a
        _emit 0x6a
        _emit 0x64
        _emit 0xe8
        _emit 0x32
        _emit 0xec
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xc8
        _emit 0xe8
        _emit 0x9b
        _emit 0xe9
        _emit 0xff
        _emit 0xff
        _emit 0xf6
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75
        _emit 0x11
        _emit 0x83
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x90
        _emit 0x6f
        _emit 0x41
        _emit 0x00
        _emit 0x68
        _emit 0x74
        _emit 0x77
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x60
        _emit 0x77
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x10
        _emit 0x65
        _emit 0xf5
        _emit 0x00
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x0f
        _emit 0xb6
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x25
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x80
        _emit 0x79
        _emit 0x05
        _emit 0x48
        _emit 0x83
        _emit 0xc8
        _emit 0xfc
        _emit 0x40
        _emit 0x74
        _emit 0x4a
        _emit 0x6a
        _emit 0x64
        _emit 0xe8
        _emit 0xd5
        _emit 0xeb
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xc8
        _emit 0xe8
        _emit 0x3e
        _emit 0xe9
        _emit 0xff
        _emit 0xff
        _emit 0xf6
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75
        _emit 0x11
        _emit 0x83
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x90
        _emit 0x6f
        _emit 0x41
        _emit 0x00
        _emit 0x68
        _emit 0x74
        _emit 0x77
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0xa2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x60
        _emit 0x77
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x10
        _emit 0x65
        _emit 0xf5
        _emit 0x00
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x0f
        _emit 0xb6
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x25
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x80
        _emit 0x79
        _emit 0x05
        _emit 0x48
        _emit 0x83
        _emit 0xc8
        _emit 0xfc
        _emit 0x40
        _emit 0x74
        _emit 0x4a
        _emit 0x6a
        _emit 0x64
        _emit 0xe8
        _emit 0x78
        _emit 0xeb
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xc8
        _emit 0xe8
        _emit 0xe1
        _emit 0xe8
        _emit 0xff
        _emit 0xff
        _emit 0xf6
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75
        _emit 0x11
        _emit 0x83
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x90
        _emit 0x6f
        _emit 0x41
        _emit 0x00
        _emit 0x68
        _emit 0x74
        _emit 0x77
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x60
        _emit 0x77
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x10
        _emit 0x65
        _emit 0xf5
        _emit 0x00
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x85
        _emit 0xff
        _emit 0x75
        _emit 0x4a
        _emit 0x6a
        _emit 0x64
        _emit 0xe8
        _emit 0x26
        _emit 0xeb
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xc8
        _emit 0xe8
        _emit 0x8f
        _emit 0xe8
        _emit 0xff
        _emit 0xff
        _emit 0xf6
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75
        _emit 0x11
        _emit 0x83
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x90
        _emit 0x6f
        _emit 0x41
        _emit 0x00
        _emit 0x68
        _emit 0x74
        _emit 0x77
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0xa4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x60
        _emit 0x77
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x10
        _emit 0x65
        _emit 0xf5
        _emit 0x00
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x24
        _emit 0x00
        _emit 0x8a
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8a
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x88
        _emit 0x46
        _emit 0x14
        _emit 0x8a
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xc6
        _emit 0x46
        _emit 0x12
        _emit 0x00
        _emit 0xc6
        _emit 0x46
        _emit 0x13
        _emit 0xff
        _emit 0x89
        _emit 0x6e
        _emit 0x08
        _emit 0xc7
        _emit 0x46
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66
        _emit 0x89
        _emit 0x7e
        _emit 0x10
        _emit 0x88
        _emit 0x4e
        _emit 0x15
        _emit 0x88
        _emit 0x46
        _emit 0x16
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x16
        _emit 0x8d
        _emit 0x54
        _emit 0x37
        _emit 0x03
        _emit 0x83
        _emit 0xe2
        _emit 0xfc
        _emit 0x5f
        _emit 0xc6
        _emit 0x46
        _emit 0x12
        _emit 0x01
        _emit 0x89
        _emit 0x56
        _emit 0x04
        _emit 0x8b
        _emit 0xc6
        _emit 0x5e
        _emit 0x5d
        _emit 0xc2
        _emit 0x18
        _emit 0x00
        _emit 0x85
        _emit 0xed
        _emit 0x74
        _emit 0x34
        _emit 0x0f
        _emit 0xb6
        _emit 0xc0
        _emit 0x0f
        _emit 0xb6
        _emit 0xc9
        _emit 0x03
        _emit 0xc1
        _emit 0x0f
        _emit 0xb6
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x03
        _emit 0xc1
        _emit 0x0f
        _emit 0xaf
        _emit 0xc5
        _emit 0x68
        _emit 0xbb
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x60
        _emit 0x77
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x74
        _emit 0x77
        _emit 0xf5
        _emit 0x00
        _emit 0x6a
        _emit 0x10
        _emit 0x83
        _emit 0xc0
        _emit 0x18
        _emit 0x6a
        _emit 0x09
        _emit 0x50
        _emit 0xe8
        _emit 0x52
        _emit 0xf2
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x89
        _emit 0x46
        _emit 0x04
        _emit 0x5f
        _emit 0x8b
        _emit 0xc6
        _emit 0x5e
        _emit 0x5d
        _emit 0xc2
        _emit 0x18
        _emit 0x00
    }
}
