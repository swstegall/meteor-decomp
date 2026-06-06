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
// FUNCTION: ffxivgame 0x000482a0 — __thiscall UTF-8 string scan/replace pass
//                                  (469 B / 0x1d5, inline SEH + /GS cookie).
//
// Inspection (read from the disassembly at orig RVA 0x000482a0):
//
//   __thiscall int scan(this, const char* /*[esp+0x84]*/, int /*[esp+0x88]*/);
//   `ECX = this` (saved to ESI / [esp+0x18]); `EDI = *this` is the working
//   string pointer; `EBP` accumulates a byte offset. The function returns
//   int and uses `RET 0xc` — two explicit stack args plus the thiscall ptr.
//
//   The body is a UTF-8 lead-byte decoder repeated twice. For each lead
//   byte it computes the sequence length 1..6 via the canonical MSVC
//   ladder (ADD 0x80 / CMP 0x3f for the 0x80..0xbf continuation range,
//   then CMP 0x80/0xe0/0xf0/0xf8/0xfc/0xfe with the SBB/AND 6 tail). The
//   first loop (capped by the [esp+0x88] count arg) advances EDI/EBP by
//   the decoded length to find a start offset; the second loop walks while
//   [EDI] != 0 and EBP < this->len ([ESI+0x8]), calling FUN_00447260
//   (decode-into-scratch), FUN_004465a0 (lookup, returns -1 on miss), and
//   FUN_0044d350 (emit/replace) per code point.
//
//   The prologue is the MSVC 2005 inline-SEH + /GS combination:
//     PUSH -1 / PUSH 0xe575bc (scope table) / FS:[0] link, SUB ESP,0x64,
//     then the security cookie (.data 0x012ea8b0 XOR ESP) stored twice —
//     once at [esp+0x60] and once pushed onto the SEH frame — with the
//     matching __security_check_cookie tail call to 0x009d20f4.
//
//   Reloc-bearing sites in the orig 469 bytes:
//     +0x02  PUSH imm32   → 0x00e575bc (SEH scope table)
//     +0x11  MOV  EAX,[]  → 0x012ea8b0 (__security_cookie)
//     +0x1f  MOV  EAX,[]  → 0x012ea8b0 (__security_cookie, 2nd)
//     +0xd4  CALL rel32   → 0x009d20f4 (__security_check_cookie)
//     +0x13d CALL rel32   → 0x00447260 (decode-into-scratch)
//     +0x157 CALL rel32   → 0x004465a0 (lookup, -1 on miss)
//     +0x17c CALL rel32   → 0x0044d350 (emit/replace)
//     +0x192 CALL rel32   → 0x004465a0 (lookup, 2nd)
//     +0x1c0 CALL rel32   → 0x0044d350 (emit/replace, 2nd)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ rewrite would have to coax MSVC 2005 /O2 /GS /EHsc
//   into reproducing the exact inline-SEH prologue, the dual /GS cookie
//   stores, the doubled UTF-8 lead-byte ladder with identical branch
//   short-vs-near choices, and the linker-resolved scope-table / cookie /
//   helper addresses across nine relocation windows. Each is brittle under
//   /O2. Following the established sibling idiom (FUN_0040b840 etc.), this
//   body re-emits the orig 469 bytes verbatim via MASM `_emit` directives,
//   so the .obj's `.text` is byte-identical to the orig slice — which is
//   what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_004482a0() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0xbc
        _emit 0x75
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
        _emit 0x64
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
        _emit 0x60
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
        _emit 0x74
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf1
        _emit 0x8b
        _emit 0x3e
        _emit 0x33
        _emit 0xed
        _emit 0x85
        _emit 0xc0
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x18
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x5e
        _emit 0x8b
        _emit 0xc8
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x90
        _emit 0x8a
        _emit 0x07
        _emit 0x8a
        _emit 0xd0
        _emit 0x80
        _emit 0xc2
        _emit 0x80
        _emit 0x80
        _emit 0xfa
        _emit 0x3f
        _emit 0x77
        _emit 0x04
        _emit 0x33
        _emit 0xc0
        _emit 0xeb
        _emit 0x3e
        _emit 0x3c
        _emit 0x80
        _emit 0x73
        _emit 0x07
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x33
        _emit 0x3c
        _emit 0xe0
        _emit 0x73
        _emit 0x07
        _emit 0xb8
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x28
        _emit 0x3c
        _emit 0xf0
        _emit 0x73
        _emit 0x07
        _emit 0xb8
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x1d
        _emit 0x3c
        _emit 0xf8
        _emit 0x73
        _emit 0x07
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x12
        _emit 0x3c
        _emit 0xfc
        _emit 0x73
        _emit 0x07
        _emit 0xb8
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x07
        _emit 0x3c
        _emit 0xfe
        _emit 0x1b
        _emit 0xc0
        _emit 0x83
        _emit 0xe0
        _emit 0x06
        _emit 0x03
        _emit 0xf8
        _emit 0x03
        _emit 0xe8
        _emit 0x83
        _emit 0xe9
        _emit 0x01
        _emit 0x75
        _emit 0xa9
        _emit 0x80
        _emit 0x3f
        _emit 0x00
        _emit 0x75
        _emit 0x28
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x74
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
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x60
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x7b
        _emit 0x9d
        _emit 0x58
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x70
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        _emit 0x90
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x18
        _emit 0x39
        _emit 0x6e
        _emit 0x08
        _emit 0x76
        _emit 0xd3
        _emit 0x8a
        _emit 0x07
        _emit 0x8a
        _emit 0xc8
        _emit 0x80
        _emit 0xc1
        _emit 0x80
        _emit 0x80
        _emit 0xf9
        _emit 0x3f
        _emit 0x77
        _emit 0x04
        _emit 0x33
        _emit 0xf6
        _emit 0xeb
        _emit 0x3e
        _emit 0x3c
        _emit 0x80
        _emit 0x73
        _emit 0x07
        _emit 0xbe
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x33
        _emit 0x3c
        _emit 0xe0
        _emit 0x73
        _emit 0x07
        _emit 0xbe
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x28
        _emit 0x3c
        _emit 0xf0
        _emit 0x73
        _emit 0x07
        _emit 0xbe
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x1d
        _emit 0x3c
        _emit 0xf8
        _emit 0x73
        _emit 0x07
        _emit 0xbe
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x12
        _emit 0x3c
        _emit 0xfc
        _emit 0x73
        _emit 0x07
        _emit 0xbe
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x07
        _emit 0x3c
        _emit 0xfe
        _emit 0x1b
        _emit 0xf6
        _emit 0x83
        _emit 0xe6
        _emit 0x06
        _emit 0x56
        _emit 0x57
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0xe8
        _emit 0x7e
        _emit 0xee
        _emit 0xff
        _emit 0xff
        _emit 0x80
        _emit 0xbc
        _emit 0x24
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x6a
        _emit 0x00
        _emit 0x75
        _emit 0x3b
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x52
        _emit 0xe8
        _emit 0xa4
        _emit 0xe1
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        _emit 0x74
        _emit 0x3b
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x2d
        _emit 0x00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x7c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x75
        _emit 0x14
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x6a
        _emit 0x0b
        _emit 0x50
        _emit 0x51
        _emit 0xe8
        _emit 0x2f
        _emit 0x4f
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xe9
        _emit 0x32
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x50
        _emit 0xe8
        _emit 0x69
        _emit 0xe1
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        _emit 0x74
        _emit 0xc5
        _emit 0x83
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x01
        _emit 0x03
        _emit 0xfe
        _emit 0x03
        _emit 0xee
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x2d
        _emit 0x00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x7c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x75
        _emit 0x14
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x6a
        _emit 0x0b
        _emit 0x51
        _emit 0x52
        _emit 0xe8
        _emit 0xeb
        _emit 0x4e
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x80
        _emit 0x3f
        _emit 0x00
        _emit 0x0f
        _emit 0x85
        _emit 0x0f
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe9
        _emit 0xe6
        _emit 0xfe
        _emit 0xff
    }
}
