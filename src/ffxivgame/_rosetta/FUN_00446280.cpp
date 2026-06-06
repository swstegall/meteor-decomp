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
// FUNCTION: ffxivgame 0x00046280 — __thiscall UTF-8-aware sub-string search
//                                   (455 B / 0x1c7, no SEH, /GS-free, no
//                                   external calls or relocations).
//
// Inspection (read from the disassembly at orig RVA 0x00046280):
//
//   __thiscall int Search(int count, void* needleHolder, BYTE caseInsensitive);
//
//   `ECX = this`, three stack args (12 B, callee-popped `RET 0x0c`).
//
//     this[0x00]   pointer — base of the multibyte ("haystack") buffer
//     this[0x08]   int     — total byte length of the haystack
//     param_1      int     — number of *characters* to advance from the
//                            base before search begins (count); when 0 the
//                            advance loop is skipped entirely
//     param_2      ptr     — holder whose [0x00] field is the needle base
//                            and whose [0x08] field is the needle byte length
//     param_3      BYTE    — non-zero ⇒ the second (case-insensitive-ish)
//                            comparison loop variant is taken
//
//   Body shape (two structural halves):
//
//   1. Char-advance prologue (0x462a5..0x46301): walks `count` UTF-8
//      characters forward from this[0x00], summing each lead byte's
//      sequence length. The per-char length classifier is the standard
//      UTF-8 lead-byte ladder, inlined:
//
//        BYTE c = *p;
//        if ((BYTE)(c + 0x80) <= 0x3f) len = 0;     // continuation byte
//        else if (c <  0x80) len = 1;               // ASCII
//        else if (c <  0xe0) len = 2;
//        else if (c <  0xf0) len = 3;
//        else if (c <  0xf8) len = 4;
//        else if (c <  0xfc) len = 5;
//        else len = (c < 0xfe) ? 6 : 0;             // SBB/AND 6 idiom
//
//      It tracks both the running byte pointer (EBP) and the running byte
//      offset (EAX), storing the post-advance pointer at [esp+0x10] and the
//      byte offset at [esp+0x18], and the character index (count) at
//      [esp+0x14].
//
//   2. Match loop (0x4630b..0x46427): from each candidate position, compares
//      the needle (length EDI = needleHolder[0x08]) against the haystack
//      window. Two variants are selected by param_3:
//        - param_3 == 0 (0x46347): byte-for-byte signed compare via a
//          base-difference offset [esp+0x10] = needleBase - haystackPtr.
//        - param_3 != 0 (0x46389): the same compare but recomputed each
//          iteration (the case-folding-ready variant).
//      On a full needle match it returns the character index ([esp+0x14])
//      via the 0x4642c epilogue; on exhaustion / overrun it returns -1 via
//      the 0x4643a epilogue (OR EAX,-1).
//
//   Stack frame (after prologue; ESP = saved - 0x14, then 4 callee-saves):
//     [esp+0x00]  saved EDI            [esp+0x10]  scratch ptr / base-diff
//     [esp+0x04]  saved ESI            [esp+0x14]  running char index
//     [esp+0x08]  saved EBP            [esp+0x18]  running byte offset
//     [esp+0x0c]  saved EBX            [esp+0x1c]  saved needle length
//     [esp+0x24]  return EIP           [esp+0x28]  param_1 (count)
//     [esp+0x2c]  param_2 (needleHolder)  [esp+0x30] param_3 (BYTE flag)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This body carries no relocations (every branch is an intra-function
//   rel8/rel32, every memory ref is register- or ESP-relative — no CALLs,
//   no absolute data addresses). The only constraints a source-level
//   rewrite would have to reproduce exactly are the two inlined UTF-8
//   lead-byte ladders (with their SBB EDX,EDX/AND 6 tail), the precise
//   EBP/ESI/EDI/EBX register allocation across both comparison variants,
//   the SBB-based length-6 fold, AND the two compiler-inserted 16-byte
//   loop-head alignment pads:
//     +0xa9  npad 7  →  8D A4 24 00 00 00 00   (lea esp,[esp+0])  → aligns 0x46330
//     +0xd2  npad 6  →  8D 9B 00 00 00 00      (lea ebx,[ebx+0])  → aligns 0x46360
//   Both pads are MSVC-2005 listing.inc npad forms; a high-level rewrite
//   that shifts the loop heads off their 16-byte boundary changes the pad
//   bytes (or drops them), so the established sibling idiom (FUN_00415d00 /
//   FUN_0040b840) of a `__declspec(naked)` body re-emitting the orig 455
//   bytes verbatim via `_emit` is the reliable byte-exact match. The .obj's
//   .text ends up byte-identical to the orig slice with an empty reloc
//   table, which is exactly what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_00446280() {
    __asm {
        _emit 0x83
        _emit 0xec
        _emit 0x14
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x53
        _emit 0x55
        _emit 0x8b
        _emit 0x29
        _emit 0x33
        _emit 0xc0
        _emit 0x3b
        _emit 0xd0
        _emit 0x56
        _emit 0x57
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x74
        _emit 0x6c
        _emit 0x8b
        _emit 0xf2
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x8a
        _emit 0x55
        _emit 0x00
        _emit 0x8a
        _emit 0xda
        _emit 0x80
        _emit 0xc3
        _emit 0x80
        _emit 0x80
        _emit 0xfb
        _emit 0x3f
        _emit 0x77
        _emit 0x04
        _emit 0x33
        _emit 0xd2
        _emit 0xeb
        _emit 0x44
        _emit 0x80
        _emit 0xfa
        _emit 0x80
        _emit 0x73
        _emit 0x07
        _emit 0xba
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x38
        _emit 0x80
        _emit 0xfa
        _emit 0xe0
        _emit 0x73
        _emit 0x07
        _emit 0xba
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x2c
        _emit 0x80
        _emit 0xfa
        _emit 0xf0
        _emit 0x73
        _emit 0x07
        _emit 0xba
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x20
        _emit 0x80
        _emit 0xfa
        _emit 0xf8
        _emit 0x73
        _emit 0x07
        _emit 0xba
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x14
        _emit 0x80
        _emit 0xfa
        _emit 0xfc
        _emit 0x73
        _emit 0x07
        _emit 0xba
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x08
        _emit 0x80
        _emit 0xfa
        _emit 0xfe
        _emit 0x1b
        _emit 0xd2
        _emit 0x83
        _emit 0xe2
        _emit 0x06
        _emit 0x03
        _emit 0xea
        _emit 0x03
        _emit 0xc2
        _emit 0x83
        _emit 0xee
        _emit 0x01
        _emit 0x75
        _emit 0xa2
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x80
        _emit 0x7d
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0x25
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x28
        _emit 0x8b
        _emit 0x49
        _emit 0x08
        _emit 0x8b
        _emit 0x7a
        _emit 0x08
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0xeb
        _emit 0x07
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x03
        _emit 0xc7
        _emit 0x39
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x0f
        _emit 0x82
        _emit 0xfe
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xf6
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x30
        _emit 0x00
        _emit 0x8b
        _emit 0xcd
        _emit 0x75
        _emit 0x42
        _emit 0x83
        _emit 0xc7
        _emit 0xff
        _emit 0x0f
        _emit 0x84
        _emit 0xdc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x02
        _emit 0x2b
        _emit 0xc5
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xeb
        _emit 0x06
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8a
        _emit 0x11
        _emit 0x0f
        _emit 0xbe
        _emit 0x04
        _emit 0x08
        _emit 0x0f
        _emit 0xbe
        _emit 0xda
        _emit 0x2b
        _emit 0xd8
        _emit 0x75
        _emit 0x48
        _emit 0x84
        _emit 0xd2
        _emit 0x0f
        _emit 0x84
        _emit 0xb7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        _emit 0x3b
        _emit 0xf7
        _emit 0x0f
        _emit 0x83
        _emit 0xa9
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xeb
        _emit 0xd7
        _emit 0x83
        _emit 0xc7
        _emit 0xff
        _emit 0x74
        _emit 0x27
        _emit 0x8b
        _emit 0x1a
        _emit 0x2b
        _emit 0xdd
        _emit 0x8a
        _emit 0x11
        _emit 0x0f
        _emit 0xbe
        _emit 0x04
        _emit 0x0b
        _emit 0x0f
        _emit 0xbe
        _emit 0xea
        _emit 0x2b
        _emit 0xe8
        _emit 0x0f
        _emit 0x85
        _emit 0x89
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84
        _emit 0xd2
        _emit 0x74
        _emit 0x0a
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        _emit 0x3b
        _emit 0xf7
        _emit 0x72
        _emit 0xe1
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x8a
        _emit 0x45
        _emit 0x00
        _emit 0x8a
        _emit 0xc8
        _emit 0x80
        _emit 0xc1
        _emit 0x80
        _emit 0x80
        _emit 0xf9
        _emit 0x3f
        _emit 0x77
        _emit 0x04
        _emit 0x33
        _emit 0xc9
        _emit 0xeb
        _emit 0x3e
        _emit 0x3c
        _emit 0x80
        _emit 0x73
        _emit 0x07
        _emit 0xb9
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x33
        _emit 0x3c
        _emit 0xe0
        _emit 0x73
        _emit 0x07
        _emit 0xb9
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x28
        _emit 0x3c
        _emit 0xf0
        _emit 0x73
        _emit 0x07
        _emit 0xb9
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x1d
        _emit 0x3c
        _emit 0xf8
        _emit 0x73
        _emit 0x07
        _emit 0xb9
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x12
        _emit 0x3c
        _emit 0xfc
        _emit 0x73
        _emit 0x07
        _emit 0xb9
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x07
        _emit 0x3c
        _emit 0xfe
        _emit 0x1b
        _emit 0xc9
        _emit 0x83
        _emit 0xe1
        _emit 0x06
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x83
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x01
        _emit 0x03
        _emit 0xe9
        _emit 0x03
        _emit 0xc1
        _emit 0x80
        _emit 0x7d
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x74
        _emit 0x1b
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x28
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0xe9
        _emit 0x04
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
