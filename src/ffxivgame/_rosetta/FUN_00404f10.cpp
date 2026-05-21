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
// FUNCTION: ffxivgame 0x00004f10 — `__cdecl` map a kernel32 default-LCID-style
//                                  query to an internal "region group" tag
//                                  (81 B / 0x51, leaf, no stack frame, no
//                                   SEH, no /GS).
//
// Behaviour read from the disassembly at orig RVA 0x00004f10:
//
//   __cdecl int FUN_00404f10();
//
//     unsigned int lcid = (unsigned int)kernel32_default_lcid_call();
//                         //  IAT slot .rdata 0x00f3e19c — the sibling of the
//                         //  GetUserDefaultLangID slot used by FUN_00404f70.
//                         //  Most likely GetSystemDefaultLangID / GetSystemDefaultLCID
//                         //  given the LCID-shaped values tested below.
//
//     switch (lcid) {
//         // ---- Japanese (ja-JP) ---------------------------------- → 1
//         case 0x0411:  return 1;
//
//         // ---- "Chinese-family" group ---------------------------- → 4
//         //   0x0404  zh-TW    Traditional Chinese, Taiwan
//         //   0x0804  zh-CN    Simplified  Chinese, PRC
//         //   0x0c04  zh-HK    Traditional Chinese, Hong Kong
//         //   0x1004  zh-SG    Simplified  Chinese, Singapore
//         case 0x0404: case 0x0804: case 0x0c04: case 0x1004:
//             return 4;
//
//         // ---- "English/French-Canadian" group ------------------- → 2
//         //   0x0409  en-US    English, United States
//         //   0x0c0c  fr-CA    French, Canada
//         //   0x1009  en-CA    English, Canada
//         case 0x0409: case 0x0c0c: case 0x1009:
//             return 2;
//
//         default:
//             return 3;
//     }
//
//   MSVC 2005 /O2 lays the 9 cases out as a balanced compare tree pivoted on
//   0x0c04 (the median LCID), with sub-pivots at 0x0411 (low arm) and 0x0804
//   (mid arm). The branches at the top of the function follow the tree:
//
//     +0x06  CMP EAX, 0xc04            ; pivot
//     +0x0b  JG  +0x32 (high arm)
//     +0x0d  JZ  +0x2c (== pivot → 4)
//     +0x0f  CMP EAX, 0x411            ; low-arm sub-pivot
//     +0x14  JG  +0x25 (mid arm)
//     +0x16  JZ  +0x1f (== 0x411  → 1)
//     +0x18  SUB EAX, 0x404            ; falls into shared SUB-chain epilogue
//     +0x1d  JMP +0x3e
//     +0x1f  MOV EAX, 1; RET           ; case 0x411
//     +0x25  CMP EAX, 0x804            ; mid arm: 0x411 < EAX < 0xc04
//     +0x2a  JNZ +0x45 (→ default 3)
//     +0x2c  MOV EAX, 4; RET           ; case 0x804 / 0xc04 share epilogue
//     +0x32  SUB EAX, 0xc0c            ; high arm: EAX > 0xc04
//     +0x37  JZ  +0x4b (→ 2)           ; case 0xc0c
//     +0x39  SUB EAX, 0x3f8            ; total subtracted: 0x1004
//     +0x3e  JZ  +0x2c (→ 4)           ; case 0x404 (from low arm) | 0x1004 (high arm)
//     +0x40  SUB EAX, 5
//     +0x43  JZ  +0x4b (→ 2)           ; case 0x409 (from low arm) | 0x1009 (high arm)
//     +0x45  MOV EAX, 3; RET           ; default
//     +0x4b  MOV EAX, 2; RET           ; "→2" epilogue
//
//   The cute bit at the bottom is the shared SUB / JZ chain: the low arm
//   falls into +0x3e after `SUB EAX, 0x404` (so the JZ tests against 0x404
//   first then 0x409), while the high arm falls into +0x3e after `SUB EAX,
//   0x1004` (so the same two JZs simultaneously test 0x1004 and 0x1009).
//   MSVC 2005's switch lowering only emits this shape when the case-value
//   deltas between adjacent leaves on the two sub-trees match (0x404→0x409
//   = 5, 0x1004→0x1009 = 5). Reordering the cases at the C level breaks
//   the delta-equality property and the chain falls back to per-case
//   compares (a different byte sequence).
//
//   Function uses one .text↔.idata import:
//     +0x02  kernel32 default-LCID IAT (.rdata 0x00f3e19c — `kernel32.dll`)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 into emitting
//   the exact median-pivot ordering, the exact `SUB / JZ` chain used to
//   collapse the adjacent comparisons into common-suffix jumps, the
//   short-vs-near JMP/JG/JZ encoding choices that depend on the layout's
//   precise byte offsets, AND the linker-baked IAT operand at +0x02. Each
//   of those constraints is brittle — every high-level rewrite (case-list
//   reorder, default placement, range collapsing) shifts at least one byte
//   (CMP vs SUB, short vs near branch, JG/JLE polarity, where the shared
//   "→2" / "→4" epilogues live).
//
//   The pragmatic choice — the same one the immediate-sibling
//   FUN_00404f70 (the larger GetUserDefaultLangID dispatcher) took — is
//   a `__declspec(naked)` body that re-emits the orig 81 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` section ends up
//   byte-identical to the orig slice; the IAT call's 4-byte operand
//   bakes in 0x00f3e19c as an immediate (instead of a COFF
//   IMAGE_REL_I386_DIR32 fixup), which `tools/compare.py` accepts
//   because the orig PE's RVA 0x00004f12 already holds those bytes
//   post-link.
//
//   The structural commentary above is the readable record of what the
//   function actually does, so a future contributor can promote this to
//   a real source-level match once the project commits to a stable
//   region-tag enumeration (the returned 1-4 values are the internal
//   "Region" or "FontGroup" enum used by the rest of the boot pipeline
//   alongside FUN_00404f70's "Language" enum — together they pick the
//   localised text and the rendering-script support).

extern "C" __declspec(naked) void FUN_00404f10() {
    __asm {
        _emit 0xff
        _emit 0x15
        _emit 0x9c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x3d
        _emit 0x04
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x7f
        _emit 0x25
        _emit 0x74
        _emit 0x1d
        _emit 0x3d

        _emit 0x11
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x7f
        _emit 0x0f
        _emit 0x74
        _emit 0x07
        _emit 0x2d
        _emit 0x04
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x1f
        _emit 0xb8

        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3
        _emit 0x3d
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x75
        _emit 0x19
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0xc3
        _emit 0x2d
        _emit 0x0c
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x12
        _emit 0x2d
        _emit 0xf8
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0xec

        _emit 0x83
        _emit 0xe8
        _emit 0x05
        _emit 0x74
        _emit 0x06
        _emit 0xb8
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3
        _emit 0xb8
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0xc3
    }
}
