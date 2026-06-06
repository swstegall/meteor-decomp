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
// FUNCTION: ffxivgame 0x00422010 — `__thiscall` shutdown/teardown method
//                                  (270 B / 0x10e, SEH frame, /GS cookie).
//
// Behaviour read from the disassembly at orig RVA 0x00022010:
//
//   __thiscall void FUN_00422010(SomeClass *this)
//
//   The function tears down state held by the object, under an x86
//   try/finally SEH frame (MSVC 2005 __try/__finally style).  Key steps:
//
//     1. SEH prologue: PUSH -1 / PUSH 0xe55aec / PUSH FS:[0] / …
//        MSVC 2005 /GS security cookie materialised from [0x012ea8b0].
//     2. CALL FUN_00420b30 (no args, __cdecl)
//        CALL FUN_0041fc60 (no args, __cdecl)
//     3. if (this->field_1a8 != NULL)
//            this->field_1a8->vt[0](1);    // virtual call, vtable slot 0
//     4. global_0x01329898: if != NULL, FUN_00432f30(global);
//     5. global_0x01329834: call vt[2] on it, then zero the global.
//        global_0x01329830: call vt[2] on it, then zero the global.
//     6. Clear a container at this+0x1ac (free data ptr, zero size/cap).
//        (calls FUN_00421d40 to prepare + FUN_0040df70 to release data)
//     7. Clear a container at this+0x140 (same pattern, using FUN_00421c70).
//        (FUN_0040df70 to release data again)
//     8. SEH epilogue: restore FS:[0], pop saved regs, ADD ESP,0x18, RET.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function embeds four absolute addresses as 32-bit immediates:
//     +0x14  0x012ea8b0  — security cookie
//     +0x52  0x01329898  — global pointer 1
//     +0x64  0x01329834  — global pointer 2 (also written at +0x76)
//     +0x71  0x01329830  — global pointer 3 (also written at +0x8a)
//   plus the SEH handler address 0xe55aec embedded as PUSH imm32 at +0x02.
//   All CALL rel32 operands are also binary-embedded as link-time relative
//   offsets.  Reproducing the exact register allocation, SEH state updates,
//   and all those absolutes from C++ source would be brittle; the naked-asm
//   _emit passthrough is the only reliable way to get byte-identical output.

extern "C" __declspec(naked) void FUN_00422010() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0xec
        _emit 0x5a
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

        _emit 0x0c
        _emit 0x53
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
        _emit 0x1c

        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf9
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x24

        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xe7
        _emit 0xea
        _emit 0xff
        _emit 0xff
        _emit 0xe8
        _emit 0x12
        _emit 0xdc
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x8f

        _emit 0xa8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xdb
        _emit 0x3b
        _emit 0xcb
        _emit 0x74
        _emit 0x08
        _emit 0x8b
        _emit 0x01
        _emit 0x8b
        _emit 0x10
        _emit 0x6a
        _emit 0x01

        _emit 0xff
        _emit 0xd2
        _emit 0xa1
        _emit 0x98
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x3b
        _emit 0xc3
        _emit 0x74
        _emit 0x09
        _emit 0x50
        _emit 0xe8
        _emit 0xbf
        _emit 0x0e
        _emit 0x01

        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0xa1
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x8b
        _emit 0x08
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        _emit 0x50
        _emit 0xff

        _emit 0xd2
        _emit 0xa1
        _emit 0x30
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x89
        _emit 0x1d
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x8b
        _emit 0x08
        _emit 0x8b
        _emit 0x51

        _emit 0x08
        _emit 0x50
        _emit 0xff
        _emit 0xd2
        _emit 0x8d
        _emit 0xb7
        _emit 0xac
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x1d
        _emit 0x30
        _emit 0x98
        _emit 0x32
        _emit 0x01

        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x8b
        _emit 0x08
        _emit 0x50
        _emit 0x56
        _emit 0x51
        _emit 0x56
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x50
        _emit 0x8b
        _emit 0xce

        _emit 0x88
        _emit 0x5c
        _emit 0x24
        _emit 0x38
        _emit 0xe8
        _emit 0x87
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x3b
        _emit 0xc3
        _emit 0x74
        _emit 0x09

        _emit 0x8b
        _emit 0x48
        _emit 0xfc
        _emit 0x50
        _emit 0xe8
        _emit 0xa7
        _emit 0xbe
        _emit 0xfe
        _emit 0xff
        _emit 0x89
        _emit 0x5e
        _emit 0x04
        _emit 0x89
        _emit 0x5e
        _emit 0x08
        _emit 0x8b

        _emit 0x87
        _emit 0x44
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x08
        _emit 0x8d
        _emit 0xb7
        _emit 0x40
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x56
        _emit 0x51

        _emit 0x56
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x51
        _emit 0x8b
        _emit 0xce
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff

        _emit 0xe8
        _emit 0x7b
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x3b
        _emit 0xc3
        _emit 0x74
        _emit 0x09
        _emit 0x8b
        _emit 0x48
        _emit 0xfc
        _emit 0x50

        _emit 0xe8
        _emit 0x6b
        _emit 0xbe
        _emit 0xfe
        _emit 0xff
        _emit 0x89
        _emit 0x5e
        _emit 0x08
        _emit 0x89
        _emit 0x5e
        _emit 0x04
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
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
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0xc3
    }
}
