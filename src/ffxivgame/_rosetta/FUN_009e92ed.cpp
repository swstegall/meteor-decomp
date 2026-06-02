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
// FUNCTION: ffxivgame 0x009e92ed — fd-validity check and file-handle lookup
//                                  (215 B / 0xd7, __cdecl, SEH frame).
//
// Asm shape (read from build/pe-layout/ffxivgame/text.bin @ +0x5e82ed,
// 215 bytes — RVA 0x005e92ed..0x005e93c4):
//
//   int __cdecl FUN_009e92ed(int fd);
//
// The function validates an integer file descriptor against the CRT fd
// table, sets errno=EBADF (9) and returns -1 on any failure, or calls
// helper functions to operate on the open fd and returns the result.
//
// SEH prolog:  PUSH 0x10 / PUSH scope_table / CALL _EH_prolog
// SEH epilog:  CALL cleanup_helper / MOV EAX, local / CALL _EH_epilog / RET
//
// Reconstruction strategy — naked asm byte passthrough.
//
//   The MSVC 2005 SEH frame shape (EH3-style prolog with scope_table VA,
//   state tracking via [ebp-4], and the two-level epilog unwinder) plus
//   the precise register allocation across the validity-check / bit-test
//   path are compiler-emitted shapes that depend on the exact locals
//   layout and the linked scope table. Naked asm passthrough is the
//   safe path.

extern "C" __declspec(naked) void FUN_009e92ed() {
    __asm {
        _emit 0x6a
        _emit 0x10
        _emit 0x68
        _emit 0x08
        _emit 0xd6
        _emit 0x22
        _emit 0x01
        _emit 0xe8
        _emit 0xf7
        _emit 0x51
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        _emit 0x83
        _emit 0xf8
        _emit 0xfe
        _emit 0x75
        _emit 0x13
        _emit 0xe8
        _emit 0x41
        _emit 0x0a
        _emit 0xff
        _emit 0xff
        _emit 0xc7
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        _emit 0xe9
        _emit 0xaa
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xdb
        _emit 0x3b
        _emit 0xc3
        _emit 0x7c
        _emit 0x08
        _emit 0x3b
        _emit 0x05
        _emit 0xdc
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        _emit 0x72
        _emit 0x1a
        _emit 0xe8
        _emit 0x20
        _emit 0x0a
        _emit 0xff
        _emit 0xff
        _emit 0xc7
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0xe8
        _emit 0x59
        _emit 0x8f
        _emit 0xfe
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0xeb
        _emit 0xd0
        _emit 0x8b
        _emit 0xc8
        _emit 0xc1
        _emit 0xf9
        _emit 0x05
        _emit 0x8d
        _emit 0x3c
        _emit 0x8d
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        _emit 0x8b
        _emit 0xf0
        _emit 0x83
        _emit 0xe6
        _emit 0x1f
        _emit 0xc1
        _emit 0xe6
        _emit 0x06
        _emit 0x8b
        _emit 0x0f
        _emit 0x0f
        _emit 0xb6
        _emit 0x4c
        _emit 0x0e
        _emit 0x04
        _emit 0x83
        _emit 0xe1
        _emit 0x01
        _emit 0x74
        _emit 0xc6
        _emit 0x50
        _emit 0xe8
        _emit 0x8b
        _emit 0xe4
        _emit 0xff
        _emit 0xff
        _emit 0x59
        _emit 0x89
        _emit 0x5d
        _emit 0xfc
        _emit 0x8b
        _emit 0x07
        _emit 0xf6
        _emit 0x44
        _emit 0x06
        _emit 0x04
        _emit 0x01
        _emit 0x74
        _emit 0x31
        _emit 0xff
        _emit 0x75
        _emit 0x08
        _emit 0xe8
        _emit 0x05
        _emit 0xe4
        _emit 0xff
        _emit 0xff
        _emit 0x59
        _emit 0x50
        _emit 0xff
        _emit 0x15
        _emit 0x40
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x0b
        _emit 0xff
        _emit 0x15
        _emit 0xc4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x89
        _emit 0x45
        _emit 0xe4
        _emit 0xeb
        _emit 0x03
        _emit 0x89
        _emit 0x5d
        _emit 0xe4
        _emit 0x39
        _emit 0x5d
        _emit 0xe4
        _emit 0x74
        _emit 0x19
        _emit 0xe8
        _emit 0xbf
        _emit 0x09
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x4d
        _emit 0xe4
        _emit 0x89
        _emit 0x08
        _emit 0xe8
        _emit 0xa2
        _emit 0x09
        _emit 0xff
        _emit 0xff
        _emit 0xc7
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0x4d
        _emit 0xe4
        _emit 0xff
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x45
        _emit 0xe4
        _emit 0xe8
        _emit 0x72
        _emit 0x51
        _emit 0xff
        _emit 0xff
        _emit 0xc3
    }
}
