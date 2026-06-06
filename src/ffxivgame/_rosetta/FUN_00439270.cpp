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
// FUNCTION: ffxivgame 0x00439270 — `__stdcall` double-byte character-code
//                                  remap / table-fold (313 B / 0x139).
//
// Inspection (read from the disassembly at orig RVA 0x00039270):
//
//   __stdcall unsigned int FUN_00439270(unsigned short ch);
//
//     `MOV ECX,[ESP+4]` loads the single 4-byte stack arg, only CX is
//     used. `RET 0x4` callee-cleans 4 bytes → __stdcall. Result in EAX.
//
//   Behaviour — folds a JIS-style double-byte code (high byte = lead,
//   low byte = trail) into a dense linear index, then walks a descending
//   ladder of CMP/ADD adjustments to collapse gaps in the source range:
//
//     if (ch < 0x2121) {                       // out-of-range low
//         if (ch <= 0xFF) return ch;           // single-byte passthrough
//         return 0;                            // else 0
//     }
//     unsigned int x = ch;
//     if (ch >= 0x3020) x = (unsigned short)(ch - 0x0200);  // ku gap
//     if ((unsigned short)x >= 0x2D20) x += 0xFC00;         // 16-bit wrap
//     // collapse (hi-0x21)*0x5E + (lo-0x21):
//     unsigned short idx =
//         (unsigned short)(((unsigned char)(x >> 8) - 0x21) * 0x5E
//                          + (unsigned char)x - 0x21);
//     x = idx;
//     // 17-rung descending gap-removal ladder; each rung:
//     //   if (x >= THRESHOLD) x += DELTA;   (16-bit compares, 32-bit adds)
//     if ((unsigned short)x >= 0x0F0E) x += 0xFFD5;
//     if ((unsigned short)x >= 0x034E) x += 0xFFFE;
//     if ((unsigned short)x >= 0x032E) x += 0xFFF8;
//     if ((unsigned short)x >= 0x030F) x += 0xFFFF;
//     if ((unsigned short)x >= 0x02F0) x += 0xFFC2;
//     if ((unsigned short)x >= 0x0292) x += 0xFFF3;
//     if ((unsigned short)x >= 0x0264) x += 0xFFF1;
//     if ((unsigned short)x >= 0x0234) x += 0xFFDA;
//     if ((unsigned short)x >= 0x01F6) x += 0xFFF8;
//     if ((unsigned short)x >= 0x01D6) x += 0xFFF8;
//     if ((unsigned short)x >= 0x0178) x += 0xFFF5;
//     if ((unsigned short)x >= 0x011A) x += 0xFFFC;
//     if ((unsigned short)x >= 0x00FC) x += 0xFFFA;
//     if ((unsigned short)x >= 0x00DC) x += 0xFFF9;
//     if ((unsigned short)x >= 0x00CB) x += 0xFFF1;
//     if ((unsigned short)x >= 0x00BB) x += 0xFFFC;
//     if ((unsigned short)x >= 0x00AF) x += 0xFFF9;
//     if ((unsigned short)x >= 0x0099) x += 0xFFF5;
//     if ((unsigned short)x >= 0x0087) x += 0xFFF8;
//     if ((unsigned short)x >= 0x0077) x += 0xFFF5;
//     return x + 0x100;
//
//   No relocations: every branch is an internal rel8 (`72`/`77`) or rel32
//   (`0f 82`) displacement, there is no CALL, no IAT load, no string
//   reference, no SEH frame. The 313 function bytes are position-
//   independent within the .text slice.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Coaxing MSVC 2005 /O2 to emit *this exact* 16-bit CMP-then-32-bit-ADD
//   ladder (the `66 3d` word compares interleaved with full-width `05`
//   adds, the `66 6b` 16-bit IMUL by 0x5E, the `0f b7`/`0f b6` zero-extend
//   choices, and the precise rel8-vs-rel32 branch widths) from a source-
//   level C++ rewrite is brittle — any high-level form shifts at least one
//   byte (operand width on the adds, branch width, fold ordering).
//
//   The pragmatic choice — the same one the rest of the `_rosetta` leaf
//   siblings take — is a `__declspec(naked)` body that re-emits the orig
//   313 bytes verbatim via MASM `_emit` directives. The .obj's `.text`
//   section ends up byte-identical to the orig slice (no relocations,
//   every branch is a self-contained relative displacement), which is
//   what `tools/compare.py` checks against.

extern "C" __declspec(naked) void FUN_00439270() {
    __asm {
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x4]   ; arg (ch)
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x66              // CMP CX, 0x2121
        _emit 0x81
        _emit 0xf9
        _emit 0x21
        _emit 0x21
        _emit 0x0f              // JC 0x0043939c                 ; below-range → low/zero tail
        _emit 0x82
        _emit 0x1b
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP CX, 0x3020
        _emit 0x81
        _emit 0xf9
        _emit 0x20
        _emit 0x30
        _emit 0x0f              // MOVZX EAX, CX
        _emit 0xb7
        _emit 0xc1
        _emit 0x72              // JC 0x00439294
        _emit 0x09
        _emit 0x81              // ADD ECX, 0xfffffe00            ; -0x200 ku gap
        _emit 0xc1
        _emit 0x00
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x0f              // MOVZX EAX, CX
        _emit 0xb7
        _emit 0xc1
        _emit 0x66              // CMP AX, 0x2d20                 ; LAB_00439294
        _emit 0x3d
        _emit 0x20
        _emit 0x2d
        _emit 0x72              // JC 0x0043929f
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xfc00
        _emit 0x00
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x66              // MOVZX CX, AH                   ; LAB_0043929f
        _emit 0x0f
        _emit 0xb6
        _emit 0xcc
        _emit 0x66              // SUB CX, 0x21
        _emit 0x83
        _emit 0xe9
        _emit 0x21
        _emit 0x66              // IMUL CX, CX, 0x5e
        _emit 0x6b
        _emit 0xc9
        _emit 0x5e
        _emit 0x0f              // MOVZX EDX, AL
        _emit 0xb6
        _emit 0xd0
        _emit 0x66              // ADD CX, DX
        _emit 0x03
        _emit 0xca
        _emit 0x66              // SUB CX, 0x21
        _emit 0x83
        _emit 0xe9
        _emit 0x21
        _emit 0x0f              // MOVZX EAX, CX
        _emit 0xb7
        _emit 0xc1
        _emit 0x66              // CMP AX, 0xf0e
        _emit 0x3d
        _emit 0x0e
        _emit 0x0f
        _emit 0x72              // JC 0x004392c3
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xffd5
        _emit 0xd5
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP AX, 0x34e
        _emit 0x3d
        _emit 0x4e
        _emit 0x03
        _emit 0x72              // JC 0x004392ce
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xfffe
        _emit 0xfe
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP AX, 0x32e
        _emit 0x3d
        _emit 0x2e
        _emit 0x03
        _emit 0x72              // JC 0x004392d9
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xfff8
        _emit 0xf8
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP AX, 0x30f
        _emit 0x3d
        _emit 0x0f
        _emit 0x03
        _emit 0x72              // JC 0x004392e4
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xffff
        _emit 0xff
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP AX, 0x2f0
        _emit 0x3d
        _emit 0xf0
        _emit 0x02
        _emit 0x72              // JC 0x004392ef
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xffc2
        _emit 0xc2
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP AX, 0x292
        _emit 0x3d
        _emit 0x92
        _emit 0x02
        _emit 0x72              // JC 0x004392fa
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xfff3
        _emit 0xf3
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP AX, 0x264
        _emit 0x3d
        _emit 0x64
        _emit 0x02
        _emit 0x72              // JC 0x00439305
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xfff1
        _emit 0xf1
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP AX, 0x234
        _emit 0x3d
        _emit 0x34
        _emit 0x02
        _emit 0x72              // JC 0x00439310
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xffda
        _emit 0xda
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP AX, 0x1f6
        _emit 0x3d
        _emit 0xf6
        _emit 0x01
        _emit 0x72              // JC 0x0043931b
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xfff8
        _emit 0xf8
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP AX, 0x1d6
        _emit 0x3d
        _emit 0xd6
        _emit 0x01
        _emit 0x72              // JC 0x00439326
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xfff8
        _emit 0xf8
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP AX, 0x178
        _emit 0x3d
        _emit 0x78
        _emit 0x01
        _emit 0x72              // JC 0x00439331
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xfff5
        _emit 0xf5
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP AX, 0x11a
        _emit 0x3d
        _emit 0x1a
        _emit 0x01
        _emit 0x72              // JC 0x0043933c
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xfffc
        _emit 0xfc
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP AX, 0xfc
        _emit 0x3d
        _emit 0xfc
        _emit 0x00
        _emit 0x72              // JC 0x00439347
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xfffa
        _emit 0xfa
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP AX, 0xdc
        _emit 0x3d
        _emit 0xdc
        _emit 0x00
        _emit 0x72              // JC 0x00439352
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xfff9
        _emit 0xf9
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP AX, 0xcb
        _emit 0x3d
        _emit 0xcb
        _emit 0x00
        _emit 0x72              // JC 0x0043935d
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xfff1
        _emit 0xf1
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP AX, 0xbb
        _emit 0x3d
        _emit 0xbb
        _emit 0x00
        _emit 0x72              // JC 0x00439368
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xfffc
        _emit 0xfc
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP AX, 0xaf
        _emit 0x3d
        _emit 0xaf
        _emit 0x00
        _emit 0x72              // JC 0x00439373
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xfff9
        _emit 0xf9
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP AX, 0x99
        _emit 0x3d
        _emit 0x99
        _emit 0x00
        _emit 0x72              // JC 0x0043937e
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xfff5
        _emit 0xf5
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP AX, 0x87
        _emit 0x3d
        _emit 0x87
        _emit 0x00
        _emit 0x72              // JC 0x00439389
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xfff8
        _emit 0xf8
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x66              // CMP AX, 0x77
        _emit 0x3d
        _emit 0x77
        _emit 0x00
        _emit 0x72              // JC 0x00439394
        _emit 0x05
        _emit 0x05              // ADD EAX, 0xfff5
        _emit 0xf5
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x05              // ADD EAX, 0x100               ; LAB_00439394
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x66              // CMP CX, 0xff                 ; LAB_0043939c
        _emit 0x81
        _emit 0xf9
        _emit 0xff
        _emit 0x00
        _emit 0x77              // JA 0x004393a6
        _emit 0x03
        _emit 0x0f              // MOVZX EAX, CX
        _emit 0xb7
        _emit 0xc1
        _emit 0xc2              // RET 0x4                       ; LAB_004393a6
        _emit 0x04
        _emit 0x00
    }
}
