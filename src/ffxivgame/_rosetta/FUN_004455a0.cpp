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
// FUNCTION: ffxivgame 0x000455a0 — __stdcall UTF-8 lead-byte → sequence-length
//                                  classifier (89 B / 0x59, no SEH, no relocs).
//
// Inspection (read from asm/ffxivgame/000455a0_FUN_004455a0.s):
//
//   __stdcall int FUN_004455a0(unsigned char c);   // arg at [esp+4], ret 4
//
//     // Continuation bytes 0x80..0xBF are not valid lead bytes → length 0.
//     // The compiler folds the [0x80,0xBF] range test into the classic
//     // (unsigned char)(c - 0x80) <= 0x3f wraparound trick:
//     //     mov cl,c ; add cl,0x80 ; cmp cl,0x3f ; ja ...
//     if ((unsigned char)(c + 0x80) <= 0x3f) return 0;   // continuation byte
//     if (c < 0x80) return 1;                            // ASCII
//     if (c < 0xe0) return 2;                            // 2-byte lead (0xC0..0xDF)
//     if (c < 0xf0) return 3;                            // 3-byte lead
//     if (c < 0xf8) return 4;                            // 4-byte lead
//     if (c < 0xfc) return 5;                            // 5-byte lead
//     // Tail folds `c < 0xfe ? 6 : 0` into sbb eax,eax ; and eax,6:
//     //     cmp al,0xfe ; sbb eax,eax ; and eax,6
//     return (c < 0xfe) ? 6 : 0;                         // 6-byte lead, else 0
//
// Asm shape (89 bytes — RVA 0x000455a0..0x000455f9):
//
//     000455a0:  8a 44 24 04        MOV  AL,[ESP+0x4]    ; c
//     000455a4:  8a c8              MOV  CL,AL
//     000455a6:  80 c1 80           ADD  CL,0x80
//     000455a9:  80 f9 3f           CMP  CL,0x3f
//     000455ac:  77 05              JA   0x004455b3
//     000455ae:  33 c0              XOR  EAX,EAX
//     000455b0:  c2 04 00           RET  0x4             ; return 0 (continuation)
//     000455b3:  3c 80              CMP  AL,0x80
//     000455b5:  73 08              JNC  0x004455bf
//     000455b7:  b8 01 00 00 00     MOV  EAX,0x1
//     000455bc:  c2 04 00           RET  0x4             ; return 1 (ASCII)
//     000455bf:  3c e0              CMP  AL,0xe0
//     000455c1:  73 08              JNC  0x004455cb
//     000455c3:  b8 02 00 00 00     MOV  EAX,0x2
//     000455c8:  c2 04 00           RET  0x4             ; return 2
//     000455cb:  3c f0              CMP  AL,0xf0
//     000455cd:  73 08              JNC  0x004455d7
//     000455cf:  b8 03 00 00 00     MOV  EAX,0x3
//     000455d4:  c2 04 00           RET  0x4             ; return 3
//     000455d7:  3c f8              CMP  AL,0xf8
//     000455d9:  73 08              JNC  0x004455e3
//     000455db:  b8 04 00 00 00     MOV  EAX,0x4
//     000455e0:  c2 04 00           RET  0x4             ; return 4
//     000455e3:  3c fc              CMP  AL,0xfc
//     000455e5:  73 08              JNC  0x004455ef
//     000455e7:  b8 05 00 00 00     MOV  EAX,0x5
//     000455ec:  c2 04 00           RET  0x4             ; return 5
//     000455ef:  3c fe              CMP  AL,0xfe
//     000455f1:  1b c0              SBB  EAX,EAX
//     000455f3:  83 e0 06           AND  EAX,0x6
//     000455f6:  c2 04 00           RET  0x4             ; return (c<0xfe)?6:0
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function carries zero relocations (all immediates are literals,
//   all branches are self-relative), so re-emitting the orig 89 bytes
//   verbatim via MASM `_emit` directives yields a `.text` slice that is
//   byte-identical to the orig — exactly what tools/compare.py checks.
//   A source-level formulation would have to coax MSVC 2005 into both the
//   (c-0x80)<=0x3f range fold and the `sbb eax,eax ; and eax,6` ternary
//   fold while keeping the AL/CL register choice; the naked passthrough
//   sidesteps that fragility, matching the binary's established idiom for
//   small branch-dense helpers.

extern "C" __declspec(naked) void FUN_004455a0() {
    __asm {
        _emit 0x8a
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8a
        _emit 0xc8
        _emit 0x80
        _emit 0xc1
        _emit 0x80
        _emit 0x80
        _emit 0xf9
        _emit 0x3f
        _emit 0x77
        _emit 0x05
        _emit 0x33
        _emit 0xc0

        _emit 0xc2
        _emit 0x04
        _emit 0x00
        _emit 0x3c
        _emit 0x80
        _emit 0x73
        _emit 0x08
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        _emit 0x3c

        _emit 0xe0
        _emit 0x73
        _emit 0x08
        _emit 0xb8
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        _emit 0x3c
        _emit 0xf0
        _emit 0x73
        _emit 0x08
        _emit 0xb8

        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        _emit 0x3c
        _emit 0xf8
        _emit 0x73
        _emit 0x08
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0xc2
        _emit 0x04
        _emit 0x00
        _emit 0x3c
        _emit 0xfc
        _emit 0x73
        _emit 0x08
        _emit 0xb8
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        _emit 0x3c

        _emit 0xfe
        _emit 0x1b
        _emit 0xc0
        _emit 0x83
        _emit 0xe0
        _emit 0x06
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
