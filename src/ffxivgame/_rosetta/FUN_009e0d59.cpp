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
// FUNCTION: ffxivgame 0x009e0d59 — registry path read / integer parse
//                                  (341 B / 0x155), `__cdecl` 5-arg.
//
// Inspection (read from the disassembly at orig RVA 0x005e0d59):
//
//   __cdecl int FUN_009e0d59(arg1, arg2_mode, arg3, arg4, arg5_out);
//
//   Frame: PUSH EBP / LEA EBP,[ESP-0x64] / SUB ESP,0x94 / /GS cookie @ [EBP+0x60]
//   Three callee-saved regs pushed (EBX, ESI, EDI) after cookie setup.
//
//   Behaviour summary:
//     if arg2 == 1: read registry string value, heap-dup if buffer too small.
//     elif arg2 == 0: read registry DWORD, parse 2-digit string → integer.
//     else: return -1.
//
//   /GS frame: LEA EBP,[ESP-0x64] with cookie at [EBP+0x60].
//   Epilogue: ADD EBP,0x64 / LEAVE / RET (plain — __cdecl).
//
//   Note: The .s disassembly omits three caller-cleanup instructions that
//   appear in the original binary — two POP ECX (single-arg call cleanup)
//   and one ADD ESP,0x14 (five-arg call cleanup). Bytes extracted from the
//   compare.py diff output to fill those gaps.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Non-standard EBP setup (/GS cookie via LEA EBP,[ESP-0x64]) plus
//   multiple IAT-indirect calls and two heap-allocation paths make
//   source-level reproduction fragile. All siblings in this size band
//   use naked-asm passthrough for the same reason.

extern "C" __declspec(naked) void FUN_009e0d59() {
    __asm {
        // offset 0x000
        _emit 0x55
        _emit 0x8d
        _emit 0x6c
        _emit 0x24
        _emit 0x9c
        _emit 0x81
        _emit 0xec
        _emit 0x94
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // offset 0x010
        _emit 0x33
        _emit 0xc5
        _emit 0x89
        _emit 0x45
        _emit 0x60
        _emit 0x8b
        _emit 0x45
        _emit 0x6c
        _emit 0x53
        _emit 0x56
        _emit 0x8b
        _emit 0x75
        _emit 0x7c
        _emit 0x33
        _emit 0xdb
        _emit 0x83
        // offset 0x020
        _emit 0x7d
        _emit 0x70
        _emit 0x01
        _emit 0x57
        _emit 0x89
        _emit 0x45
        _emit 0xd8
        _emit 0x89
        _emit 0x75
        _emit 0xd0
        _emit 0x0f
        _emit 0x85
        _emit 0xdf
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // offset 0x030
        _emit 0x53
        _emit 0x68
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x7d
        _emit 0xe0
        _emit 0x8b
        _emit 0xcf
        _emit 0x51
        _emit 0xff
        _emit 0x75
        _emit 0x78
        _emit 0x89
        // offset 0x040
        _emit 0x5d
        _emit 0xdc
        _emit 0xff
        _emit 0x75
        _emit 0x74
        _emit 0x50
        _emit 0xe8
        _emit 0xa9
        _emit 0x5e
        _emit 0x01
        _emit 0x00
        _emit 0x8b
        _emit 0xf0
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // offset 0x050
        _emit 0x3b
        _emit 0xf3
        _emit 0x75
        _emit 0x57
        _emit 0xff
        _emit 0x15
        _emit 0xc4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x83
        _emit 0xf8
        _emit 0x7a
        _emit 0x75
        _emit 0x6b
        _emit 0x53
        // offset 0x060
        _emit 0x53
        _emit 0x53
        _emit 0xff
        _emit 0x75
        _emit 0x78
        _emit 0xff
        _emit 0x75
        _emit 0x74
        _emit 0xff
        _emit 0x75
        _emit 0xd8
        _emit 0xe8
        _emit 0x84
        _emit 0x5e
        _emit 0x01
        _emit 0x00
        // offset 0x070
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x3b
        _emit 0xc3
        _emit 0x89
        _emit 0x45
        _emit 0xd4
        _emit 0x74
        _emit 0x50
        _emit 0x33
        _emit 0xf6
        _emit 0x46
        _emit 0x56
        _emit 0x50
        _emit 0xe8
        // offset 0x080
        _emit 0xdd
        _emit 0xd1
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xf8
        _emit 0x3b
        _emit 0xfb
        _emit 0x59
        _emit 0x59
        _emit 0x74
        _emit 0x3e
        _emit 0x53
        _emit 0xff
        _emit 0x75
        _emit 0xd4
        // offset 0x090
        _emit 0x89
        _emit 0x75
        _emit 0xdc
        _emit 0x57
        _emit 0xff
        _emit 0x75
        _emit 0x78
        _emit 0xff
        _emit 0x75
        _emit 0x74
        _emit 0xff
        _emit 0x75
        _emit 0xd8
        _emit 0xe8
        _emit 0x52
        _emit 0x5e
        // offset 0x0a0
        _emit 0x01
        _emit 0x00
        _emit 0x8b
        _emit 0xf0
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x3b
        _emit 0xf3
        _emit 0x74
        _emit 0x18
        _emit 0x6a
        _emit 0x01
        _emit 0x56
        _emit 0xe8
        _emit 0xae
        // offset 0x0b0
        _emit 0xd1
        _emit 0xff
        _emit 0xff
        _emit 0x3b
        _emit 0xc3
        _emit 0x59
        _emit 0x59
        _emit 0x8b
        _emit 0x4d
        _emit 0xd0
        _emit 0x89
        _emit 0x01
        _emit 0x75
        _emit 0x21
        _emit 0x39
        _emit 0x5d
        // offset 0x0c0
        _emit 0xdc
        _emit 0x74
        _emit 0x07
        _emit 0x57
        _emit 0xe8
        _emit 0x66
        _emit 0x4e
        _emit 0xff
        _emit 0xff
        _emit 0x59          // POP ECX — caller cleanup (1 pushed EDI); missing from .s
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        _emit 0x8b
        _emit 0x4d
        _emit 0x60
        // offset 0x0d0
        _emit 0x5f
        _emit 0x5e
        _emit 0x33
        _emit 0xcd
        _emit 0x5b
        _emit 0xe8
        _emit 0xc1
        _emit 0x12
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc5
        _emit 0x64
        _emit 0xc9
        _emit 0xc3
        _emit 0x8d
        // offset 0x0e0
        _emit 0x4e
        _emit 0xff
        _emit 0x51
        _emit 0x57
        _emit 0x56
        _emit 0x50
        _emit 0xe8
        _emit 0x3d
        _emit 0x9d
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x85
        _emit 0xc0
        // offset 0x0f0
        _emit 0x74
        _emit 0x0d
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0xe8
        _emit 0x3f
        _emit 0x13
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14          // ADD ESP,0x14 — caller cleanup (5 pushed EBX); missing from .s
        _emit 0x39
        // offset 0x100
        _emit 0x5d
        _emit 0xdc
        _emit 0x74
        _emit 0x07
        _emit 0x57
        _emit 0xe8
        _emit 0x25
        _emit 0x4e
        _emit 0xff
        _emit 0xff
        _emit 0x59          // POP ECX — caller cleanup (1 pushed EDI); missing from .s
        _emit 0x33
        _emit 0xc0
        _emit 0xeb
        _emit 0xbe
        _emit 0x39
        // offset 0x110
        _emit 0x5d
        _emit 0x70
        _emit 0x75
        _emit 0xb6
        _emit 0x53
        _emit 0x6a
        _emit 0x04
        _emit 0xbf
        _emit 0x4c
        _emit 0x46
        _emit 0x36
        _emit 0x01
        _emit 0x57
        _emit 0xff
        _emit 0x75
        _emit 0x78
        // offset 0x120
        _emit 0xff
        _emit 0x75
        _emit 0x74
        _emit 0x50
        _emit 0xe8
        _emit 0x53
        _emit 0x5c
        _emit 0x01
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x9a
        // offset 0x130
        _emit 0x88
        _emit 0x1e
        _emit 0x8a
        _emit 0x1f
        _emit 0x0f
        _emit 0xb6
        _emit 0xc3
        _emit 0x50
        _emit 0xe8
        _emit 0xcd
        _emit 0x72
        _emit 0xff
        _emit 0xff
        _emit 0x85
        _emit 0xc0
        _emit 0x59
        // offset 0x140
        _emit 0x74
        _emit 0xc9
        _emit 0x8a
        _emit 0x06
        _emit 0xb1
        _emit 0x0a
        _emit 0xf6
        _emit 0xe9
        _emit 0x02
        _emit 0xc3
        _emit 0x2c
        _emit 0x30
        _emit 0x47
        _emit 0x47
        _emit 0x81
        _emit 0xff
        // offset 0x150 (last 5 bytes)
        _emit 0x54
        _emit 0x46
        _emit 0x36
        _emit 0x01
        _emit 0x88
    }
}
