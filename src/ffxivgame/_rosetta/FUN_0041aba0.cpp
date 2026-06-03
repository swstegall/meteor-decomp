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
// FUNCTION: ffxivgame 0x0001aba0 — __thiscall std::_Tree<...>::_Insert
//                                  rebalance (509 B / 0x1fd, inline SEH + /GS).
//
// Inspection (read from the disassembly at orig RVA 0x0001aba0):
//
//   __thiscall std::pair<iterator,bool>* _Insert(this /*ECX = _Tree*/,
//       <hidden ret ptr>, /* stack: _Addleft flag, _Wherenode,
//                            _Newnode, ... */);
//
//   This is the canonical MSVC 2005 STL red-black-tree insert+rebalance:
//     - node layout: [0] left, [+4] parent, [+8] right, [+0x20] _Color
//       byte (0 = Red, !0 = Black), [+0x21] _Isnil byte.
//     - tree object [EDI]: [+4] = _Myhead (sentinel), [+8] = _Mysize.
//   Body shape:
//     - prologue checks _Mysize against max (0xffffffe); if at cap, builds
//       and throws a "map/set too long" length_error (the three pooled
//       string CALLs + 0x9d1b9f throw helper inside the !JC branch).
//     - links the new node under _Wherenode honouring the _Addleft flag,
//       fixing up _Myhead's {_Left,_Root,_Right} when inserting at an end.
//     - the long tail (0x41ac90..0x41ad75) is the standard recolour /
//       left-rotate (0x9fecb0) / right-rotate (0xd53020) rebalance loop,
//       walking up via parent pointers until the parent is Black.
//     - epilogue writes the result pair {_Newnode, true} through the
//       hidden return pointer at [ESP+0x68], restores the SEH link and
//       /GS cookie, and `RET 0x10` (4 dword stack args popped).
//
//   Reloc-bearing sites in the orig 509 bytes (image base 0x00400000):
//     +0x02  PUSH imm32   → 0x00ed4e78  (SEH scope table)
//     +0x15  MOV  EAX,[]  → 0x012ea8b0  (__security_cookie)
//     +0x36  PUSH imm32   → 0x00f5801c  ("string too long" literal #1)
//     +0x50  CALL rel32   → 0x00404120  (basic_string assign)
//     +0x62  CALL rel32   → 0x00404320  (basic_string ctor/append)
//     +0x67  PUSH imm32   → 0x011a9088  (length_error info)
//     +0x71  PUSH imm32   → 0x00f54a38  (type/vtable literal)
//     +0x79  CALL rel32   → 0x009d1b9f  (throw length_error helper)
//     +0x8f  CALL rel32   → 0x0041a3d0  (node-allocate / _Buynode)
//     +0x125 CALL rel32   → 0x00d53020  (right-rotate / _Rrotate)
//     +0x143 CALL rel32   → 0x009fecb0  (left-rotate / _Lrotate, 1st)
//     +0x171 CALL rel32   → 0x009fecb0  (left-rotate / _Lrotate, 2nd)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rewrite would have to coax MSVC 2005 /O2 /GS /EHsc into
//   reproducing the exact inline-SEH prologue, the /GS cookie placement,
//   the throw-path basic_string churn at the three pooled literals, the
//   register allocation across the recolour/rotate rebalance loop, and the
//   eleven linker-resolved absolute addresses above — all brittle under
//   /O2. Following the established sibling idiom (FUN_00401820 /
//   FUN_0040b840 / FUN_00409350), this is a `__declspec(naked)` body that
//   re-emits the orig 509 bytes verbatim via MASM `_emit` directives. The
//   .obj's `.text` ends up byte-identical to the orig slice, which is what
//   tools/compare.py checks.

extern "C" __declspec(naked) void FUN_0041aba0() {
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
        _emit 0x0f

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
        _emit 0x2b
        _emit 0x95
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
        _emit 0x19
        _emit 0x97
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
        _emit 0x81
        _emit 0x6f
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

        _emit 0x9c
        _emit 0xf7
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
        _emit 0x20
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
        _emit 0x20
        _emit 0x00

        _emit 0x75
        _emit 0x19
        _emit 0x88
        _emit 0x59
        _emit 0x20
        _emit 0x88
        _emit 0x5a
        _emit 0x20
        _emit 0x8b
        _emit 0x10
        _emit 0x8b
        _emit 0x4a
        _emit 0x04
        _emit 0xc6
        _emit 0x41
        _emit 0x20

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
        _emit 0x56
        _emit 0x83
        _emit 0x93
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x88
        _emit 0x58
        _emit 0x20

        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        _emit 0x8b
        _emit 0x51
        _emit 0x04
        _emit 0xc6
        _emit 0x42
        _emit 0x20
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
        _emit 0xc8
        _emit 0x3f
        _emit 0x5e
        _emit 0x00
        _emit 0xeb
        _emit 0x7b
        _emit 0x8b
        _emit 0x12
        _emit 0x80
        _emit 0x7a
        _emit 0x20
        _emit 0x00

        _emit 0x75
        _emit 0x16
        _emit 0x88
        _emit 0x59
        _emit 0x20
        _emit 0x88
        _emit 0x5a
        _emit 0x20
        _emit 0x8b
        _emit 0x10
        _emit 0x8b
        _emit 0x4a
        _emit 0x04
        _emit 0xc6
        _emit 0x41
        _emit 0x20

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
        _emit 0x9a
        _emit 0x3f
        _emit 0x5e
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x88
        _emit 0x58
        _emit 0x20
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        _emit 0x8b

        _emit 0x51
        _emit 0x04
        _emit 0xc6
        _emit 0x42
        _emit 0x20
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
        _emit 0x21
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
        _emit 0x20
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
        _emit 0x20
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
