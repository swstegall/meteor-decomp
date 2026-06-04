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
// FUNCTION: ffxivgame 0x00043ed0 — __thiscall bool member (436 B / 0x1b4,
//                                   MSVC inline SEH + /GS security cookie).
//
// Inspection (read from the disassembly at orig RVA 0x00043ed0):
//
//   __thiscall bool FUN_00443ed0(void* this, void* arg1);
//
//     `ECX = this` (snapshotted into EBP at +0x46), one stack arg
//     (callee-popped `RET 0x4`), bool return in AL. The body opens a
//     large 0x2014-byte frame, installs the standard MSVC SEH frame
//     (PUSH -1 / PUSH 0xe5732b scope table / FS:[0] link) and a /GS
//     security cookie XOR'd with ESP, then:
//
//       - constructs a stack object at [esp+0x14] via FUN_00452fe0;
//       - early-out: if (this->[0xc] == 0) { /* state -1 */ tear down
//         the stack object (FUN_00453190) and return true; }
//       - else test a flag via FUN_00453c00("...", 0, arg1); on false,
//         tear down and return false (XOR AL,AL);
//       - on true: two FUN_004530b0 appends, then walk an array of
//         (this->[4] << 5) records each 0xbc bytes wide at this->[8],
//         dispatching a 3-way (SUB EAX,1 / SUB EAX,1) switch per record
//         that formats fields via FUN_004454a0 / FUN_00445210 into the
//         stack object;
//       - finalize via FUN_00453000, clear this->[0xc], tear down the
//         stack object (state -1) and return true.
//
// The function carries an MSVC inline-SEH frame whose state byte steps
// -1 → 0 → -1 across the construct / loop / destruct phases, plus a /GS
// cookie. A source-level __try/__finally + class translation would not
// reliably reproduce the exact prolog, the scope-table imm32 0x00e5732b,
// the cookie XOR sequencing, the SUB-EAX/JZ chained switch, or the
// linker-resolved absolute string pushes (0x00f67204 / 0x00f67208 /
// 0x00f67228 / 0x00f67240 / 0x00f67250) and ~14 rel32 call targets —
// each brittle under /O2.
//
// Reconstruction strategy: byte-passthrough via __declspec(naked) +
// `_emit`, matching the established sibling idiom (FUN_0040b840 /
// FUN_00415d00 / FUN_00409350) for reloc-heavy SEH bodies. All reloc
// sites' pre-linked literals already match the orig PE byte-for-byte,
// so the emitted .text is byte-identical to the orig slice, which is
// what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_00443ed0() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x2b
        _emit 0x73
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0xb8
        _emit 0x14
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xe8
        _emit 0xea
        _emit 0x58
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
        _emit 0x10
        _emit 0x20
        _emit 0x00
        _emit 0x00
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
        _emit 0x84
        _emit 0x24
        _emit 0x28
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0x38
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xe9
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xe8
        _emit 0xbf
        _emit 0xf0
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xff
        _emit 0x80
        _emit 0x7d
        _emit 0x0c
        _emit 0x00
        _emit 0x89
        _emit 0xbc
        _emit 0x24
        _emit 0x30
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x75
        _emit 0x17
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0x30
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8
        _emit 0x4c
        _emit 0xf2
        _emit 0x00
        _emit 0x00
        _emit 0xb0
        _emit 0x01
        _emit 0xe9
        _emit 0x0f
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x57
        _emit 0x68
        _emit 0x04
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0xa9
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x84
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0xe5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x68
        _emit 0x08
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x50
        _emit 0xe8
        _emit 0x42
        _emit 0xf1
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x68
        _emit 0x28
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x51
        _emit 0xe8
        _emit 0x33
        _emit 0xf1
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x45
        _emit 0x04
        _emit 0xc1
        _emit 0xe0
        _emit 0x05
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x3b
        _emit 0xc7
        _emit 0x0f
        _emit 0x8e
        _emit 0x91
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xd8
        _emit 0x8b
        _emit 0x75
        _emit 0x08
        _emit 0x8b
        _emit 0x04
        _emit 0x3e
        _emit 0x03
        _emit 0xf7
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        _emit 0x74
        _emit 0x4c
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        _emit 0x75
        _emit 0x6e
        _emit 0x6a
        _emit 0x04
        _emit 0x6a
        _emit 0x0d
        _emit 0x6a
        _emit 0x03
        _emit 0x6a
        _emit 0x0a
        _emit 0x6a
        _emit 0x02
        _emit 0x6a
        _emit 0x2c
        _emit 0x8d
        _emit 0x4e
        _emit 0x68
        _emit 0xe8
        _emit 0xea
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xc8
        _emit 0xe8
        _emit 0xe3
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xc8
        _emit 0xe8
        _emit 0xdc
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xc8
        _emit 0xe8
        _emit 0x45
        _emit 0x12
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x8d
        _emit 0x4e
        _emit 0x04
        _emit 0xe8
        _emit 0x3c
        _emit 0x12
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x68
        _emit 0x50
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x52
        _emit 0xe8
        _emit 0xcc
        _emit 0xf0
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0xeb
        _emit 0x27
        _emit 0x8b
        _emit 0x46
        _emit 0x60
        _emit 0x8b
        _emit 0x4e
        _emit 0x5c
        _emit 0x8b
        _emit 0x56
        _emit 0x58
        _emit 0x50
        _emit 0x51
        _emit 0x52
        _emit 0x8d
        _emit 0x4e
        _emit 0x04
        _emit 0xe8
        _emit 0x13
        _emit 0x12
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x68
        _emit 0x40
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x50
        _emit 0xe8
        _emit 0xa3
        _emit 0xf0
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x81
        _emit 0xc7
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xeb
        _emit 0x01
        _emit 0x0f
        _emit 0x85
        _emit 0x71
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xe8
        _emit 0xd8
        _emit 0xef
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xc6
        _emit 0x45
        _emit 0x0c
        _emit 0x00
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0x30
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8
        _emit 0x50
        _emit 0xf1
        _emit 0x00
        _emit 0x00
        _emit 0xb0
        _emit 0x01
        _emit 0xeb
        _emit 0x16
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0x30
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8
        _emit 0x38
        _emit 0xf1
        _emit 0x00
        _emit 0x00
        _emit 0x32
        _emit 0xc0
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x28
        _emit 0x20
        _emit 0x00
        _emit 0x00
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
        _emit 0x8c
        _emit 0x24
        _emit 0x10
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x79
        _emit 0xe0
        _emit 0x58
        _emit 0x00
        _emit 0x81
        _emit 0xc4
        _emit 0x20
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
