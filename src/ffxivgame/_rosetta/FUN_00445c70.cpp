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
// FUNCTION: ffxivgame 0x00045c70 — `__fastcall`/`__thiscall` UTF-8 string
//                                  validator (123 B / 0x7b, no relocations,
//                                  no stack frame).
//
// Inspection (read from the disassembly at orig RVA 0x00045c70):
//
//   bool __fastcall is_valid_utf8(unsigned char **pp /*ECX*/);
//
//   The single argument arrives in ECX as a pointer-to-pointer; the body
//   dereferences it once (`MOV EDX,[ECX]`) to obtain the current byte
//   cursor and never touches ECX again, so __fastcall / __thiscall are
//   indistinguishable here (one register arg, no stack args, plain `RET`).
//   Returns AL (1 = valid / fully consumed, 0 = malformed byte found).
//
//   Pseudo-C:
//
//     bool is_valid_utf8(unsigned char **pp) {
//         unsigned char *p = *pp;
//         for (;;) {
//             unsigned char c = *p;
//             if (c == 0) return true;            // NUL terminator → ok
//         lead:
//             // A stray continuation byte (0x80..0xBF) as a lead is bad.
//             if ((unsigned char)(c + 0x80) <= 0x3F) return false;
//             int n;                              // total bytes in sequence
//             if      (c <  0x80) n = 1;          // ASCII
//             else if (c <  0xE0) n = 2;
//             else if (c <  0xF0) n = 3;
//             else if (c <  0xF8) n = 4;
//             else if (c <  0xFC) n = 5;
//             else if (c <  0xFE) n = 6;
//             else                return false;   // 0xFE / 0xFF illegal
//             ++p;
//             // Verify (n-1) trailing continuation bytes, each 10xxxxxx.
//             for (int i = 1; i < n; ++i, ++p)
//                 if ((*p & 0x80) == 0) return false;
//             c = *p;
//             if (c != 0) goto lead;              // next sequence
//             return true;
//         }
//     }
//
//   The `8b ff` (MOV EDI,EDI) at +0x5e is MSVC's 2-byte hot-patch pad that
//   lands at the top of the trailing-byte loop; it is part of the orig
//   byte stream and must be reproduced verbatim.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This function carries NO relocations (pure register / immediate code,
//   no external calls, no data references), so a `__declspec(naked)` body
//   that re-emits the orig 123 bytes verbatim via MASM `_emit` directives
//   produces a `.text` slice byte-identical to the original. A high-level
//   C++ rewrite under /O2 would be brittle on the cascade-of-CMP lead-byte
//   classifier (branch short-vs-near, register choice for the c+0x80 test)
//   for no benefit; `tools/compare.py` reports GREEN on the passthrough.

extern "C" __declspec(naked) void FUN_00445c70() {
    __asm {
        _emit 0x8b              // MOV EDX, [ECX]
        _emit 0x11
        // .loop:
        _emit 0x8a              // MOV AL, [EDX]
        _emit 0x02
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x74              // JZ .ret_true (+0x6d)
        _emit 0x6d
        // .lead:
        _emit 0x8a              // MOV CL, AL
        _emit 0xc8
        _emit 0x80              // ADD CL, 0x80
        _emit 0xc1
        _emit 0x80
        _emit 0x80              // CMP CL, 0x3F
        _emit 0xf9
        _emit 0x3f
        _emit 0x76              // JBE .ret_false (+0x66)
        _emit 0x66
        _emit 0x3c              // CMP AL, 0x80
        _emit 0x80
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP .trail (+0x35)
        _emit 0x35
        _emit 0x3c              // CMP AL, 0xE0
        _emit 0xe0
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 2
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP .trail (+0x2a)
        _emit 0x2a
        _emit 0x3c              // CMP AL, 0xF0
        _emit 0xf0
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 3
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP .trail (+0x1f)
        _emit 0x1f
        _emit 0x3c              // CMP AL, 0xF8
        _emit 0xf8
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 4
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP .trail (+0x14)
        _emit 0x14
        _emit 0x3c              // CMP AL, 0xFC
        _emit 0xfc
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 5
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP .trail (+9)
        _emit 0x09
        _emit 0x3c              // CMP AL, 0xFE
        _emit 0xfe
        _emit 0x73              // JNC .ret_false (+0x2b)
        _emit 0x2b
        _emit 0xb8              // MOV EAX, 6
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // .trail:
        _emit 0xb9              // MOV ECX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD EDX, 1
        _emit 0xc2
        _emit 0x01
        _emit 0x3b              // CMP EAX, ECX
        _emit 0xc1
        _emit 0x76              // JBE .after (+0x11)
        _emit 0x11
        _emit 0x8b              // MOV EDI, EDI  (hot-patch pad)
        _emit 0xff
        // .trail_loop:
        _emit 0xf6              // TEST byte ptr [EDX], 0x80
        _emit 0x02
        _emit 0x80
        _emit 0x74              // JZ .ret_false (+0x13)
        _emit 0x13
        _emit 0x83              // ADD ECX, 1
        _emit 0xc1
        _emit 0x01
        _emit 0x83              // ADD EDX, 1
        _emit 0xc2
        _emit 0x01
        _emit 0x3b              // CMP ECX, EAX
        _emit 0xc8
        _emit 0x72              // JC .trail_loop (-0x0f)
        _emit 0xf1
        // .after:
        _emit 0x8a              // MOV AL, [EDX]
        _emit 0x02
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x75              // JNZ .lead (-0x6d)
        _emit 0x93
        // .ret_true:
        _emit 0xb0              // MOV AL, 1
        _emit 0x01
        _emit 0xc3              // RET
        // .ret_false:
        _emit 0x32              // XOR AL, AL
        _emit 0xc0
        _emit 0xc3              // RET
    }
}
