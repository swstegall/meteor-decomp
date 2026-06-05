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
// FUNCTION: ffxivgame 0x00045550 — UTF-8 lead-byte sequence length classifier
//                                  (__cdecl, 75 B / 0x4b, zero relocs)
//
// __cdecl int utf8_seq_len(unsigned char c)
//
// Classifies a single byte by its UTF-8 lead-byte category and returns the
// length (in bytes) of the encoded sequence it introduces — or 0 when the
// byte cannot legally start a sequence:
//
//     0x00..0x7f → 1   (ASCII / single byte)
//     0x80..0xbf → 0   (continuation byte — never a lead)
//     0xc0..0xdf → 2   (2-byte sequence lead)
//     0xe0..0xef → 3   (3-byte sequence lead)
//     0xf0..0xf7 → 4   (4-byte sequence lead)
//     0xf8..0xfb → 5   (5-byte sequence lead)
//     0xfc..0xfd → 6   (6-byte sequence lead)
//     0xfe..0xff → 0   (invalid)
//
// Logical structure:
//
//   int utf8_seq_len(unsigned char c) {
//       if (c >= 0x80 && c < 0xc0) return 0;   // continuation byte first
//       if (c < 0x80) return 1;
//       if (c < 0xe0) return 2;
//       if (c < 0xf0) return 3;
//       if (c < 0xf8) return 4;
//       if (c < 0xfc) return 5;
//       return (c < 0xfe) ? 6 : 0;             // SBB/AND branchless tail
//   }
//
// The leading continuation-byte test compiles to MSVC's range-check idiom
// `(unsigned char)(c + 0x80) <= 0x3f` (ADD CL,0x80 / CMP CL,0x3f / JA), and
// the final `(c < 0xfe) ? 6 : 0` lowers to the carry trick
// `CMP AL,0xfe / SBB EAX,EAX / AND EAX,6`.
//
// Reloc-bearing sites: NONE. Every immediate and displacement in the
// 75-byte slice is a self-contained constant — no CALL rel32, no IAT load,
// no DIR32 image-base load. The .obj's `.text` matches the orig slice
// byte-for-byte with zero linker fixups required.
//
// Reconstruction strategy — naked-asm byte passthrough. Driving MSVC 2005
// /O2 to emit this exact compare-chain register schedule (CL pre-add for the
// first range check, the SBB/AND branchless tail) from surface C++ is not
// reliably reproducible, so per the established sibling idiom (FUN_004086a0)
// the 75 orig bytes are re-emitted verbatim via MASM `_emit` directives.

extern "C" __declspec(naked) void FUN_00445550() {
    __asm {
        // 00045550: mov al, byte ptr [esp+0x4]
        _emit 0x8a
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00045554: mov cl, al
        _emit 0x8a
        _emit 0xc8
        // 00045556: add cl, 0x80
        _emit 0x80
        _emit 0xc1
        _emit 0x80
        // 00045559: cmp cl, 0x3f
        _emit 0x80
        _emit 0xf9
        _emit 0x3f
        // 0004555c: ja +0x3 -> 0x00445561
        _emit 0x77
        _emit 0x03
        // 0004555e: xor eax, eax
        _emit 0x33
        _emit 0xc0
        // 00045560: ret
        _emit 0xc3
        // ----- 0x00445561 -----
        // 00045561: cmp al, 0x80
        _emit 0x3c
        _emit 0x80
        // 00045563: jnc +0x6 -> 0x0044556b
        _emit 0x73
        _emit 0x06
        // 00045565: mov eax, 0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004556a: ret
        _emit 0xc3
        // ----- 0x0044556b -----
        // 0004556b: cmp al, 0xe0
        _emit 0x3c
        _emit 0xe0
        // 0004556d: jnc +0x6 -> 0x00445575
        _emit 0x73
        _emit 0x06
        // 0004556f: mov eax, 0x2
        _emit 0xb8
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00045574: ret
        _emit 0xc3
        // ----- 0x00445575 -----
        // 00045575: cmp al, 0xf0
        _emit 0x3c
        _emit 0xf0
        // 00045577: jnc +0x6 -> 0x0044557f
        _emit 0x73
        _emit 0x06
        // 00045579: mov eax, 0x3
        _emit 0xb8
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004557e: ret
        _emit 0xc3
        // ----- 0x0044557f -----
        // 0004557f: cmp al, 0xf8
        _emit 0x3c
        _emit 0xf8
        // 00045581: jnc +0x6 -> 0x00445589
        _emit 0x73
        _emit 0x06
        // 00045583: mov eax, 0x4
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00045588: ret
        _emit 0xc3
        // ----- 0x00445589 -----
        // 00045589: cmp al, 0xfc
        _emit 0x3c
        _emit 0xfc
        // 0004558b: jnc +0x6 -> 0x00445593
        _emit 0x73
        _emit 0x06
        // 0004558d: mov eax, 0x5
        _emit 0xb8
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00045592: ret
        _emit 0xc3
        // ----- 0x00445593 -----
        // 00045593: cmp al, 0xfe
        _emit 0x3c
        _emit 0xfe
        // 00045595: sbb eax, eax
        _emit 0x1b
        _emit 0xc0
        // 00045597: and eax, 0x6
        _emit 0x83
        _emit 0xe0
        _emit 0x06
        // 0004559a: ret
        _emit 0xc3
    }
}
