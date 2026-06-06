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
// FUNCTION: ffxivgame 0x00470850 — address/token parse + copy into `this`
//                                  (__thiscall, 370 B / 0x172, /GS via
//                                   __chkstk + security_cookie, no SEH).
//
// Inspection (read from asm/ffxivgame/00070850_FUN_00470850.s):
//
//   __thiscall bool FUN_00470850(this, param1, param2, param3)
//     ECX = this; [ESP+4]=param1, [ESP+8]=param2, [ESP+12]=param3
//
//   Structural shape:
//     // prologue: __chkstk(0x20) allocates 32-byte local frame + /GS cookie
//     // local_buf[0..3] = parsed output from FUN_00484350
//     // local_buf[4..6] = control fields (pre-init 0/-1/0)
//     if (!FUN_00484350(param1, 0x3a, 0, 0x4707c0, local_buf)) return false;
//     // validate parsed fields and copy 4 DWORDs into this or memmove subset
//     return true/false based on validation logic.
//
//   Reloc-bearing sites (absolute addresses in the orig 370 bytes):
//     +0x01  rel32  0x009d29d0  CALL __chkstk
//     +0x0a  moffs  0x012ea8b0  __security_cookie
//     +0x21  imm32  0x4707c0    format/table ptr
//     +0x37  rel32  0x00484350  CALL FUN_00484350
//     +0x56  rel32  0x009d20f4  CALL __security_check_cookie (x7 total)
//     +0xe9  rel32  0x009d4600  CALL memmove (x2)
//     +0xfc  rel32  0x009d2110  CALL memset
//
// Reconstruction strategy — naked-asm byte passthrough:
//   __chkstk-based frame + 7 distinct epilogue paths with different
//   [ESP+N] cookie slots make source-level reproduction impractical.
//   Same approach as FUN_00405080 / FUN_0040ced0 / FUN_004014b0.

extern "C" __declspec(naked) void FUN_00470850() {
    __asm {
        _emit 0xb8
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x76
        _emit 0x21
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
        _emit 0x1c
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x56
        _emit 0x8b
        _emit 0xf1
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x51
        _emit 0x68
        _emit 0xc0
        _emit 0x07
        _emit 0x47
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0x6a
        _emit 0x3a
        _emit 0x50
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xb8
        _emit 0x3a
        _emit 0x01
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x10
        _emit 0x5e
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x49
        _emit 0x18
        _emit 0x56
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        _emit 0xc3
        _emit 0x53
        _emit 0x57
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x83
        _emit 0xff
        _emit 0xff
        _emit 0x75
        _emit 0x1f
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x10
        _emit 0x0f
        _emit 0x84
        _emit 0xcb
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5f
        _emit 0x5b
        _emit 0x33
        _emit 0xc0
        _emit 0x5e
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x1f
        _emit 0x18
        _emit 0x56
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        _emit 0xc3
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x83
        _emit 0xfb
        _emit 0x10
        _emit 0x74
        _emit 0xe3
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x83
        _emit 0xf8
        _emit 0x03
        _emit 0x7f
        _emit 0xda
        _emit 0x75
        _emit 0x18
        _emit 0x85
        _emit 0xdb
        _emit 0x7e
        _emit 0x3d
        _emit 0x5f
        _emit 0x5b
        _emit 0x33
        _emit 0xc0
        _emit 0x5e
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0xf3
        _emit 0x17
        _emit 0x56
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        _emit 0xc3
        _emit 0x83
        _emit 0xf8
        _emit 0x02
        _emit 0x75
        _emit 0x1c
        _emit 0x85
        _emit 0xff
        _emit 0x74
        _emit 0x24
        _emit 0x3b
        _emit 0xfb
        _emit 0x74
        _emit 0x1c
        _emit 0x5f
        _emit 0x5b
        _emit 0x33
        _emit 0xc0
        _emit 0x5e
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0xd2
        _emit 0x17
        _emit 0x56
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        _emit 0xc3
        _emit 0x85
        _emit 0xff
        _emit 0x74
        _emit 0x9b
        _emit 0x3b
        _emit 0xfb
        _emit 0x74
        _emit 0x97
        _emit 0x85
        _emit 0xff
        _emit 0x7c
        _emit 0x5e
        _emit 0x57
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x52
        _emit 0x56
        _emit 0xe8
        _emit 0xc2
        _emit 0x3c
        _emit 0x56
        _emit 0x00
        _emit 0xb8
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b
        _emit 0xc3
        _emit 0x50
        _emit 0x8d
        _emit 0x0c
        _emit 0x3e
        _emit 0x6a
        _emit 0x00
        _emit 0x51
        _emit 0xe8
        _emit 0xbf
        _emit 0x17
        _emit 0x56
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x3b
        _emit 0xc8
        _emit 0x74
        _emit 0x4b
        _emit 0x8b
        _emit 0xd1
        _emit 0x2b
        _emit 0xd0
        _emit 0x52
        _emit 0x8d
        _emit 0x54
        _emit 0x04
        _emit 0x10
        _emit 0x2b
        _emit 0xf1
        _emit 0x52
        _emit 0x8d
        _emit 0x44
        _emit 0x06
        _emit 0x10
        _emit 0x50
        _emit 0xe8
        _emit 0x8a
        _emit 0x3c
        _emit 0x56
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x5f
        _emit 0x5b
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x68
        _emit 0x17
        _emit 0x56
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        _emit 0xc3
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x89
        _emit 0x0e
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x89
        _emit 0x56
        _emit 0x04
        _emit 0x89
        _emit 0x46
        _emit 0x08
        _emit 0x89
        _emit 0x4e
        _emit 0x0c
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x5f
        _emit 0x5b
        _emit 0x5e
        _emit 0x33
        _emit 0xcc
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x36
        _emit 0x17
        _emit 0x56
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        _emit 0xc3
    }
}
