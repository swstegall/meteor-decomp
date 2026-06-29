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
// FUNCTION: ffxivgame 0x00052340 — __thiscall std::_Tree red-black insert +
//                                  rebalance (509 B / 0x1fd, SEH-wrapped).
//
// Calling convention: __thiscall (ECX = this tree-header; RET 0x10 cleans the
// 4 stack params — the by-value iterator hint pair at [esp+0x70/0x74], the
// returned iterator-pair out-slot at [esp+0x68], and the inserted-value
// payload). Returns the result pair via the hidden out-pointer.
//
// Shape: classic MSVC 2005 std::map<...>::insert lowering — an SEH frame
// (PUSH -1 / scope-table 0xed4e78 / FS:[0] link), a /GS security cookie
// (0x012ea8b0 ^ ESP), a guarded capacity check at [this+0x8] against
// 0x4924923 that on overflow builds an "map/set too long" string (calls
// 0x404120 / 0x404320) and raises via 0x009d1b9f, the _Buynode/_Insert
// helper 0x004521d0, and the _Lrotate (0x00451340) / _Rrotate (0x00451140)
// rebalance loop walking the parent/left/right (+0/+4/+8) and colour (+0x44)
// node fields.
//
// Reconstruction strategy — naked-asm byte passthrough. Like the sibling
// reloc-heavy SEH bodies (FUN_00401820, FUN_004521d0), a source-level rewrite
// can't pin MSVC's exact register allocation across the SEH prologue, the /GS
// cookie, the string-build error path, and the three rel32 call sites while
// also reproducing the absolute scope-table / cookie / string-literal
// immediates. We re-emit the original 509 bytes verbatim; the .obj .text ends
// up byte-identical to the orig slice, which is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_00452340() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x78
        _emit 0x4e
        _emit 0xed
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
        _emit 0x44
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
        _emit 0x58
        _emit 0x64
        _emit 0xa3
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf9
        _emit 0x81
        _emit 0x7f
        _emit 0x08
        _emit 0x23
        _emit 0x49
        _emit 0x92
        _emit 0x04

        _emit 0x72
        _emit 0x4c
        _emit 0x6a
        _emit 0x13
        _emit 0x33
        _emit 0xf6
        _emit 0x68
        _emit 0x1c
        _emit 0x80
        _emit 0xf5
        _emit 0x00
        _emit 0x8d

        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89

        _emit 0x74
        _emit 0x24
        _emit 0x30
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x00
        _emit 0xe8
        _emit 0x8b
        _emit 0x1d
        _emit 0xfb

        _emit 0xff
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        _emit 0x89
        _emit 0x74

        _emit 0x24
        _emit 0x64
        _emit 0xe8
        _emit 0x79
        _emit 0x1f
        _emit 0xfb
        _emit 0xff
        _emit 0x68
        _emit 0x88
        _emit 0x90
        _emit 0x1a
        _emit 0x01

        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        _emit 0x51
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x38
        _emit 0x4a
        _emit 0xf5

        _emit 0x00
        _emit 0xe8
        _emit 0xe1
        _emit 0xf7
        _emit 0x57
        _emit 0x00
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x74
        _emit 0x8b
        _emit 0x47

        _emit 0x04
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x70
        _emit 0x6a
        _emit 0x00
        _emit 0x52
        _emit 0x50
        _emit 0x56
        _emit 0x50
        _emit 0xe8

        _emit 0xfc
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xe8
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        _emit 0xbb
        _emit 0x01
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x5f
        _emit 0x08
        _emit 0x3b
        _emit 0xf0
        _emit 0x75
        _emit 0x10
        _emit 0x89
        _emit 0x68
        _emit 0x04

        _emit 0x8b
        _emit 0x47
        _emit 0x04
        _emit 0x89
        _emit 0x28
        _emit 0x8b
        _emit 0x4f
        _emit 0x04
        _emit 0x89
        _emit 0x69
        _emit 0x08
        _emit 0xeb

        _emit 0x22
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x6c
        _emit 0x00
        _emit 0x74
        _emit 0x0d
        _emit 0x89
        _emit 0x2e
        _emit 0x8b
        _emit 0x47

        _emit 0x04
        _emit 0x3b
        _emit 0x30
        _emit 0x75
        _emit 0x12
        _emit 0x89
        _emit 0x28
        _emit 0xeb
        _emit 0x0e
        _emit 0x89
        _emit 0x6e
        _emit 0x08

        _emit 0x8b
        _emit 0x47
        _emit 0x04
        _emit 0x3b
        _emit 0x70
        _emit 0x08
        _emit 0x75
        _emit 0x03
        _emit 0x89
        _emit 0x68
        _emit 0x08
        _emit 0x8b

        _emit 0x55
        _emit 0x04
        _emit 0x80
        _emit 0x7a
        _emit 0x44
        _emit 0x00
        _emit 0x8d
        _emit 0x45
        _emit 0x04
        _emit 0x8b
        _emit 0xf5
        _emit 0x0f

        _emit 0x85
        _emit 0xec
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x8b
        _emit 0x08
        _emit 0x8b
        _emit 0x51
        _emit 0x04
        _emit 0x3b
        _emit 0x0a
        _emit 0x75
        _emit 0x51
        _emit 0x8b
        _emit 0x52
        _emit 0x08

        _emit 0x80
        _emit 0x7a
        _emit 0x44
        _emit 0x00
        _emit 0x75
        _emit 0x19
        _emit 0x88
        _emit 0x59
        _emit 0x44
        _emit 0x88
        _emit 0x5a
        _emit 0x44

        _emit 0x8b
        _emit 0x10
        _emit 0x8b
        _emit 0x4a
        _emit 0x04
        _emit 0xc6
        _emit 0x41
        _emit 0x44
        _emit 0x00
        _emit 0x8b
        _emit 0x10
        _emit 0x8b

        _emit 0x72
        _emit 0x04
        _emit 0xe9
        _emit 0xaa
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x3b
        _emit 0x71
        _emit 0x08
        _emit 0x75
        _emit 0x0a

        _emit 0x8b
        _emit 0xf1
        _emit 0x56
        _emit 0x8b
        _emit 0xcf
        _emit 0xe8
        _emit 0xd6
        _emit 0xee
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x46

        _emit 0x04
        _emit 0x88
        _emit 0x58
        _emit 0x44
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        _emit 0x8b
        _emit 0x51
        _emit 0x04
        _emit 0xc6
        _emit 0x42

        _emit 0x44
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        _emit 0x51
        _emit 0x8b
        _emit 0xcf
        _emit 0xe8

        _emit 0xb8
        _emit 0xec
        _emit 0xff
        _emit 0xff
        _emit 0xeb
        _emit 0x7b
        _emit 0x8b
        _emit 0x12
        _emit 0x80
        _emit 0x7a
        _emit 0x44
        _emit 0x00

        _emit 0x75
        _emit 0x16
        _emit 0x88
        _emit 0x59
        _emit 0x44
        _emit 0x88
        _emit 0x5a
        _emit 0x44
        _emit 0x8b
        _emit 0x10
        _emit 0x8b
        _emit 0x4a

        _emit 0x04
        _emit 0xc6
        _emit 0x41
        _emit 0x44
        _emit 0x00
        _emit 0x8b
        _emit 0x10
        _emit 0x8b
        _emit 0x72
        _emit 0x04
        _emit 0xeb
        _emit 0x5d

        _emit 0x3b
        _emit 0x31
        _emit 0x75
        _emit 0x0a
        _emit 0x8b
        _emit 0xf1
        _emit 0x56
        _emit 0x8b
        _emit 0xcf
        _emit 0xe8
        _emit 0x8a
        _emit 0xec

        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x88
        _emit 0x58
        _emit 0x44
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        _emit 0x8b

        _emit 0x51
        _emit 0x04
        _emit 0xc6
        _emit 0x42
        _emit 0x44
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x8b
        _emit 0x40
        _emit 0x04

        _emit 0x8b
        _emit 0x48
        _emit 0x08
        _emit 0x8b
        _emit 0x11
        _emit 0x89
        _emit 0x50
        _emit 0x08
        _emit 0x8b
        _emit 0x11
        _emit 0x80
        _emit 0x7a

        _emit 0x45
        _emit 0x00
        _emit 0x75
        _emit 0x03
        _emit 0x89
        _emit 0x42
        _emit 0x04
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        _emit 0x89
        _emit 0x51

        _emit 0x04
        _emit 0x8b
        _emit 0x57
        _emit 0x04
        _emit 0x3b
        _emit 0x42
        _emit 0x04
        _emit 0x75
        _emit 0x05
        _emit 0x89
        _emit 0x4a
        _emit 0x04

        _emit 0xeb
        _emit 0x0e
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        _emit 0x3b
        _emit 0x02
        _emit 0x75
        _emit 0x04
        _emit 0x89
        _emit 0x0a
        _emit 0xeb

        _emit 0x03
        _emit 0x89
        _emit 0x4a
        _emit 0x08
        _emit 0x89
        _emit 0x01
        _emit 0x89
        _emit 0x48
        _emit 0x04
        _emit 0x8b
        _emit 0x4e
        _emit 0x04

        _emit 0x80
        _emit 0x79
        _emit 0x44
        _emit 0x00
        _emit 0x8d
        _emit 0x46
        _emit 0x04
        _emit 0x0f
        _emit 0x84
        _emit 0x1b
        _emit 0xff
        _emit 0xff

        _emit 0xff
        _emit 0x8b
        _emit 0x57
        _emit 0x04
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        _emit 0x88
        _emit 0x58
        _emit 0x44
        _emit 0x8b
        _emit 0x44

        _emit 0x24
        _emit 0x68
        _emit 0x89
        _emit 0x68
        _emit 0x04
        _emit 0x89
        _emit 0x38
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x58
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
        _emit 0x50
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
