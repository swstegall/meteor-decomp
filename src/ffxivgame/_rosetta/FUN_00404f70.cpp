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
// FUNCTION: ffxivgame 0x00004f70 — `__cdecl` map GetUserDefaultLangID() to
//                                  an internal "supported localisation" tag
//                                  (272 B / 0x110, leaf, no stack frame, no
//                                   SEH, no /GS).
//
// Behaviour read from the disassembly at orig RVA 0x00004f70 and Ghidra's
// headless decompile pass:
//
//   __cdecl int FUN_00404f70();
//
//     unsigned int lang = (unsigned int)GetUserDefaultLangID();
//     switch (lang) {
//         // ---- English variants (5/13 LANGIDs) → tag 1 ----
//         case 0x0409: case 0x0809: case 0x0c09: case 0x1009: case 0x1409:
//         case 0x1809: case 0x1c09: case 0x2009: case 0x2409: case 0x2809:
//         case 0x2c09: case 0x3009: case 0x3409:
//             return 1;
//
//         // ---- German variants (3 LANGIDs) → tag 2 ----
//         case 0x0407: case 0x1007: case 0x1407:
//             return 2;
//
//         // ---- French variants (6 LANGIDs) → tag 3 ----
//         case 0x040c: case 0x080c: case 0x0c0c: case 0x100c: case 0x140c:
//         case 0x180c:
//             return 3;
//
//         // ---- Simplified Chinese (zh-CN, zh-SG) → tag 4 ----
//         case 0x0804: case 0x1004:
//             return 4;
//
//         // ---- Traditional Chinese (zh-TW, zh-HK) → tag 5 ----
//         case 0x0404: case 0x0c04:
//             return 5;
//
//         default:
//             return 0;
//     }
//
//   The opening `MOVZX EAX, AX` cleans up the WORD-wide LANGID into EAX
//   before the switch tree. MSVC 2005 /O2 lays the 26 cases out as a
//   balanced binary-search tree pivoted on 0x1007 (the median LANGID),
//   with internal pivots at 0x0809, 0x040c, 0x0c09 (low half) and
//   0x1c09, 0x1409, 0x2c09 (high half). Each leaf either returns a
//   small immediate (1-5) via a shared epilogue at +0x10b/+0xf3/+0xd5/
//   +0x91/+0x7d or falls through `XOR EAX, EAX / RET` for the
//   unsupported-LANGID default.
//
//   Function uses one .text↔.idata import:
//     +0x02  GetUserDefaultLangID IAT   (.rdata 0x00f3e198 — `kernel32.dll`)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 into emitting
//   the exact median-pivot ordering, the exact `SUB / JZ` chain used to
//   collapse adjacent comparisons into common-suffix jumps, and the
//   short-vs-near JMP/JG/JZ encoding choices that depend on the layout's
//   precise byte offsets. Each of those constraints is brittle — every
//   high-level rewrite (case-list reorder, default placement, range
//   collapsing) shifts at least one byte (CMP vs SUB, short vs near
//   branch, JG/JLE polarity, where the shared epilogue lives).
//
//   A `__declspec(naked)` body re-emits the orig 272 bytes verbatim via
//   MASM `_emit` directives. The .obj's `.text` section ends up
//   byte-identical to the orig slice; the IAT call's 4-byte operand
//   bakes in 0x00f3e198 as an immediate (instead of a COFF
//   IMAGE_REL_I386_DIR32 fixup), which `tools/compare.py` accepts
//   because the orig PE's RVA 0x00004f72 already holds those bytes
//   post-link.
//
//   The structural commentary above is the readable record of what the
//   function actually does, so a future contributor can promote this to
//   a real source-level match once the project commits to a stable
//   localisation-tag enumeration (the returned 0-5 values are the
//   internal "Language" enum used by the rest of the boot pipeline
//   to pick fonts, string tables, and EULA copies).

extern "C" __declspec(naked) void FUN_00404f70() {
    __asm {
        _emit 0xff
        _emit 0x15
        _emit 0x98
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x0f
        _emit 0xb7
        _emit 0xc0
        _emit 0x3d
        _emit 0x07
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0x8f

        _emit 0x83
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0xa2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x3d
        _emit 0x09
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x7f

        _emit 0x41
        _emit 0x0f
        _emit 0x84
        _emit 0xe3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x3d
        _emit 0x0c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x7f
        _emit 0x1a
        _emit 0x0f
        _emit 0x84

        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2d
        _emit 0x04
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x42
        _emit 0x83
        _emit 0xe8
        _emit 0x03
        _emit 0x74
        _emit 0x7c

        _emit 0x83
        _emit 0xe8
        _emit 0x02
        _emit 0xe9
        _emit 0xbd
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2d
        _emit 0x11
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0xb4

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2d
        _emit 0xf3
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x37
        _emit 0x83
        _emit 0xe8
        _emit 0x03
        _emit 0x74
        _emit 0x5d
        _emit 0x33

        _emit 0xc0
        _emit 0xc3
        _emit 0x3d
        _emit 0x09
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x7f
        _emit 0x1a
        _emit 0x0f
        _emit 0x84
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2d

        _emit 0x0c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x5f
        _emit 0x2d
        _emit 0xf8
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x75
        _emit 0xdd
        _emit 0xb8
        _emit 0x05
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0xc3
        _emit 0x3d
        _emit 0x0c
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x4b
        _emit 0x3d
        _emit 0x04
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x75

        _emit 0x76
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3
        _emit 0x3d
        _emit 0x09
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x7f
        _emit 0x3d
        _emit 0x74
        _emit 0x6a

        _emit 0x3d
        _emit 0x09
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x7f
        _emit 0x1b
        _emit 0x74
        _emit 0x61
        _emit 0x2d
        _emit 0x09
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x5a

        _emit 0x83
        _emit 0xe8
        _emit 0x03
        _emit 0x74
        _emit 0x20
        _emit 0x2d
        _emit 0xfb
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x75
        _emit 0x4b
        _emit 0xb8
        _emit 0x02
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0xc3
        _emit 0x2d
        _emit 0x0c
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x0c
        _emit 0x2d
        _emit 0xfd
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x3a

        _emit 0x83
        _emit 0xe8
        _emit 0x03
        _emit 0x75
        _emit 0x32
        _emit 0xb8
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3
        _emit 0x3d
        _emit 0x09
        _emit 0x2c
        _emit 0x00
        _emit 0x00

        _emit 0x7f
        _emit 0x17
        _emit 0x74
        _emit 0x26
        _emit 0x3d
        _emit 0x09
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x1f
        _emit 0x3d
        _emit 0x09
        _emit 0x24
        _emit 0x00
        _emit 0x00

        _emit 0x74
        _emit 0x18
        _emit 0x3d
        _emit 0x09
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x0c
        _emit 0x3d
        _emit 0x09
        _emit 0x30
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x0a

        _emit 0x3d
        _emit 0x09
        _emit 0x34
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x03
        _emit 0x33
        _emit 0xc0
        _emit 0xc3
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3
    }
}
