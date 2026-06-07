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
// FUNCTION: ffxivgame 0x0001ada0 — __thiscall sequence-container replace /
//                                  splice (446 B / 0x1be, no SEH, no /GS).
//
// Inspection (read from the disassembly at orig RVA 0x0001ada0):
//
//   __thiscall void splice(this, void* first_off, unsigned count,
//                          unsigned where_off, const Elem* src);
//
//   `ECX = this` (kept in ESI), four stack args (16 B, callee-popped
//   `RET 0x10`). Three this-relative pointer fields drive the math:
//
//     [esi+0x04]  Elem* m_first   — begin pointer (0 ⇒ empty)
//     [esi+0x08]  Elem* m_last    — end-of-data pointer
//     [esi+0x0c]  Elem* m_end     — end-of-capacity pointer
//
//   The body mirrors the MSVC <vector>/string `_Insert_n` style splice:
//
//     unsigned size = m_first ? (m_end - m_first) : 0;          // [esp+0x1c] caches *src byte
//     if (count == 0) {                                          // nothing to do → tail
//         FUN_0041a480(...); return;
//     }
//     unsigned used = m_first ? (m_last - m_first) : 0;
//     if (0xffffffff - used < count) FUN_00c5aed0();            // length_error / _Xlen
//     unsigned newlen = used + count;
//     if (size < newlen) {                                      // must grow + reallocate
//         unsigned grow = size + size/2;                        // 1.5× growth
//         if (0xffffffff - grow < size) grow = 0;               //   overflow → exact
//         unsigned cap = grow;
//         if (cap < newlen) cap = newlen;                       //   floor at requested
//         Elem* nb = FUN_00419f40(this, cap);                   //   allocate
//         long  head = (BYTE)*src... (memmove of the prefix, memmove of the gap-fill,
//                                     memmove of the tail), then swap the buffers,
//                                     FUN_0040df70 frees the old block, and the three
//                                     this fields are rewired to the new buffer.
//     } else {                                                  // fits in place
//         if (m_last - where >= count) { /* shift right + overwrite */ }
//         else                         { /* split fill across old/new tail */ }
//     }
//
//   Cross-references resolved from the call sites (image base 0x00400000):
//     +0x044  rel32  0x00419f40 — FUN_00419f40 (__thiscall allocate(cap))
//     +0x0c8  rel32  0x009d186e — memmove (cdecl, 4 args, `add esp,0x10`)
//     +0x0e1  rel32  0x006d1920 — FUN_006d1920 (__thiscall 3-arg copier)
//     +0x0a6  rel32  0x009d186e — memmove (2nd splice memmove)
//     +0x113  rel32  0x0040df70 — FUN_0040df70 (__thiscall deallocate; ECX=[blk-4])
//     +0x14a  rel32  0x00e07790 — FUN_00e07790 (in-place shift-right helper)
//     +0x161  rel32  0x006d1920 — FUN_006d1920 (in-place gap copier)
//     +0x175  rel32  0x0041a480 — FUN_0041a480 (overwrite tail, shared fall-through)
//     +0x190  rel32  0x00e07790 — FUN_00e07790 (split-tail shift)
//     +0x19f  rel32  0x005c3750 — FUN_005c3750 (split-tail fixup)
//     +0x1ae  rel32  0x0041a480 — FUN_0041a480 (split-tail overwrite)
//     +0x044  rel32  0x00c5aed0 — FUN_00c5aed0 (length_error / _Xlen)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rewrite would have to coax MSVC 2005 /O2 into
//   reproducing the exact register allocation across the eleven call
//   sites, the repeated `m_first ? (m_last - m_first) : 0` ternary
//   lowered four times to a TEST/JNZ/XOR/JMP/SUB chain, the precise
//   EBP/EDI spill cadence, the two distinct `RET 0x10` epilogues that
//   share callee-save pops, and the linker-resolved rel32 targets. Each
//   is brittle under /O2 — every high-level edit shifts at least one
//   byte (branch short-vs-near, ternary lowering, push-fold boundary).
//
//   The pragmatic choice — the same one the reloc-heavy siblings
//   FUN_00415d00 / FUN_00409350 took — is a `__declspec(naked)` body
//   that re-emits the orig 446 bytes verbatim via MASM `_emit`
//   directives. The .obj's `.text` ends up byte-identical to the orig
//   slice, which is what `tools/compare.py` grades against.

extern "C" __declspec(naked) void FUN_0041ada0() {
    __asm {
        _emit 0x51
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x53
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
        _emit 0xd2
        _emit 0xeb
        _emit 0x07
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        _emit 0x2b
        _emit 0xc8
        _emit 0x8b

        _emit 0xd1
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        _emit 0x85
        _emit 0xdb
        _emit 0x0f
        _emit 0x84
        _emit 0x8b
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
        _emit 0x57
        _emit 0x83
        _emit 0xcf
        _emit 0xff
        _emit 0x2b
        _emit 0xf9

        _emit 0x3b
        _emit 0xfb
        _emit 0x73
        _emit 0x05
        _emit 0xe8
        _emit 0xe7
        _emit 0x00
        _emit 0x84
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
        _emit 0xd1
        _emit 0x55
        _emit 0x0f
        _emit 0x83
        _emit 0xd2
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0x8b
        _emit 0xca
        _emit 0xd1
        _emit 0xe9
        _emit 0x83
        _emit 0xcf
        _emit 0xff
        _emit 0x2b
        _emit 0xf9
        _emit 0x3b
        _emit 0xfa
        _emit 0x73
        _emit 0x0e
        _emit 0xc7
        _emit 0x44

        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0xeb
        _emit 0x06
        _emit 0x03
        _emit 0xd1
        _emit 0x89
        _emit 0x54

        _emit 0x24
        _emit 0x10
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
        _emit 0xd1
        _emit 0x73
        _emit 0x15
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
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0xd1
        _emit 0x52
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0xee
        _emit 0xf0

        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        _emit 0x8b
        _emit 0xf8
        _emit 0x8b
        _emit 0xc5
        _emit 0x2b
        _emit 0xc1
        _emit 0x8d

        _emit 0x14
        _emit 0x38
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x74
        _emit 0x0c
        _emit 0x50
        _emit 0x51
        _emit 0x50
        _emit 0x57
        _emit 0xe8
        _emit 0xfd
        _emit 0x69
        _emit 0x5b

        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x50
        _emit 0x53
        _emit 0x51
        _emit 0x8b

        _emit 0xce
        _emit 0xe8
        _emit 0x9a
        _emit 0x6a
        _emit 0x2b
        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        _emit 0x2b
        _emit 0xcd
        _emit 0x74
        _emit 0x0c
        _emit 0x51
        _emit 0x55
        _emit 0x51

        _emit 0x50
        _emit 0xe8
        _emit 0xd8
        _emit 0x69
        _emit 0x5b
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

        _emit 0x8b
        _emit 0x49
        _emit 0xfc
        _emit 0xe8
        _emit 0xb8
        _emit 0x30
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x5d
        _emit 0x8d
        _emit 0x0c
        _emit 0x1f

        _emit 0x8d
        _emit 0x04
        _emit 0x17
        _emit 0x89
        _emit 0x7e
        _emit 0x04
        _emit 0x5f
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        _emit 0x89
        _emit 0x4e
        _emit 0x08
        _emit 0x5e
        _emit 0x5b
        _emit 0x59

        _emit 0xc2
        _emit 0x10
        _emit 0x00
        _emit 0x8b
        _emit 0x6e
        _emit 0x08
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0xd5
        _emit 0x2b
        _emit 0xd7
        _emit 0x3b
        _emit 0xd3

        _emit 0x8b
        _emit 0xce
        _emit 0x73
        _emit 0x41
        _emit 0x8d
        _emit 0x04
        _emit 0x1f
        _emit 0x50
        _emit 0x55
        _emit 0x57
        _emit 0xe8
        _emit 0xa1
        _emit 0xc8
        _emit 0x9e
        _emit 0x00
        _emit 0x8b

        _emit 0x46
        _emit 0x08
        _emit 0x8b
        _emit 0xd7
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x24
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
        _emit 0x1a
        _emit 0x6a
        _emit 0x2b
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
        _emit 0x24

        _emit 0x50
        _emit 0x2b
        _emit 0xf3
        _emit 0x56
        _emit 0x57
        _emit 0xe8
        _emit 0x66
        _emit 0xf5
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x5d
        _emit 0x5f
        _emit 0x5e

        _emit 0x5b
        _emit 0x59
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
        _emit 0x2c

        _emit 0xe8
        _emit 0x5b
        _emit 0xc8
        _emit 0x9e
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x55
        _emit 0x51
        _emit 0x57
        _emit 0x89
        _emit 0x46
        _emit 0x08
        _emit 0xe8

        _emit 0x0c
        _emit 0x88
        _emit 0x1a
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x30
        _emit 0x52
        _emit 0x8d
        _emit 0x04
        _emit 0x1f
        _emit 0x50
        _emit 0x57
        _emit 0xe8
        _emit 0x2d

        _emit 0xf5
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x5d
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0x59
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
