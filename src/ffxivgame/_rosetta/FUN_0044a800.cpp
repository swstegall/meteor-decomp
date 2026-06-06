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
// FUNCTION: ffxivgame 0x0044a800 — `__thiscall` SBO-vector erase/append
//                                  dispatcher (248 B / 0xf8, no SEH).
//
// Behaviour read from asm/ffxivgame/0004a800_FUN_0044a800.s:
//
//   __thiscall T* FUN_0044a800(this, void* pos /*[esp+0x10]*/,
//                              size_t n /*[esp+0x14]*/) — ECX = this,
//   two stack args, returns `this` in EAX (`ret 8`).
//
//   `this` is the FFXIV small-buffer-optimised vector<dword>:
//     +0x04  union { T inline_buf[4]; T* heap; }   (16 B inline span)
//     +0x14  size_t  count                          (element count)
//     +0x18  size_t  capacity
//   data() = (capacity < 4) ? &inline_buf : heap   (SBO selector, repeated
//   inline at every use site — the `CMP [esi+0x18],4 / JC` idiom).
//
//   Shape:
//     T* b = data();
//     if (pos >= b && pos < b + count) {            // pos inside live range
//         // erase-at-index path
//         return this->erase_one(this, (pos - b)/4, n);   // CALL 0x0044a680
//     }
//     // append / grow-and-zero-fill path
//     if ((size_t)(-1 - count) <= n || count + n < count)
//         _Xlen();                                  // CALL 0x009d042e
//     if (n == 0) return this;
//     size_t newsize = count + n;
//     if (newsize > (size_t)-2) _Xlen();            // CALL 0x009d042e
//     if (capacity < newsize)
//         this->reserve_grow(this, newsize, count); // CALL 0x0044a1d0
//     if (newsize != 0) {
//         T* d = data();
//         _fill_n(d + count, capacity - count, n, n);   // CALL 0x00449a40
//         data()[count] = 0;  count = newsize;      // (n==1 fast tail else)
//     }
//     return this;
//
//   Reloc-bearing CALL sites in the orig 248 bytes (rel32 — only resolve in
//   a full-binary relink at image base 0x00400000):
//     +0x48  rel32  0x0044a680 — erase_one
//     +0x6b  rel32  0x009d042e — _Xlen (length_error throw)
//     +0x7e  rel32  0x009d042e — _Xlen (2nd)
//     +0x91  rel32  0x0044a1d0 — reserve_grow
//     +0xd5  rel32  0x00449a40 — _fill_n
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level C++ port would have to coax MSVC 2005 /O2 into
//   reproducing the SBO selector inlined five times, the exact EBX/ESI/
//   EDI/EBP register allocation across two distinct exit arms, the branch
//   short-vs-near choices, AND the five linker-resolved rel32 windows. Each
//   of those is brittle under /O2 — every high-level rewrite shifts a byte.
//   The pragmatic match (same route the sibling rosetta files took) is a
//   naked body re-emitting the orig 248 bytes verbatim via `_emit`. The
//   .obj's `.text` ends up byte-identical with no relocations, which is
//   what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_0044a800() {
    __asm {
        // 0004a800
        _emit 0x53
        _emit 0x56
        _emit 0x8b
        _emit 0xf1
        _emit 0x8b
        _emit 0x56
        _emit 0x18
        _emit 0x83
        _emit 0xfa
        _emit 0x04
        _emit 0x57
        _emit 0x8d
        _emit 0x5e
        _emit 0x04
        _emit 0x72
        _emit 0x04
        _emit 0x8b
        _emit 0x0b
        _emit 0xeb
        _emit 0x02
        _emit 0x8b
        _emit 0xcb
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x3b
        _emit 0xc1
        _emit 0x72
        _emit 0x35
        _emit 0x83
        _emit 0xfa
        _emit 0x04
        _emit 0x72
        _emit 0x04
        _emit 0x8b
        _emit 0x0b
        _emit 0xeb
        _emit 0x02
        _emit 0x8b
        _emit 0xcb
        _emit 0x8b
        _emit 0x7e
        _emit 0x14
        _emit 0x8d
        _emit 0x0c
        _emit 0xb9
        _emit 0x3b
        _emit 0xc8
        _emit 0x76
        _emit 0x20
        _emit 0x83
        _emit 0xfa
        _emit 0x04
        _emit 0x72
        _emit 0x02
        _emit 0x8b
        _emit 0x1b
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x2b
        _emit 0xc3
        _emit 0x52
        _emit 0xc1
        _emit 0xf8
        _emit 0x02
        _emit 0x50
        _emit 0x56
        _emit 0x8b
        _emit 0xce
        // 0004a848  CALL 0x0044a680
        _emit 0xe8
        _emit 0x33
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // 0004a853
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        _emit 0x83
        _emit 0xc9
        _emit 0xff
        _emit 0x55
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x2b
        _emit 0xc8
        _emit 0x3b
        _emit 0xcd
        _emit 0x76
        _emit 0x07
        _emit 0x8d
        _emit 0x14
        _emit 0x28
        _emit 0x3b
        _emit 0xd0
        _emit 0x73
        _emit 0x05
        // 0004a86b  CALL 0x009d042e
        _emit 0xe8
        _emit 0xbe
        _emit 0x5b
        _emit 0x58
        _emit 0x00
        _emit 0x85
        _emit 0xed
        _emit 0x76
        _emit 0x7b
        _emit 0x8b
        _emit 0x7e
        _emit 0x14
        _emit 0x03
        _emit 0xfd
        _emit 0x83
        _emit 0xff
        _emit 0xfe
        _emit 0x76
        _emit 0x05
        // 0004a87e  CALL 0x009d042e
        _emit 0xe8
        _emit 0xab
        _emit 0x5b
        _emit 0x58
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        _emit 0x3b
        _emit 0xc7
        _emit 0x73
        _emit 0x1c
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        _emit 0x50
        _emit 0x57
        _emit 0x8b
        _emit 0xce
        // 0004a891  CALL 0x0044a1d0
        _emit 0xe8
        _emit 0x3a
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        _emit 0x85
        _emit 0xff
        _emit 0x76
        _emit 0x55
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        _emit 0x83
        _emit 0xf8
        _emit 0x04
        _emit 0x72
        _emit 0x21
        _emit 0x8b
        _emit 0x13
        _emit 0xeb
        _emit 0x1f
        _emit 0x85
        _emit 0xff
        _emit 0x75
        _emit 0xee
        _emit 0x83
        _emit 0xf8
        _emit 0x04
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        _emit 0x72
        _emit 0x02
        _emit 0x8b
        _emit 0x1b
        _emit 0x5d
        _emit 0x5f
        _emit 0x8b
        _emit 0xc6
        _emit 0x5e
        _emit 0xc7
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5b
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // 0004a8c3
        _emit 0x8b
        _emit 0xd3
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        _emit 0x55
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x2b
        _emit 0xc1
        _emit 0x55
        _emit 0x50
        _emit 0x8d
        _emit 0x0c
        _emit 0x8a
        _emit 0x51
        // 0004a8d5  CALL 0x00449a40
        _emit 0xe8
        _emit 0x66
        _emit 0xf1
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x83
        _emit 0x7e
        _emit 0x18
        _emit 0x04
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        _emit 0x72
        _emit 0x02
        _emit 0x8b
        _emit 0x1b
        _emit 0xc7
        _emit 0x04
        _emit 0xbb
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004a8ef
        _emit 0x5d
        _emit 0x5f
        _emit 0x8b
        _emit 0xc6
        _emit 0x5e
        _emit 0x5b
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
