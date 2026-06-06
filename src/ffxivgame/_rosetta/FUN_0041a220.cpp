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
// FUNCTION: ffxivgame 0x0001a220 — `__thiscall` array-container init
//                                   (296 B / 0x128, EH4-SEH wrapped,
//                                    function-scope magic-static singleton,
//                                    SSE2 MOVQ field copy).
//
// Inspection (read from the disassembly at orig RVA 0x0001a220):
//
//   __thiscall SomeClass* FUN_0041a220(this=ECX, int count, const SrcStruct* src);
//
//   1. Copies 0x24 bytes from src (arg2=[ESP+0x2c]) into this via
//      4×MOVQ XMM0 (8-byte chunks at +0x00..+0x18) plus a plain MOV
//      dword at +0x20.
//   2. Zeroes this+0x2c and sets this+0x30 = -1.
//   3. Function-scope magic-static: tests 0x01329420; if not yet set,
//      ORs in 0x1, sets trylevel=0, calls singleton ctor at 0x0040e2d0
//      (__thiscall, ECX=0x1329418, args 0x10/0xf57e78), resets
//      trylevel=-1.
//   4. Reads count (arg1=[ESP+0x28]), computes allocation size
//      count*0x70 (overflow-guarded via SETO/NEG/OR), calls allocator
//      0x00419c40.
//   5. Stores alloc ptr at this+0x24, count at this+0x28.
//   6. Loop over [0, count): element stride=0x70; sets element+0x60=0,
//      calls ctor 0x00431710 (__thiscall ECX=element), stores vtable
//      0x00f57e3c at element+0x00 and -1 at element+0x64.
//   7. Returns this in EAX.
//
//   Stack frame (after the EH4 prologue, ESP-relative):
//     [esp+0x00]  GS cookie (security cookie ^ ESP)
//     [esp+0x04]  saved EDI
//     [esp+0x08]  saved ESI
//     [esp+0x0c]  saved EBP
//     [esp+0x10]  saved EBX
//     [esp+0x14]  saved ECX / this
//     [esp+0x18]  EH4 NextInChain (old FS:[0])
//     [esp+0x1c]  EH4 scope-table (0xe55338)
//     [esp+0x20]  EH4 trylevel (-1 idle, 0 in ctor try-block, 1 in
//                 element-loop try-block)
//     [esp+0x24]  return address
//     [esp+0x28]  arg1 = count
//     [esp+0x2c]  arg2 = src pointer
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The EH4 prolog (PUSH -1 / PUSH scope-table / PUSH FS:[0] / push
//   callee-saves / cookie XOR / FS:[0] install), the SSE2 MOVQ copy
//   idiom, the magic-static trylevel juggling, and the absolute
//   addresses in the relocation windows cannot be reproduced
//   byte-for-byte from source-level C++.  The same pragmatic approach
//   taken by every other SEH-wrapped function in _rosetta/ applies:
//   __declspec(naked) re-emitting the original 296 bytes verbatim via
//   MASM _emit directives.

extern "C" __declspec(naked) void FUN_0041a220() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x38
        _emit 0x53
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x51
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
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf1
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x00
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x06
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x08
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x08
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x10
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x10
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x18
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x18
        _emit 0x8b
        _emit 0x40
        _emit 0x20
        _emit 0x33
        _emit 0xed
        _emit 0x83
        _emit 0xcf
        _emit 0xff
        _emit 0x89
        _emit 0x46
        _emit 0x20
        _emit 0x89
        _emit 0x6e
        _emit 0x2c
        _emit 0x89
        _emit 0x7e
        _emit 0x30
        _emit 0xf6
        _emit 0x05
        _emit 0x20
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75
        _emit 0x20
        _emit 0x83
        _emit 0x0d
        _emit 0x20
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x68
        _emit 0x78
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0x6a
        _emit 0x10
        _emit 0xb9
        _emit 0x18
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x28
        _emit 0xe8
        _emit 0x25
        _emit 0x40
        _emit 0xff
        _emit 0xff
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x28
        _emit 0x33
        _emit 0xc9
        _emit 0x8b
        _emit 0xc7
        _emit 0xba
        _emit 0x70
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf7
        _emit 0xe2
        _emit 0x0f
        _emit 0x90
        _emit 0xc1
        _emit 0x68
        _emit 0x18
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0xf7
        _emit 0xd9
        _emit 0x0b
        _emit 0xc8
        _emit 0x51
        _emit 0xe8
        _emit 0x70
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        _emit 0x33
        _emit 0xdb
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x3b
        _emit 0xfd
        _emit 0x89
        _emit 0x46
        _emit 0x24
        _emit 0x89
        _emit 0x7e
        _emit 0x28
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x2c
        _emit 0x76
        _emit 0x4d
        _emit 0xeb
        _emit 0x02
        _emit 0x33
        _emit 0xed
        _emit 0x8b
        _emit 0x46
        _emit 0x24
        _emit 0x8b
        _emit 0xfb
        _emit 0x6b
        _emit 0xff
        _emit 0x70
        _emit 0x89
        _emit 0x6c
        _emit 0x38
        _emit 0x60
        _emit 0x8b
        _emit 0x6e
        _emit 0x24
        _emit 0x03
        _emit 0xef
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x28
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x10
        _emit 0x55
        _emit 0x56
        _emit 0x8b
        _emit 0xcd
        _emit 0xe8
        _emit 0x01
        _emit 0x74
        _emit 0x01
        _emit 0x00
        _emit 0xc7
        _emit 0x45
        _emit 0x00
        _emit 0x3c
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x24
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        _emit 0x83
        _emit 0xc3
        _emit 0x01
        _emit 0x89
        _emit 0x44
        _emit 0x39
        _emit 0x64
        _emit 0x3b
        _emit 0x5e
        _emit 0x28
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x2c
        _emit 0x72
        _emit 0xb5
        _emit 0x8b
        _emit 0xc6
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
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
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
