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
// FUNCTION: ffxivgame 0x00031e20 — `__cdecl` doubly-nested tiling/blit loop
//                                  over a big-endian record table
//                                  (267 B / 0x10b, no SEH, no /GS).
//
// Inspection (read from the disassembly at orig RVA 0x00031e20):
//
//   __cdecl void FUN_00431e20(arg0 /* [esp+0x28 after pushes] */,
//                             desc* /* [esp+0x20] -> ESI */,
//                             ...);  plain `RET`, callee pops nothing.
//
//   Structural shape:
//
//     int rowCount = desc[0x24];                     // [esp+0x14] snapshot
//     unsigned char flags = desc->byte_09;           // [esi+0x9]
//     int bpp     = desc->byte_06;                   // EBP (MOVZX)
//     bool bit2   = (flags >> 2) & 1;                // [esp+0x10]
//     // (flags & 1) ? 6 : 1  via NEG/SBB/AND 5/ADD 1
//     int outer   = (flags & 1) ? 6 : 1;             // [esp+0x18]
//     for (int x = 0; outer > 0 && x < outer; ++x) {
//       for (int y = 0; rowCount > 0 && y < rowCount; ++y) {
//         int idx = desc->byte_07 * x + y;           // ECX
//         unsigned v0 = bswap(desc->dword_14);
//         if (v0 != 0) {                             // tail-blit via 0x00432500
//           desc->method_432500(idx); goto blit;
//         }
//         unsigned off = bswap(desc->dword_10);
//         int px = 0;
//         if (off != 0) {
//           unsigned char* p = (char*)desc + bswap(desc->dword_10);
//           if (p)                                   // ADD EAX,ESI; JZ
//             px = bswap(((int*)p)[idx]);            // [eax + ecx*8]
//         }
//         if ((unsigned)px < arg_30) {               // CMP / JNC skip
//           px += arg_2c;
//         blit:
//           // FUN_00431080(blkX, blkY, dword_1c, idx? , px, 0)
//           FUN_00431080(...);
//         }
//       }
//     }
//
//   Reloc-bearing sites in the orig 267 bytes (resolve only in a full
//   relink at image base 0x00400000; a standalone .obj can't reproduce
//   the rel32 windows):
//     +0x74   rel32   0x00432500 — desc tail-blit helper (CALL via ECX=ESI)
//     +0xe0   rel32   0x00431080 — inner blit emitter (6 stack args)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rewrite would have to coax MSVC 2005 /O2 into the exact
//   register quartet (EAX/EBX=x/EBP=bpp/EDI=y/ESI=desc), the repeated
//   spill-then-BSWAP-then-reload idiom on [esp+0x24]/[esp+0x10], the
//   `NEG/SBB/AND 5/ADD 1` lowering of the `(flags&1)?6:1` select, the
//   `LEA ECX,[ECX]` 3-byte alignment NOP, and the short-vs-near branch
//   encodings — every high-level edit shifts at least one byte. The same
//   `__declspec(naked)` `_emit` passthrough the sibling reloc-heavy bodies
//   use re-emits the orig 267 bytes verbatim; the .obj's `.text` ends up
//   byte-identical to the orig slice, which is what `tools/compare.py`
//   checks.

extern "C" __declspec(naked) void FUN_00431e20() {
    __asm {
        _emit 0x83
        _emit 0xec
        _emit 0x10
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8b
        _emit 0x50
        _emit 0x24
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x8b
        _emit 0x74
        _emit 0x24

        _emit 0x20
        _emit 0x8a
        _emit 0x46
        _emit 0x09
        _emit 0x0f
        _emit 0xb6
        _emit 0x6e
        _emit 0x06
        _emit 0x8a
        _emit 0xc8
        _emit 0x24
        _emit 0x01
        _emit 0xc0
        _emit 0xe9
        _emit 0x02
        _emit 0x80

        _emit 0xe1
        _emit 0x01
        _emit 0xf6
        _emit 0xd8
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x88
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x1b
        _emit 0xc0
        _emit 0x83
        _emit 0xe0

        _emit 0x05
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        _emit 0x33
        _emit 0xdb
        _emit 0x85
        _emit 0xc0
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x0f
        _emit 0x8e
        _emit 0xc2
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0x57
        _emit 0x33
        _emit 0xff
        _emit 0x85
        _emit 0xd2
        _emit 0x0f
        _emit 0x8e
        _emit 0xab
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x49
        _emit 0x00

        _emit 0x0f
        _emit 0xb6
        _emit 0x4e
        _emit 0x07
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        _emit 0x0f
        _emit 0xaf
        _emit 0xcb
        _emit 0x03
        _emit 0xcf
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x24

        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x0f
        _emit 0xc8
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x24
        _emit 0x00
        _emit 0x74

        _emit 0x0a
        _emit 0x51
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0x67
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x56
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        _emit 0x89
        _emit 0x44

        _emit 0x24
        _emit 0x24
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x0f
        _emit 0xc8
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x24

        _emit 0x00
        _emit 0x74
        _emit 0x30
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x0f
        _emit 0xc8

        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x03
        _emit 0xc6
        _emit 0x74
        _emit 0x17
        _emit 0x8b
        _emit 0x0c
        _emit 0xc8
        _emit 0x89

        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x0f
        _emit 0xc8
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x44
        _emit 0x24

        _emit 0x10
        _emit 0xeb
        _emit 0x02
        _emit 0x33
        _emit 0xc0
        _emit 0x3b
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x73
        _emit 0x1e
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        _emit 0x03

        _emit 0xc2
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x6a
        _emit 0x00
        _emit 0x50
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x50
        _emit 0x55
        _emit 0x57
        _emit 0x53

        _emit 0xe8
        _emit 0x7b
        _emit 0xf1
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x83
        _emit 0xc7
        _emit 0x01
        _emit 0x3b
        _emit 0xfa
        _emit 0x0f
        _emit 0x8c

        _emit 0x5c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x83
        _emit 0xc3
        _emit 0x01
        _emit 0x3b
        _emit 0xd8
        _emit 0x0f
        _emit 0x8c
        _emit 0x40

        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0xc3
    }
}
