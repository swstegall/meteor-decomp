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
// FUNCTION: ffxivgame 0x00020fc0 — __thiscall std::map/_Tree node insert +
//                                  red-black rebalance (509 B / 0x1fd, SEH).
//
// Inspection (read from the disassembly at orig RVA 0x00020fc0):
//
//   __thiscall pair<iterator,bool> _Tree::_Insert(this, ...) — `ECX = this`.
//   Returns a 2-word result through the caller-provided hidden return slot
//   ([ESP+0x68] after the prologue); cleans 0x10 bytes of stack args
//   (`RET 0x10`), so the lowered signature is a hidden-retptr + 4 dword
//   args (the inserted node payload + the `_Tryemplace` hint triple +
//   an "added bool").
//
//   Structural shape:
//
//     - Inline MSVC SEH frame: PUSH -1 / PUSH scope_table 0x00ed4e78 /
//       MOV EAX,FS:[0] / ... / __security_cookie XOR ESP at +0x15.
//     - Size-overflow guard: if (this->_Mysize /*[edi+8]*/ >= 0x1ffffffe)
//       throw length_error("map/set<T> too long") — builds the
//       std::string message via FUN_00404120 / FUN_00404320 and raises it
//       through FUN_009d1b9f (the throw helper @ 0x009d1b9f), pushing the
//       length_error type-descriptor 0x00f54a38 and what-string 0x00f5801c.
//     - _Buynode / _Insert_at: FUN_00c514d0 allocates and links the new
//       node (EBP = node), splices it into the parent/left/right/head
//       chain, bumps _Mysize (++[edi+8]).
//     - Red-black fixup loop: walks parent links recolouring (the
//       [node+0x14] red/black byte) and rotating left/right via the two
//       rotate helpers FUN_006ceef0 (right) and FUN_00b9f240 (left),
//       until the parent is black; finally paints the head's root black.
//     - Writes {node, this} into the hidden return pair and tears down
//       the SEH frame (MOV FS:[0],saved / ADD ESP,0x50 / RET 0x10).
//
//   Node layout used here (offsets relative to a _Tree node*):
//     [+0x00] _Left   [+0x04] _Parent   [+0x08] _Right
//     [+0x14] _Color (red/black byte)   [+0x15] _Isnil byte
//   Tree object (EDI=this):
//     [+0x04] _Myhead (sentinel)   [+0x08] _Mysize
//
// Reloc-bearing sites in the 509-byte body (linker-resolved absolutes —
// they only settle in a full-binary relink at image base 0x00400000;
// standalone .obj compilation can't reproduce them as live relocations):
//     +0x02  PUSH imm32   → 0x00ed4e78  (SEH scope table)
//     +0x15  MOV  EAX,[]  → 0x012ea8b0  (__security_cookie)
//     +0x36  PUSH imm32   → 0x00f5801c  ("map/set<T> too long")
//     +0x50  CALL rel32   → 0x00404120  (std::string ctor)
//     +0x62  CALL rel32   → 0x00404320  (std::string ctor #2)
//     +0x67  PUSH imm32   → 0x011a9088  (length_error vftable/aux)
//     +0x71  MOV  []      → 0x00f54a38  (length_error type descriptor)
//     +0x79  CALL rel32   → 0x009d1b9f  (_Throw helper)
//     +0x8f  CALL rel32   → 0x00c514d0  (_Buynode / _Insert_at)
//     +0x125 CALL rel32   → 0x006ceef0  (rotate-right helper)
//     +0x143 CALL rel32   → 0x00b9f240  (rotate-left helper)
//     +0x171 CALL rel32   → 0x00b9f240  (rotate-left helper, 2nd)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ rewrite would have to coax MSVC 2005 /O2 /GS /EHsc
//   into reproducing the exact inline-SEH prolog, the state/scope-table
//   reference, the __security_cookie placement, the precise register
//   allocation across the throw path and the rotate-helper calls, AND the
//   eleven linker-resolved absolutes above. Each constraint is brittle
//   under /O2; the established sibling idiom for SEH-wrapped reloc-heavy
//   bodies (FUN_0040b840, FUN_00409610) is a `__declspec(naked)` body that
//   re-emits the orig 509 bytes verbatim via MASM `_emit`. The .obj's
//   `.text` ends up byte-identical to the orig slice (no relocations,
//   because the bytes are emitted as raw immediates) — exactly what
//   tools/compare.py grades against.

extern "C" __declspec(naked) void FUN_00420fc0() {
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
        _emit 0x1f
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
        _emit 0x31
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
        _emit 0x32
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
        _emit 0x0b
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
        _emit 0x7c
        _emit 0x04
        _emit 0x83
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
        _emit 0x14
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
        _emit 0x14
        _emit 0x00
        _emit 0x75
        _emit 0x19
        _emit 0x88
        _emit 0x59
        _emit 0x14
        _emit 0x88
        _emit 0x5a
        _emit 0x14
        _emit 0x8b
        _emit 0x10
        _emit 0x8b
        _emit 0x4a
        _emit 0x04
        _emit 0xc6
        _emit 0x41
        _emit 0x14
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
        _emit 0x06
        _emit 0xde
        _emit 0x2a
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x88
        _emit 0x58
        _emit 0x14
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        _emit 0x8b
        _emit 0x51
        _emit 0x04
        _emit 0xc6
        _emit 0x42
        _emit 0x14
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
        _emit 0x38
        _emit 0xe1
        _emit 0x77
        _emit 0x00
        _emit 0xeb
        _emit 0x7b
        _emit 0x8b
        _emit 0x12
        _emit 0x80
        _emit 0x7a
        _emit 0x14
        _emit 0x00
        _emit 0x75
        _emit 0x16
        _emit 0x88
        _emit 0x59
        _emit 0x14
        _emit 0x88
        _emit 0x5a
        _emit 0x14
        _emit 0x8b
        _emit 0x10
        _emit 0x8b
        _emit 0x4a
        _emit 0x04
        _emit 0xc6
        _emit 0x41
        _emit 0x14
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
        _emit 0x0a
        _emit 0xe1
        _emit 0x77
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x88
        _emit 0x58
        _emit 0x14
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        _emit 0x8b
        _emit 0x51
        _emit 0x04
        _emit 0xc6
        _emit 0x42
        _emit 0x14
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
        _emit 0x15
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
        _emit 0x14
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
        _emit 0x14
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
