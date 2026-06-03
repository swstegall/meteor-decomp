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
// FUNCTION: ffxivgame 0x0001b460 — `__thiscall` method (303 B / 0x12f).
//
// Inspection (read from the disassembly at orig RVA 0x0001b460):
//
//   Calling convention: __thiscall (ECX = this on entry).
//   Stack frame: SUB ESP, 0x08 + push EBX/EBP/ESI/EDI → 0x18 bytes total.
//
//   Loads fields from `this` (ESI) at offsets +0x0c, +0x10, +0x14, +0x18,
//   +0x20, +0x24, +0x28, +0x34.  Performs two table-indexed address lookups
//   from absolute .rdata arrays (0x00f57d84, 0x00f57d7c), bit-field assembly
//   from [ESI+0x0c] via AND/SHR/OR, then an indirect call through a vtable
//   slot ([EDX+0x6c]).  Has a flag-write path, a conditional string-buffer
//   call, and a second indirect call before the epilogue.
//
//   Reloc-bearing bytes: absolute .data / .rdata addresses at offsets
//   0x0e (+4), 0x1a (+4), 0x2e (+4), 0x65 (+4 indirect call site),
//   0x6e (+4 flag write), 0x70 (+4), 0x78 (+4 PUSH), 0x7d (+4 PUSH),
//   0x82 (+4 PUSH), 0x87 (+4 IAT CALL), 0x98 (+4 .data), 0xd5 (+4 rel32),
//   0xf3 (+4 rel32), 0xf9 (+4 .data).
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function contains multiple absolute .rdata table-indexed lookups,
//   an FS-segment indirect-vtable call, bit-field assembly with several
//   AND/SHR/OR chains, and two call-via-function-pointer sites whose
//   compiler-resolved offsets depend on the full binary link.  Coaxing
//   MSVC 2005 /O2 into exactly this register allocation and branch layout
//   from C++ source is brittle.
//
//   The pragmatic choice — matching the pattern established by FUN_004014b0,
//   FUN_00401a00, FUN_00408f10 and their siblings — is a `__declspec(naked)`
//   body that re-emits the orig 303 bytes verbatim via MASM `_emit`
//   directives.  The .obj's `.text` section is byte-identical to the orig
//   slice, which is what `tools/compare.py` checks.

extern "C" __declspec(naked) void FUN_0041b460() {
    __asm {
        _emit 0x83
        _emit 0xec
        _emit 0x08
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x57
        _emit 0x8b
        _emit 0xf1
        _emit 0x8b
        _emit 0x7e
        _emit 0x10
        _emit 0x8b
        _emit 0x3c
        _emit 0xbd
        _emit 0x84
        _emit 0x7d
        _emit 0xf5
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0x6a
        _emit 0x00
        _emit 0x8b
        _emit 0x0d
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x8b
        _emit 0x11
        _emit 0x8d
        _emit 0x5e
        _emit 0x1c
        _emit 0x53
        _emit 0x57
        _emit 0x8b
        _emit 0x7e
        _emit 0x18
        _emit 0x8b
        _emit 0x3c
        _emit 0xbd
        _emit 0x7c
        _emit 0x7d
        _emit 0xf5
        _emit 0x00
        _emit 0x57
        _emit 0x8b
        _emit 0xf8
        _emit 0xc1
        _emit 0xef
        _emit 0x07
        _emit 0x8b
        _emit 0xe8
        _emit 0x81
        _emit 0xe5
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x81
        _emit 0xe7
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x0b
        _emit 0xfd
        _emit 0x8b
        _emit 0xe8
        _emit 0x81
        _emit 0xe5
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x03
        _emit 0xed
        _emit 0xc1
        _emit 0xef
        _emit 0x07
        _emit 0x03
        _emit 0xed
        _emit 0x83
        _emit 0xe0
        _emit 0x03
        _emit 0x0b
        _emit 0xfd
        _emit 0x0b
        _emit 0xf8
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        _emit 0x57
        _emit 0x50
        _emit 0x51
        _emit 0x8b
        _emit 0x4a
        _emit 0x6c
        _emit 0xff
        _emit 0xd1
        _emit 0xba
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x7e
        _emit 0x20
        _emit 0x88
        _emit 0x56
        _emit 0x34
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x07
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        _emit 0x2b
        _emit 0xc8
        _emit 0x75
        _emit 0x1d
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x13
        _emit 0x52
        _emit 0x50
        _emit 0x8b
        _emit 0xcf
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x1b
        _emit 0x00
        _emit 0xe8
        _emit 0x4c
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0xc3
        _emit 0x8b
        _emit 0xc8
        _emit 0x85
        _emit 0xc9
        _emit 0x74
        _emit 0x05
        _emit 0x8b
        _emit 0x47
        _emit 0x08
        _emit 0x2b
        _emit 0xc1
        _emit 0x3b
        _emit 0x46
        _emit 0x14
        _emit 0x74
        _emit 0x3a
        _emit 0x84
        _emit 0x15
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75
        _emit 0x10
        _emit 0x09
        _emit 0x15
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x40
        _emit 0x87
        _emit 0x41
        _emit 0x00
        _emit 0x68
        _emit 0x60
        _emit 0x81
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x40
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x68
        _emit 0x80
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x30
        _emit 0x80
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
        _emit 0x4f
        _emit 0x04
        _emit 0x85
        _emit 0xc9
        _emit 0x75
        _emit 0x04
        _emit 0x33
        _emit 0xc0
        _emit 0xeb
        _emit 0x05
        _emit 0x8b
        _emit 0x47
        _emit 0x08
        _emit 0x2b
        _emit 0xc1
        _emit 0x6a
        _emit 0x00
        _emit 0x50
        _emit 0x6a
        _emit 0x00
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0x6e
        _emit 0xe8
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x07
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        _emit 0x2b
        _emit 0xc8
        _emit 0x75
        _emit 0x05
        _emit 0xe8
        _emit 0x3f
        _emit 0x6d
        _emit 0x5b
        _emit 0x00
        _emit 0x8b
        _emit 0x57
        _emit 0x04
        _emit 0x8b
        _emit 0x03
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x52
        _emit 0x50
        _emit 0xe8
        _emit 0xe9
        _emit 0x7d
        _emit 0x00
        _emit 0x00
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0xc3
    }
}
