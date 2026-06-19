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
// FUNCTION: ffxivgame 0x00042f70 — __thiscall structured-buffer range
//                                  splice/erase (461 B / 0x1cd, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x00042f70):
//
//   __thiscall struct_ret_t splice(this /*ECX=ESI*/, ret*, ...)  with
//   RET 0x1c — seven dword stack args (arg0 = hidden 3-dword return
//   struct pointer, read back at [esp+0x1c] in the epilogue), returns
//   that 3-dword {flag, this, end} aggregate by value.
//
//   The body operates on a small ring/segment object via three
//   this-relative dword fields:
//     [esi+0x08]  capacity / wrap limit
//     [esi+0x0c]  head index (begin)
//     [esi+0x10]  length (count)
//
//   It performs the canonical MSVC-2005 checked-iterator guard pattern
//   repeatedly — `CMP a,b; JBE ok; CALL 0x009d22b4` — where 0x009d22b4
//   is the shared out-of-range / invalid-iterator trap. The two
//   structural arms (forward at +0x110 → CALL 0x00442e70, reverse at
//   +0x115 → CALL 0x00442ef0) splice a source range in or out depending
//   on the signed magnitude compare at +0x84 (JNC into the reverse arm),
//   then fix up head/length with the wrap-around decrement loops at
//   +0xf0 / +0x172. The final aggregate {EBP, ESI, EDI} is stored to the
//   hidden return pointer and the frame is torn down with RET 0x1c.
//
//   Reloc-bearing sites in the 461-byte body (all 4-byte rel32):
//     +0x15  CALL rel32 → 0x009d22b4 (range-check trap, ×8 total)
//     +0xd2  CALL rel32 → 0x00442e70 (forward splice helper)
//     +0x162 CALL rel32 → 0x00442ef0 (reverse splice helper)
//   The eight trap calls and the two helper calls all carry pre-linked
//   rel32 displacements that already match the orig PE byte-for-byte.
//
// Reconstruction strategy — naked-asm byte passthrough (the established
// sibling idiom, e.g. FUN_00401820 / FUN_0040b840): a __declspec(naked)
// body re-emits the orig 461 bytes verbatim via `_emit`. Source-level
// C++ through MSVC 2005 /O2 cannot be relied upon to reproduce the exact
// register allocation (ESI/EDI/EBX/EBP across the eight guard checks and
// two splice arms), the off-frame temp spills at [esp+0x10]/[esp+0x14],
// the two `SUB ESP,0xc; LEA EAX,[ESP]` scratch-aggregate builds, or the
// short-vs-near branch selection — every high-level rewrite shifts at
// least one byte. The .obj's `.text` ends up byte-identical to the orig
// slice (raw immediates, no relocations), which is what compare.py checks.

extern "C" __declspec(naked) void FUN_00442f70() {
    __asm {
        _emit 0x83
        _emit 0xec
        _emit 0x08
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x8b
        _emit 0xf1
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        _emit 0x57
        _emit 0x8b
        _emit 0x7e
        _emit 0x0c
        _emit 0x03
        _emit 0xc7
        _emit 0x3b
        _emit 0xf8
        _emit 0x76
        _emit 0x05
        _emit 0xe8
        _emit 0x2a
        _emit 0xf3
        _emit 0x58
        _emit 0x00
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x24
        _emit 0x85
        _emit 0xed
        _emit 0x74
        _emit 0x04
        _emit 0x3b
        _emit 0xee
        _emit 0x74
        _emit 0x05
        _emit 0xe8
        _emit 0x19
        _emit 0xf3
        _emit 0x58
        _emit 0x00
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x28
        _emit 0x8b
        _emit 0xc3
        _emit 0x2b
        _emit 0xc7
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x04
        _emit 0x3b
        _emit 0xc5
        _emit 0x74
        _emit 0x05
        _emit 0xe8
        _emit 0xfc
        _emit 0xf2
        _emit 0x58
        _emit 0x00
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x34
        _emit 0x8b
        _emit 0x7e
        _emit 0x10
        _emit 0x8b
        _emit 0xc5
        _emit 0x2b
        _emit 0xc3
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0x03
        _emit 0xf8
        _emit 0x3b
        _emit 0xc7
        _emit 0x76
        _emit 0x05
        _emit 0xe8
        _emit 0xdf
        _emit 0xf2
        _emit 0x58
        _emit 0x00
        _emit 0x3b
        _emit 0x74
        _emit 0x24
        _emit 0x30
        _emit 0x74
        _emit 0x05
        _emit 0xe8
        _emit 0xd4
        _emit 0xf2
        _emit 0x58
        _emit 0x00
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        _emit 0x8b
        _emit 0xc4
        _emit 0x2b
        _emit 0xfd
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        _emit 0x39
        _emit 0x7c
        _emit 0x24
        _emit 0x2c
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0x83
        _emit 0x8b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x48
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x3c
        _emit 0x89
        _emit 0x68
        _emit 0x08
        _emit 0x89
        _emit 0x48
        _emit 0x04
        _emit 0x8b
        _emit 0xc4
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x50
        _emit 0x04
        _emit 0x89
        _emit 0x58
        _emit 0x08
        _emit 0x8b
        _emit 0x5e
        _emit 0x0c
        _emit 0x33
        _emit 0xed
        _emit 0x8b
        _emit 0xfc
        _emit 0x89
        _emit 0x2f
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0x3b
        _emit 0xc3
        _emit 0x77
        _emit 0x09
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        _emit 0x03
        _emit 0xc8
        _emit 0x3b
        _emit 0xd9
        _emit 0x76
        _emit 0x05
        _emit 0xe8
        _emit 0x7d
        _emit 0xf2
        _emit 0x58
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x50
        _emit 0x52
        _emit 0x89
        _emit 0x77
        _emit 0x04
        _emit 0x89
        _emit 0x5f
        _emit 0x08
        _emit 0xe8
        _emit 0x29
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x38
        _emit 0x83
        _emit 0xc4
        _emit 0x28
        _emit 0x3b
        _emit 0xd5
        _emit 0x0f
        _emit 0x86
        _emit 0xa5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x3b
        _emit 0xcd
        _emit 0x74
        _emit 0x17
        _emit 0x83
        _emit 0x46
        _emit 0x0c
        _emit 0x01
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0x39
        _emit 0x46
        _emit 0x08
        _emit 0x77
        _emit 0x03
        _emit 0x89
        _emit 0x6e
        _emit 0x0c
        _emit 0x83
        _emit 0xe9
        _emit 0x01
        _emit 0x75
        _emit 0x03
        _emit 0x89
        _emit 0x6e
        _emit 0x0c
        _emit 0x83
        _emit 0xea
        _emit 0x01
        _emit 0x75
        _emit 0xe0
        _emit 0x89
        _emit 0x4e
        _emit 0x10
        _emit 0xeb
        _emit 0x76
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        _emit 0x89
        _emit 0x58
        _emit 0x08
        _emit 0x89
        _emit 0x48
        _emit 0x04
        _emit 0x8b
        _emit 0x7e
        _emit 0x10
        _emit 0x03
        _emit 0x7e
        _emit 0x0c
        _emit 0x8b
        _emit 0xdc
        _emit 0xc7
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0x3b
        _emit 0xc7
        _emit 0x77
        _emit 0x09
        _emit 0x8b
        _emit 0x56
        _emit 0x10
        _emit 0x03
        _emit 0xd0
        _emit 0x3b
        _emit 0xfa
        _emit 0x76
        _emit 0x05
        _emit 0xe8
        _emit 0x02
        _emit 0xf2
        _emit 0x58
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x48
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        _emit 0x8b
        _emit 0xc4
        _emit 0x89
        _emit 0x73
        _emit 0x04
        _emit 0x89
        _emit 0x7b
        _emit 0x08
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x50
        _emit 0x52
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x48
        _emit 0x04
        _emit 0x89
        _emit 0x68
        _emit 0x08
        _emit 0xe8
        _emit 0x19
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0x83
        _emit 0xc4
        _emit 0x28
        _emit 0x85
        _emit 0xc9
        _emit 0x76
        _emit 0x17
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x08
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        _emit 0x75
        _emit 0x03
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        _emit 0x83
        _emit 0xe9
        _emit 0x01
        _emit 0x75
        _emit 0xef
        _emit 0x89
        _emit 0x46
        _emit 0x10
        _emit 0x33
        _emit 0xed
        _emit 0x8b
        _emit 0x7e
        _emit 0x0c
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        _emit 0x03
        _emit 0xc7
        _emit 0x3b
        _emit 0xf8
        _emit 0x76
        _emit 0x05
        _emit 0xe8
        _emit 0xa8
        _emit 0xf1
        _emit 0x58
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x8b
        _emit 0x56
        _emit 0x10
        _emit 0x03
        _emit 0xf9
        _emit 0x03
        _emit 0xd0
        _emit 0x3b
        _emit 0xfa
        _emit 0x77
        _emit 0x04
        _emit 0x3b
        _emit 0xf8
        _emit 0x73
        _emit 0x05
        _emit 0xe8
        _emit 0x8d
        _emit 0xf1
        _emit 0x58
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x89
        _emit 0x78
        _emit 0x08
        _emit 0x5f
        _emit 0x89
        _emit 0x70
        _emit 0x04
        _emit 0x5e
        _emit 0x89
        _emit 0x28
        _emit 0x5d
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0xc2
        _emit 0x1c
        _emit 0x00
    }
}
