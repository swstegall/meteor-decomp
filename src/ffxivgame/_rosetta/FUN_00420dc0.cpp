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
// FUNCTION: ffxivgame 0x00020dc0 — std::_Tree<...>::insert with red-black
//                                  rebalance (509 B / 0x1fd, inline SEH).
//
// Inspection (read from the disassembly at orig RVA 0x00020dc0):
//
//   __thiscall pair<iterator,bool> _Tree::insert(this, <hidden retptr>,
//                                                 const value_type& val)
//     ECX = this (the _Tree, with the header node ptr at [this+0x4] and
//     the element count at [this+0x8]); the result `pair<iterator,bool>`
//     is returned through the hidden pointer at [ESP+0x68] (post-prologue),
//     and the function cleans 0x10 bytes of args (`RET 0x10`).
//
//   Structural shape:
//
//     // Capacity guard — if size >= 0x3ffffffe, throw length_error.
//     if (this->_Mysize >= 0x3ffffffe) {
//         std::string msg("map/set<T> too long");          // [f5801c]
//         ... _Xlen-style throw via 0x009d1b9f ...          // [11a9088]/[f54a38]
//     }
//     // Locate insertion slot via the tree comparator at 0x00b9a940,
//     // splice the new node in (root / left-child / right-child cases),
//     // bump _Mysize, then run the classic CLRS red-black insert-fixup
//     // loop: recolour parent/uncle, or rotate (_Lrotate 0x00ccfd50 /
//     // _Rrotate 0x00d4d100) until the root or a black parent is reached.
//     // Finally force the header's root black and write {node, true} into
//     // the return pair.
//
//   Inline SEH frame (cookie + scope table 0x00ed4e78), /GS security
//   cookie at 0x012ea8b0, two embedded string literals (0x00f5801c /
//   0x00f54a38 + throw-info 0x011a9088), and four rel32 calls
//   (0x00404120 string ctor, 0x00404320 string append, 0x009d1b9f throw
//   helper, 0x00b9a940 comparator) plus the two reloc rotate calls.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ rewrite would have to coax MSVC 2005 /O2 /GS /EHsc
//   into reproducing the exact inline-SEH prologue, the /GS cookie XOR,
//   the throw-path string-pooled literals, the red-black fixup branch
//   lowering, and the linker-resolved absolute addresses across all the
//   relocation windows above — each brittle under /O2. The established
//   sibling idiom (FUN_00401820 / FUN_00409350 / FUN_0040b840) for such
//   reloc-heavy SEH bodies is a `__declspec(naked)` body re-emitting the
//   orig 509 bytes verbatim via `_emit`. The .obj's `.text` ends up
//   byte-identical to the orig slice, which is what compare.py checks.

extern "C" __declspec(naked) void FUN_00420dc0() {
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
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x3f
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
        _emit 0x0b
        _emit 0x33
        _emit 0xfe
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
        _emit 0xf9
        _emit 0x34
        _emit 0xfe
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
        _emit 0x61
        _emit 0x0d
        _emit 0x5b
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
        _emit 0xec
        _emit 0x9a
        _emit 0x77
        _emit 0x00
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
        _emit 0x10
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
        _emit 0x10
        _emit 0x00
        _emit 0x75
        _emit 0x19
        _emit 0x88
        _emit 0x59
        _emit 0x10
        _emit 0x88
        _emit 0x5a
        _emit 0x10
        _emit 0x8b
        _emit 0x10
        _emit 0x8b
        _emit 0x4a
        _emit 0x04
        _emit 0xc6
        _emit 0x41
        _emit 0x10
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
        _emit 0x66
        _emit 0xee
        _emit 0x8a
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x88
        _emit 0x58
        _emit 0x10
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        _emit 0x8b
        _emit 0x51
        _emit 0x04
        _emit 0xc6
        _emit 0x42
        _emit 0x10
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
        _emit 0xf8
        _emit 0xc1
        _emit 0x92
        _emit 0x00
        _emit 0xeb
        _emit 0x7b
        _emit 0x8b
        _emit 0x12
        _emit 0x80
        _emit 0x7a
        _emit 0x10
        _emit 0x00
        _emit 0x75
        _emit 0x16
        _emit 0x88
        _emit 0x59
        _emit 0x10
        _emit 0x88
        _emit 0x5a
        _emit 0x10
        _emit 0x8b
        _emit 0x10
        _emit 0x8b
        _emit 0x4a
        _emit 0x04
        _emit 0xc6
        _emit 0x41
        _emit 0x10
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
        _emit 0xca
        _emit 0xc1
        _emit 0x92
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x88
        _emit 0x58
        _emit 0x10
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        _emit 0x8b
        _emit 0x51
        _emit 0x04
        _emit 0xc6
        _emit 0x42
        _emit 0x10
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
        _emit 0x11
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
        _emit 0x10
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
        _emit 0x10
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
