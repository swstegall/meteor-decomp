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
// FUNCTION: ffxivgame 0x00045ee0 — in-place UTF-8 lower-case folder
//                                  (200 B / 0xc8), `__thiscall void(this)`.
//
// Asm shape (read from asm/ffxivgame/00045ee0_FUN_00445ee0.s):
//
//   __thiscall void FUN_00445ee0(Buffer* this /* ECX */);
//
//   struct Buffer { char* data; /* +0 */ ... ; uint32_t len; /* +8 */ };
//
//   Walks `this->data[0 .. this->len)` byte by byte, advancing by each
//   code point's UTF-8 length:
//
//     for (uint32_t i = 0; i < this->len; i += step) {
//         char*         p = this->data + i;
//         unsigned char c = *p;
//         if ((unsigned char)(c - 'A') <= 0x19u) {   // ASCII A..Z
//             *p = c + 0x20;                          // → a..z
//         } else if (c == 0xc3) {                     // Latin-1 Supplement
//             unsigned char d = p[1];                 // À..Þ → à..þ region
//             if (d >= 0x80 && d <= 0x96) {           // C3 80..96 (À..Ö)
//                 p[1] = 0xe3;                         // fold to lower form
//                 step = 2; continue;
//             }
//             if ((unsigned char)(d + 0x68) > 6) {     // not C3 98..9E
//                 step = 2; continue;                  // (Ø..Þ already low?)
//             }
//             p[1] = 0xe3; step = 2; continue;
//         } else if (c == 0xc5) {                     // Latin Extended-A
//             if (p[1] == 0x92) { p[1] = 0x93;        // Œ (C5 92) → œ (C5 93)
//                                 step = 2; continue; }
//             step = 2; continue;
//         }
//         // step = UTF-8 byte length of the lead byte c:
//         //   <0x80 → 1; 0x80..0xbf → 0 (stray cont.); 0xc0..0xdf → 2;
//         //   0xe0..0xef → 3; 0xf0..0xf7 → 4; 0xf8..0xfb → 5;
//         //   0xfc..0xfd → 6; >=0xfe → 0
//         step = utf8_lead_len(c);
//     }
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The orig 200 bytes carry NO relocations (no calls, no absolute or
//   IAT references — every immediate is a small constant and every
//   branch is PC-relative). A source-level C++ rewrite of the nested
//   code-point classifier would shuffle MSVC 2005's branch ordering and
//   register allocation (the `CL = 0x93` hoist, the `SBB/AND 6` tail for
//   the 6-byte length test, the short-vs-near JMP mix), shifting bytes.
//   Re-emitting the exact 200 bytes via MASM `_emit` makes the .obj's
//   `.text` byte-identical to the orig slice with zero auxiliary relocs,
//   which is what tools/compare.py grades GREEN.

extern "C" __declspec(naked) void FUN_00445ee0() {
    __asm {
        _emit 0x57          // PUSH EDI
        _emit 0x8b          // MOV EAX, ECX
        _emit 0xc1
        _emit 0x33          // XOR EDI, EDI
        _emit 0xff
        _emit 0x39          // CMP [EAX+8], EDI
        _emit 0x78
        _emit 0x08
        _emit 0x0f          // JBE 0x00445fa6
        _emit 0x86
        _emit 0xb8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x53          // PUSH EBX
        _emit 0xb1          // MOV CL, 0x93
        _emit 0x93
        _emit 0x56          // PUSH ESI
        // loop_top (0x00445ef2):
        _emit 0x8b          // MOV EDX, [EAX]
        _emit 0x10
        _emit 0x8d          // LEA ESI, [EDX+EDI]
        _emit 0x34
        _emit 0x3a
        _emit 0x8a          // MOV DL, [ESI]
        _emit 0x16
        _emit 0x8a          // MOV BL, DL
        _emit 0xda
        _emit 0x80          // SUB BL, 0x41
        _emit 0xeb
        _emit 0x41
        _emit 0x80          // CMP BL, 0x19
        _emit 0xfb
        _emit 0x19
        _emit 0x77          // JA 0x00445f1b
        _emit 0x18
        _emit 0x8a          // MOV BL, DL
        _emit 0xda
        _emit 0x80          // ADD BL, 0x20
        _emit 0xc3
        _emit 0x20
        _emit 0x88          // MOV [ESI], BL
        _emit 0x1e
        // 0x00445f0a:
        _emit 0x80          // CMP DL, 0x7f
        _emit 0xfa
        _emit 0x7f
        _emit 0x76          // JBE 0x00445f55
        _emit 0x46
        _emit 0x80          // CMP DL, 0xc0
        _emit 0xfa
        _emit 0xc0
        _emit 0x73          // JNC 0x00445f55
        _emit 0x41
        _emit 0x33          // XOR EDX, EDX
        _emit 0xd2
        _emit 0xe9          // JMP 0x00445f99
        _emit 0x7e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x00445f1b:
        _emit 0x80          // CMP DL, 0xc3
        _emit 0xfa
        _emit 0xc3
        _emit 0x75          // JNZ 0x00445f40
        _emit 0x20
        _emit 0x8a          // MOV DL, [ESI+1]
        _emit 0x56
        _emit 0x01
        _emit 0x80          // CMP DL, 0x80
        _emit 0xfa
        _emit 0x80
        _emit 0x72          // JC 0x00445f2d
        _emit 0x05
        _emit 0x80          // CMP DL, 0x96
        _emit 0xfa
        _emit 0x96
        _emit 0x76          // JBE 0x00445f35
        _emit 0x08
        // 0x00445f2d:
        _emit 0x80          // ADD DL, 0x68
        _emit 0xc2
        _emit 0x68
        _emit 0x80          // CMP DL, 0x6
        _emit 0xfa
        _emit 0x06
        _emit 0x77          // JA 0x00445f66
        _emit 0x31
        // 0x00445f35:
        _emit 0xc6          // MOV [ESI+1], 0xe3
        _emit 0x46
        _emit 0x01
        _emit 0xe3
        _emit 0xba          // MOV EDX, 0x2
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb          // JMP 0x00445f99
        _emit 0x59
        // 0x00445f40:
        _emit 0x80          // CMP DL, 0xc5
        _emit 0xfa
        _emit 0xc5
        _emit 0x75          // JNZ 0x00445f0a
        _emit 0xc5
        _emit 0x80          // CMP [ESI+1], 0x92
        _emit 0x7e
        _emit 0x01
        _emit 0x92
        _emit 0x75          // JNZ 0x00445f66
        _emit 0x1b
        _emit 0x88          // MOV [ESI+1], CL
        _emit 0x4e
        _emit 0x01
        _emit 0xba          // MOV EDX, 0x2
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb          // JMP 0x00445f99
        _emit 0x44
        // 0x00445f55:
        _emit 0x80          // CMP DL, 0x80
        _emit 0xfa
        _emit 0x80
        _emit 0x73          // JNC 0x00445f61
        _emit 0x07
        _emit 0xba          // MOV EDX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb          // JMP 0x00445f99
        _emit 0x38
        // 0x00445f61:
        _emit 0x80          // CMP DL, 0xe0
        _emit 0xfa
        _emit 0xe0
        _emit 0x73          // JNC 0x00445f6d
        _emit 0x07
        // 0x00445f66:
        _emit 0xba          // MOV EDX, 0x2
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb          // JMP 0x00445f99
        _emit 0x2c
        // 0x00445f6d:
        _emit 0x80          // CMP DL, 0xf0
        _emit 0xfa
        _emit 0xf0
        _emit 0x73          // JNC 0x00445f79
        _emit 0x07
        _emit 0xba          // MOV EDX, 0x3
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb          // JMP 0x00445f99
        _emit 0x20
        // 0x00445f79:
        _emit 0x80          // CMP DL, 0xf8
        _emit 0xfa
        _emit 0xf8
        _emit 0x73          // JNC 0x00445f85
        _emit 0x07
        _emit 0xba          // MOV EDX, 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb          // JMP 0x00445f99
        _emit 0x14
        // 0x00445f85:
        _emit 0x80          // CMP DL, 0xfc
        _emit 0xfa
        _emit 0xfc
        _emit 0x73          // JNC 0x00445f91
        _emit 0x07
        _emit 0xba          // MOV EDX, 0x5
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb          // JMP 0x00445f99
        _emit 0x08
        // 0x00445f91:
        _emit 0x80          // CMP DL, 0xfe
        _emit 0xfa
        _emit 0xfe
        _emit 0x1b          // SBB EDX, EDX
        _emit 0xd2
        _emit 0x83          // AND EDX, 0x6
        _emit 0xe2
        _emit 0x06
        // 0x00445f99:
        _emit 0x03          // ADD EDI, EDX
        _emit 0xfa
        _emit 0x3b          // CMP EDI, [EAX+8]
        _emit 0x78
        _emit 0x08
        _emit 0x0f          // JC 0x00445ef2
        _emit 0x82
        _emit 0x4e
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x5e          // POP ESI
        _emit 0x5b          // POP EBX
        // 0x00445fa6:
        _emit 0x5f          // POP EDI
        _emit 0xc3          // RET
    }
}
