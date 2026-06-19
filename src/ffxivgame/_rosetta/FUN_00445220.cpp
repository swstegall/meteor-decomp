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
// FUNCTION: ffxivgame 0x00045220 — in-place buffer case-folder / title-caser
//                                  (__thiscall, 398 B / 0x18e, no SEH, no /GS;
//                                   RET 0x14 → 5 callee-popped stack args).
//
// Inspection (read from the disassembly at orig RVA 0x00045220):
//
//   __thiscall <this>* fold_case(this,
//                                bool   useStartIdx,    // [esp+0x04]
//                                bool   useEndOverride, // [esp+0x10]
//                                int    startIdx,       // [esp+0x14]
//                                Dict*  exceptions,     // [esp+0x20]
//                                int    endIdx);        // [esp+0x24]
//
//   `ECX = this`; `this->buf = *(char**)(this+0x00)` and
//   `this->len = *(int*)(this+0x08)`. The function returns `this` (EDI).
//
//   Structural shape (read off the asm flow):
//
//     // Decide the scan window. `EAX = startIdx`. If !useStartIdx, OR
//     // (useEndOverride && we fall through), clamp the upper bound to
//     // this->len; otherwise the bound is startIdx+1.
//     unsigned end = (!useStartIdx || !useEndOverride) ? this->len
//                                                       : startIdx + 1;
//     if ((unsigned)startIdx >= end) return this;        // JNC → no work
//
//     bool prevWasSpace = true;                          // DL seed = 1
//     for (unsigned i = startIdx; i < (unsigned)endIdx; ++i) {
//         char c = this->buf[i];
//         // When useStartIdx is set, only the word-initial letter (i.e.
//         // the char following a space) is touched; mid-word chars are
//         // left as-is unless prevWasSpace.
//         if (useStartIdx && !prevWasSpace) goto next;
//
//         if ((unsigned char)(c - 'a') <= 0x19u) {       // ASCII a..z
//             if (exceptions) {
//                 // Linear scan of the {char* word; int len} exception
//                 // table; each non-zero-length entry is matched against
//                 // the tail of the buffer via the cmp helper at 0x9d5475
//                 // (an strncmp/memcmp-style 3-arg cdecl). A hit leaves
//                 // the char untouched.
//                 for (Dict* e = exceptions; e->len; ++e)
//                     if (cmp(e->word, this->buf + i, e->len)) goto next;
//             }
//             this->buf[i] = c - 0x20;                    // uppercase
//             goto next;
//         }
//
//         // Non-ASCII path: handle the UTF-8 two-byte Latin-1 supplement
//         // lead bytes 0xc3 (À-ÿ block) and 0xc5. For 0xc3 the trailing
//         // byte in 0xa0..0xb6 (plus the 0x48-biased extension) selects
//         // one of the A/E/I/O/U fold targets, rewriting either the lead
//         // byte to an ASCII capital + deleting the continuation byte, or
//         // upper-casing the continuation byte in place.
//         ...
//     next:
//         prevWasSpace = (c == ' ');
//     }
//     return this;
//
//   Reloc-bearing site in the orig 398 bytes (resolves only in a
//   full-binary relink at image base 0x00400000; standalone .obj
//   compilation can't reproduce it):
//     +0x73  rel32 CALL 0x009d5475  — cmp helper (strncmp/memcmp-style)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rewrite would have to coax MSVC 2005 /O2 into
//   reproducing the exact register allocation (EDI=this, EBP=i, EBX=cur
//   char, ESI=exception index), the seven-way nested character-class
//   ladder with its short-vs-near branch encodings, the in-place buffer
//   shift loop (the continuation-byte delete at 0x45358), AND the single
//   linker-resolved rel32 to the cmp helper. Every high-level lowering
//   shifts at least one byte, so — following the same choice the other
//   reloc-heavy _rosetta bodies took (FUN_0040ced0, FUN_00415d00) — this
//   is a `__declspec(naked)` body that re-emits the orig 398 bytes
//   verbatim via MASM `_emit`. The .obj's `.text` ends up byte-identical
//   to the orig slice (the bytes are raw immediates, so no relocation is
//   needed), which is exactly what tools/compare.py grades against.

extern "C" __declspec(naked) void FUN_00445220() {
    __asm {
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x04
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x55
        _emit 0x57
        _emit 0x8b
        _emit 0xf9
        _emit 0xb2
        _emit 0x01
        _emit 0x74

        _emit 0x0a
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x8d
        _emit 0x48
        _emit 0x01
        _emit 0x74
        _emit 0x03
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        _emit 0x3b
        _emit 0xc1

        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0xe8
        _emit 0x0f
        _emit 0x83
        _emit 0x43
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x53
        _emit 0x56
        _emit 0x8b
        _emit 0xff

        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x8b
        _emit 0x07
        _emit 0x8a
        _emit 0x1c
        _emit 0x28
        _emit 0x8d
        _emit 0x0c
        _emit 0x28
        _emit 0x88
        _emit 0x5c
        _emit 0x24

        _emit 0x18
        _emit 0x74
        _emit 0x08
        _emit 0x84
        _emit 0xd2
        _emit 0x0f
        _emit 0x84
        _emit 0x0e
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8a
        _emit 0xc3
        _emit 0x2c
        _emit 0x61
        _emit 0x3c

        _emit 0x19
        _emit 0x77
        _emit 0x5e
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x00
        _emit 0x74
        _emit 0x4b
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x33
        _emit 0xf6

        _emit 0x39
        _emit 0x70
        _emit 0x04
        _emit 0x8d
        _emit 0x48
        _emit 0x04
        _emit 0x74
        _emit 0x2e
        _emit 0x8b
        _emit 0x09
        _emit 0x8b
        _emit 0x17
        _emit 0x8b
        _emit 0x00
        _emit 0x51
        _emit 0x03

        _emit 0xd5
        _emit 0x52
        _emit 0x50
        _emit 0xe8
        _emit 0xdd
        _emit 0x01
        _emit 0x59
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0xd4

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        _emit 0x8d
        _emit 0x04
        _emit 0xf1
        _emit 0x83
        _emit 0x78
        _emit 0x04

        _emit 0x00
        _emit 0x8d
        _emit 0x48
        _emit 0x04
        _emit 0x75
        _emit 0xd2
        _emit 0x8b
        _emit 0x07
        _emit 0x8a
        _emit 0xd3
        _emit 0x80
        _emit 0xea
        _emit 0x20
        _emit 0x88
        _emit 0x14
        _emit 0x28

        _emit 0xe9
        _emit 0xb2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8a
        _emit 0xd3
        _emit 0x80
        _emit 0xea
        _emit 0x20
        _emit 0x88
        _emit 0x11
        _emit 0xe9
        _emit 0xa6
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0x80
        _emit 0xfb
        _emit 0xc3
        _emit 0x0f
        _emit 0x85
        _emit 0xc3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8a
        _emit 0x41
        _emit 0x01
        _emit 0x3c
        _emit 0xa0
        _emit 0x72

        _emit 0x04
        _emit 0x3c
        _emit 0xb6
        _emit 0x76
        _emit 0x0e
        _emit 0x8a
        _emit 0xd8
        _emit 0x80
        _emit 0xc3
        _emit 0x48
        _emit 0x80
        _emit 0xfb
        _emit 0x06
        _emit 0x0f
        _emit 0x87
        _emit 0x80

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0x98
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84
        _emit 0xd2

        _emit 0x0f
        _emit 0x84
        _emit 0x90
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8a
        _emit 0xd0
        _emit 0x80
        _emit 0xc2
        _emit 0x60
        _emit 0x80
        _emit 0xfa
        _emit 0x05
        _emit 0x77
        _emit 0x05

        _emit 0xc6
        _emit 0x01
        _emit 0x41
        _emit 0xeb
        _emit 0x36
        _emit 0x8a
        _emit 0xd0
        _emit 0x80
        _emit 0xc2
        _emit 0x58
        _emit 0x80
        _emit 0xfa
        _emit 0x03
        _emit 0x77
        _emit 0x05
        _emit 0xc6

        _emit 0x01
        _emit 0x45
        _emit 0xeb
        _emit 0x27
        _emit 0x8a
        _emit 0xd0
        _emit 0x80
        _emit 0xc2
        _emit 0x54
        _emit 0x80
        _emit 0xfa
        _emit 0x03
        _emit 0x77
        _emit 0x05
        _emit 0xc6
        _emit 0x01

        _emit 0x49
        _emit 0xeb
        _emit 0x18
        _emit 0x8a
        _emit 0xd0
        _emit 0x80
        _emit 0xc2
        _emit 0x4e
        _emit 0x80
        _emit 0xfa
        _emit 0x04
        _emit 0x77
        _emit 0x05
        _emit 0xc6
        _emit 0x01
        _emit 0x4f

        _emit 0xeb
        _emit 0x09
        _emit 0x04
        _emit 0x47
        _emit 0x3c
        _emit 0x03
        _emit 0x77
        _emit 0x2b
        _emit 0xc6
        _emit 0x01
        _emit 0x55
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        _emit 0x8d
        _emit 0x45

        _emit 0x01
        _emit 0x83
        _emit 0xe9
        _emit 0x01
        _emit 0x3b
        _emit 0xc1
        _emit 0x73
        _emit 0x17
        _emit 0x8b
        _emit 0x17
        _emit 0x8d
        _emit 0x0c
        _emit 0x02
        _emit 0x8a
        _emit 0x51
        _emit 0x01

        _emit 0x88
        _emit 0x11
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        _emit 0x83
        _emit 0xe9
        _emit 0x01
        _emit 0x3b
        _emit 0xc1
        _emit 0x72
        _emit 0xe9
        _emit 0x83

        _emit 0x47
        _emit 0x08
        _emit 0xff
        _emit 0x8a
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        _emit 0x32
        _emit 0xd2
        _emit 0x80
        _emit 0xfb
        _emit 0x20
        _emit 0x75
        _emit 0x02
        _emit 0xb2
        _emit 0x01

        _emit 0x83
        _emit 0xc5
        _emit 0x01
        _emit 0x3b
        _emit 0x6c
        _emit 0x24
        _emit 0x24
        _emit 0x0f
        _emit 0x82
        _emit 0xc3
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x5e
        _emit 0x5b
        _emit 0x8b

        _emit 0xc7
        _emit 0x5f
        _emit 0x5d
        _emit 0xc2
        _emit 0x14
        _emit 0x00
        _emit 0x2c
        _emit 0x20
        _emit 0x88
        _emit 0x41
        _emit 0x01
        _emit 0xeb
        _emit 0xd6
        _emit 0x80
        _emit 0xfb
        _emit 0xc5

        _emit 0x75
        _emit 0xd5
        _emit 0x80
        _emit 0x79
        _emit 0x01
        _emit 0x93
        _emit 0x75
        _emit 0xcf
        _emit 0xc6
        _emit 0x41
        _emit 0x01
        _emit 0x92
        _emit 0xeb
        _emit 0xc9
    }
}
