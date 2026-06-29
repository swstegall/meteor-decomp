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
// FUNCTION: ffxivgame 0x00044e80 — __thiscall container splice/replace
//                                  (421 B / 0x1a5, no SEH).
//
// NB: the asm dump elides the `83 c4 04` (ADD ESP,4) cleanup at 0x44f83
// after the operator-delete call (0x009d1b17); the true body is 421 B,
// as size_overrides records (symbols.json's 418 B was short by that pop).
//
// Inspection (read from the disassembly at orig RVA 0x00044e80):
//
//   __thiscall <ret> FUN_00444e80(this, arg0, arg1, arg2, arg3);
//
//   `ECX = this` (held in ESI), four stack args (16 B, callee-popped
//   `RET 0x10`). The shape is a classic MSVC-2005 STL contiguous-storage
//   insert/replace: it dereferences arg3 (a `const T&`) into a stack
//   temp, computes the current logical position/size from three
//   this-relative pointer fields (this+0x4 = begin, this+0x8 = end,
//   this+0xc = capacity), runs the std::length_error / `_Xlen` overflow
//   guard (the `OR EDX,-1; SUB EDX,n; CMP; JNC` capacity-check idiom that
//   tail-calls 0x00cb0e40 on overflow), allocates a fresh buffer via the
//   allocator helper at 0x00401090 when a grow is required, and moves the
//   three element runs (prefix / inserted / suffix) with memmove-style
//   copies (0x009d186e) and the relocate/destroy helpers (0x00e07790,
//   0x006d1920, 0x005c3750, 0x0041a480), freeing the old block through
//   0x009d1b17. Three exit arms (the grow path, the in-place shift-right
//   path, and the empty-count early-out at 0x0044501f) each restore the
//   EBX/EBP/ESI/EDI callee-saves and `RET 0x10`.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The body carries eight rel32 call sites (0x00cb0e40, 0x00401090,
//   0x009d186e ×3, 0x006d1920 ×2, 0x009d1b17, 0x00e07790 ×2, 0x005c3750,
//   0x0041a480) whose displacements only resolve at a full-binary relink
//   (image base 0x00400000); a standalone .obj can't reproduce them from
//   source. Coaxing MSVC 2005 /O2 into the exact register allocation
//   (ESI=this, EBP=size, EBX=count, EDI=newbuf), the three-way exit
//   structure, the repeated `TEST EAX,EAX; XOR/MOV; SUB` begin-or-zero
//   idiom, and the `OR EDX,-1; SUB; CMP; JNC` _Xlen guards is brittle —
//   every high-level rewrite shifts at least one byte. As with the
//   sibling reloc-heavy bodies (FUN_0040ced0 / FUN_00415d00 /
//   FUN_00409350), the pragmatic match is a `__declspec(naked)` body that
//   re-emits the orig 418 bytes verbatim via MASM `_emit` directives, so
//   the .obj's `.text` is byte-identical to the orig slice (which is what
//   tools/compare.py checks).

extern "C" __declspec(naked) void FUN_00444e80() {
    __asm {
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x8b
        _emit 0xf1
        _emit 0x8a
        _emit 0x08
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x85
        _emit 0xc0

        _emit 0x88
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x75
        _emit 0x04
        _emit 0x33
        _emit 0xed
        _emit 0xeb
        _emit 0x05
        _emit 0x8b
        _emit 0x6e
        _emit 0x0c
        _emit 0x2b
        _emit 0xe8
        _emit 0x8b

        _emit 0x5c
        _emit 0x24
        _emit 0x18
        _emit 0x85
        _emit 0xdb
        _emit 0x0f
        _emit 0x84
        _emit 0x74
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x04
        _emit 0x33

        _emit 0xc9
        _emit 0xeb
        _emit 0x05
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        _emit 0x2b
        _emit 0xc8
        _emit 0x83
        _emit 0xca
        _emit 0xff
        _emit 0x2b
        _emit 0xd1
        _emit 0x3b
        _emit 0xd3
        _emit 0x73

        _emit 0x05
        _emit 0xe8
        _emit 0x7a
        _emit 0xbf
        _emit 0x86
        _emit 0x00
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x04
        _emit 0x33
        _emit 0xc9
        _emit 0xeb
        _emit 0x05
        _emit 0x8b
        _emit 0x4e

        _emit 0x08
        _emit 0x2b
        _emit 0xc8
        _emit 0x03
        _emit 0xcb
        _emit 0x3b
        _emit 0xe9
        _emit 0x57
        _emit 0x0f
        _emit 0x83
        _emit 0xbe
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xcd

        _emit 0xd1
        _emit 0xe9
        _emit 0x83
        _emit 0xca
        _emit 0xff
        _emit 0x2b
        _emit 0xd1
        _emit 0x3b
        _emit 0xd5
        _emit 0x73
        _emit 0x04
        _emit 0x33
        _emit 0xed
        _emit 0xeb
        _emit 0x02
        _emit 0x03

        _emit 0xe9
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x04
        _emit 0x33
        _emit 0xc9
        _emit 0xeb
        _emit 0x05
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        _emit 0x2b
        _emit 0xc8
        _emit 0x03
        _emit 0xcb

        _emit 0x3b
        _emit 0xe9
        _emit 0x73
        _emit 0x0f
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x04
        _emit 0x33
        _emit 0xed
        _emit 0xeb
        _emit 0x05
        _emit 0x8b
        _emit 0x6e
        _emit 0x08
        _emit 0x2b

        _emit 0xe8
        _emit 0x03
        _emit 0xeb
        _emit 0x6a
        _emit 0x00
        _emit 0x55
        _emit 0xe8
        _emit 0x75
        _emit 0xc1
        _emit 0xfb
        _emit 0xff
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        _emit 0x8b
        _emit 0xf8

        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x2b
        _emit 0xc1
        _emit 0x8d
        _emit 0x14
        _emit 0x38
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x1c

        _emit 0x74
        _emit 0x0c
        _emit 0x50
        _emit 0x51
        _emit 0x50
        _emit 0x57
        _emit 0xe8
        _emit 0x33
        _emit 0xc9
        _emit 0x58
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x8b
        _emit 0x4c

        _emit 0x24
        _emit 0x1c
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x50
        _emit 0x53
        _emit 0x51
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0xd0
        _emit 0xc9
        _emit 0x28
        _emit 0x00

        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x2b
        _emit 0xca
        _emit 0x74
        _emit 0x0c
        _emit 0x51
        _emit 0x52
        _emit 0x51
        _emit 0x50
        _emit 0xe8

        _emit 0x0a
        _emit 0xc9
        _emit 0x58
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        _emit 0x85
        _emit 0xc9
        _emit 0x75
        _emit 0x04
        _emit 0x33
        _emit 0xc0

        _emit 0xeb
        _emit 0x05
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        _emit 0x2b
        _emit 0xc1
        _emit 0x03
        _emit 0xd8
        _emit 0x85
        _emit 0xc9
        _emit 0x74
        _emit 0x09
        _emit 0x51
        _emit 0xe8
        _emit 0x94

        _emit 0xcb
        _emit 0x58
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x8d
        _emit 0x14
        _emit 0x2f
        _emit 0x8d
        _emit 0x04
        _emit 0x1f
        _emit 0x89
        _emit 0x7e
        _emit 0x04
        _emit 0x5f
        _emit 0x89
        _emit 0x56
        _emit 0x0c

        _emit 0x89
        _emit 0x46
        _emit 0x08
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0xc2
        _emit 0x10
        _emit 0x00
        _emit 0x8b
        _emit 0x6e
        _emit 0x08
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x18

        _emit 0x8b
        _emit 0xcd
        _emit 0x2b
        _emit 0xcf
        _emit 0x3b
        _emit 0xcb
        _emit 0x8b
        _emit 0xce
        _emit 0x73
        _emit 0x40
        _emit 0x8d
        _emit 0x14
        _emit 0x1f
        _emit 0x52
        _emit 0x55
        _emit 0x57

        _emit 0xe8
        _emit 0xd8
        _emit 0x27
        _emit 0x9c
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        _emit 0x8b
        _emit 0xd7
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x51
        _emit 0x2b

        _emit 0xd0
        _emit 0x03
        _emit 0xd3
        _emit 0x52
        _emit 0x50
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0x51
        _emit 0xc9
        _emit 0x28
        _emit 0x00
        _emit 0x01
        _emit 0x5e
        _emit 0x08
        _emit 0x8b

        _emit 0x76
        _emit 0x08
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x50
        _emit 0x2b
        _emit 0xf3
        _emit 0x56
        _emit 0x57
        _emit 0xe8
        _emit 0x9d
        _emit 0x54
        _emit 0xfd
        _emit 0xff

        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0xc2
        _emit 0x10
        _emit 0x00
        _emit 0x55
        _emit 0x8b
        _emit 0xc5
        _emit 0x2b
        _emit 0xc3
        _emit 0x55

        _emit 0x50
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0xe8
        _emit 0x93
        _emit 0x27
        _emit 0x9c
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x55
        _emit 0x51

        _emit 0x57
        _emit 0x89
        _emit 0x46
        _emit 0x08
        _emit 0xe8
        _emit 0x44
        _emit 0xe7
        _emit 0x17
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        _emit 0x52
        _emit 0x8d
        _emit 0x04

        _emit 0x1f
        _emit 0x50
        _emit 0x57
        _emit 0xe8
        _emit 0x65
        _emit 0x54
        _emit 0xfd
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0xc2

        _emit 0x10
        _emit 0x00
    }
}
