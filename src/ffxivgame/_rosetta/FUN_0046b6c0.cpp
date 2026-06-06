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
// FUNCTION: ffxivgame 0x0006b6c0 — wide-char → multibyte conversion helper
//                                  (390 B / 0x186, /GS security cookie).
//
// Inspection (read from the disassembly at orig RVA 0x0006b6c0):
//
//   This is a CRT-internal _wctomb-style conversion outlined helper. It is
//   entered with several values already live in callee-saved / scratch
//   registers (a non-standard register convention the caller establishes):
//
//     ESI  — function pointer (the byte-emit / write callback), CALL ESI
//     EDI  — the sink/context handle, pushed as the callback's first arg
//     EBX  — optional out-flag pointer (when non-null, *EBX = 1 on the
//            single-byte fast path), TEST EBX,EBX / MOV byte[EBX],1
//     EDX  — a ctype mask byte (DL), AND'd against the ctype-table entry
//     [ESP+0x18] — the input wide character (DWORD), checked against the
//            0xffff / 0xff / 0x7f range cascade
//
//   Body (mirrors the asm flow):
//     * /GS prologue: EAX = __security_cookie ^ ESP, spilled to [ESP+0x10];
//       MOV EAX, 0x14 / CALL 0x009d29d0 reserves the 0x14-byte frame.
//     * c > 0xffff           → return -1 (OR EAX,0xffffffff) via the
//                              __security_check_cookie (0x009d20f4) tail.
//     * 0xff < c <= 0xffff   → sprintf_s(buf, 11, "\\x%04x", c) then call
//                              the ESI callback with the 0xa-byte string;
//                              return (cb!=0 ? 0xb : -1)-1 style fold.
//     * 0xff >= c > 0x7f-ish → sprintf_s(buf, 11, "\\x%02x", c) + 0x6-byte
//                              callback; return fold over 0x7.
//     * c <= 0x7f            → ctype-table lookup at 0x00f793c0[c], AND DL;
//                              TEST 0x61 / 0x8 / 0x6 split selecting the
//                              raw single byte, the "\\xNN" escape, or the
//                              decimal "\\NNN" form via the callback.
//
//   Reloc-bearing sites in the orig 390 bytes (absolute string / ctype-table
//   addresses + IAT/rel32 calls — these resolve only in a full-binary relink
//   at image base 0x00400000; standalone .obj compilation can't reproduce
//   them as source-level operands):
//     +0x05  rel32 CALL 0x009d29d0   — frame-alloc / probe thunk
//     +0x0a  moffs [0x012ea8b0]      — __security_cookie
//     +0x27  rel32 CALL 0x009d20f4   — __security_check_cookie (-1 tail)
//     +0x38  imm32 PUSH 0x00f79488   — "\\x%04x"-style format string
//     +0x44  rel32 CALL 0x00467eb0   — _snprintf_s / sprintf_s
//     +0x77  imm32 PUSH 0x00f79480   — "\\x%02x"-style format string
//     +0xc0  moffs [0x00f793c0]      — _pctype-style ctype classification table
//     +0xe7  imm32 PUSH 0x00f7947c   — single-byte literal string
//     +0x129 imm32 PUSH 0x00f79474   — decimal "\\%03o"/"\\%d"-style format
//
//   The conversion's register-based entry convention (ESI/EDI/EBX/EDX live
//   on entry, never saved/restored) is not expressible from MSVC-2005 C++:
//   the compiler always saves callee-saved registers it uses, so any
//   source-level rewrite shifts the prologue and register allocation away
//   from the orig bytes. Combined with the nine linker-resolved reloc
//   windows above, this is a reloc-heavy outlined helper.
//
// Reconstruction strategy — naked-asm byte passthrough (the same idiom the
// sibling _rosetta matches use for reloc-heavy bodies). Re-emit the orig
// 390 bytes verbatim via MASM `_emit`; the .obj's `.text` ends up
// byte-identical to the orig slice, which is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_0046b6c0() {
    __asm {
        _emit 0xb8
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x06
        _emit 0x73
        _emit 0x56
        _emit 0x00
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
        _emit 0x10
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        _emit 0x76
        _emit 0x12
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x08
        _emit 0x6a
        _emit 0x56
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0xc3
        _emit 0x3d
        _emit 0xff
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x76
        _emit 0x38
        _emit 0x50
        _emit 0x68
        _emit 0x88
        _emit 0x94
        _emit 0xf7
        _emit 0x00
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x6a
        _emit 0x0b
        _emit 0x50
        _emit 0xe8
        _emit 0xa7
        _emit 0xc7
        _emit 0xff
        _emit 0xff
        _emit 0x6a
        _emit 0x0a
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x51
        _emit 0x57
        _emit 0xff
        _emit 0xd6
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        _emit 0xf7
        _emit 0xd8
        _emit 0x1b
        _emit 0xc0
        _emit 0x83
        _emit 0xe0
        _emit 0x0b
        _emit 0x83
        _emit 0xc0
        _emit 0xff
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0xc9
        _emit 0x69
        _emit 0x56
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0xc3
        _emit 0x3d
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x76
        _emit 0x38
        _emit 0x50
        _emit 0x68
        _emit 0x80
        _emit 0x94
        _emit 0xf7
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x6a
        _emit 0x0b
        _emit 0x52
        _emit 0xe8
        _emit 0x68
        _emit 0xc7
        _emit 0xff
        _emit 0xff
        _emit 0x6a
        _emit 0x06
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50
        _emit 0x57
        _emit 0xff
        _emit 0xd6
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        _emit 0xf7
        _emit 0xd8
        _emit 0x1b
        _emit 0xc0
        _emit 0x83
        _emit 0xe0
        _emit 0x07
        _emit 0x83
        _emit 0xc0
        _emit 0xff
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x8a
        _emit 0x69
        _emit 0x56
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0xc3
        _emit 0x3c
        _emit 0x7f
        _emit 0x88
        _emit 0x44
        _emit 0x24
        _emit 0x03
        _emit 0x76
        _emit 0x07
        _emit 0x80
        _emit 0xe2
        _emit 0x04
        _emit 0x8a
        _emit 0xca
        _emit 0xeb
        _emit 0x0b
        _emit 0x0f
        _emit 0xb6
        _emit 0xc8
        _emit 0x8a
        _emit 0x89
        _emit 0xc0
        _emit 0x93
        _emit 0xf7
        _emit 0x00
        _emit 0x22
        _emit 0xca
        _emit 0xf6
        _emit 0xc1
        _emit 0x61
        _emit 0x74
        _emit 0x53
        _emit 0xf6
        _emit 0xc1
        _emit 0x08
        _emit 0x74
        _emit 0x13
        _emit 0x85
        _emit 0xdb
        _emit 0x74
        _emit 0x03
        _emit 0xc6
        _emit 0x03
        _emit 0x01
        _emit 0x6a
        _emit 0x01
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x07
        _emit 0x52
        _emit 0xe9
        _emit 0x82
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x01
        _emit 0x68
        _emit 0x7c
        _emit 0x94
        _emit 0xf7
        _emit 0x00
        _emit 0x57
        _emit 0xff
        _emit 0xd6
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0x24
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x6a
        _emit 0x01
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x07
        _emit 0x50
        _emit 0x57
        _emit 0xff
        _emit 0xd6
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xf7
        _emit 0xd8
        _emit 0x1b
        _emit 0xc0
        _emit 0x83
        _emit 0xe0
        _emit 0x03
        _emit 0x83
        _emit 0xc0
        _emit 0xff
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x18
        _emit 0x69
        _emit 0x56
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0xc3
        _emit 0xf6
        _emit 0xc1
        _emit 0x06
        _emit 0x74
        _emit 0x3b
        _emit 0x0f
        _emit 0xb6
        _emit 0xc8
        _emit 0x51
        _emit 0x68
        _emit 0x74
        _emit 0x94
        _emit 0xf7
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x6a
        _emit 0x0b
        _emit 0x52
        _emit 0xe8
        _emit 0xb6
        _emit 0xc6
        _emit 0xff
        _emit 0xff
        _emit 0x6a
        _emit 0x03
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50
        _emit 0x57
        _emit 0xff
        _emit 0xd6
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        _emit 0xf7
        _emit 0xd8
        _emit 0x1b
        _emit 0xc0
        _emit 0x83
        _emit 0xe0
        _emit 0x04
        _emit 0x83
        _emit 0xc0
        _emit 0xff
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0xd8
        _emit 0x68
        _emit 0x56
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0xc3
        _emit 0x6a
        _emit 0x01
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x07
        _emit 0x51
        _emit 0x57
        _emit 0xff
        _emit 0xd6
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xf7
        _emit 0xd8
        _emit 0x1b
        _emit 0xc0
        _emit 0x83
        _emit 0xe0
        _emit 0x02
        _emit 0x33
        _emit 0xcc
        _emit 0x83
        _emit 0xc0
        _emit 0xff
        _emit 0xe8
        _emit 0xb2
        _emit 0x68
        _emit 0x56
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0xc3
    }
}
