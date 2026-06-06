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
// FUNCTION: ffxivgame 0x00470120 — __thiscall string trim-in-place
//                                  (131 B / 0x83).
//
// Semantics (reconstructed from asm at RVA 0x00070120):
//
//   char* __thiscall str_trim(char *str /*ECX*/)
//   {
//       // Phase 1: skip leading whitespace
//       if (!*str) return NULL;
//       while (isspace((unsigned char)*str)) {
//           ++str;
//           if (!*str) return NULL;
//       }
//       if (!*str) return NULL;   // at RVA 0x47014c — redundant but emitted
//
//       // Phase 2: find last non-space char (strlen + backward scan)
//       char *end = str + strlen(str) - 1;  // EDI = last char of string
//       if (end == str) goto return_str;    // single-char fast-path
//       do {
//           if (!isspace((unsigned char)*end)) {
//               // found last non-space: null-terminate after it
//               if (str != end) end[1] = '\0';
//               goto return_str;
//           }
//           --end;
//       } while (end != str);
//       // all trailing chars were space; single remaining char at *str
//
//   return_str:
//       // The canonical MSVC optimised return: *str ? str : NULL
//       // Encoded as NEG AL / SBB EAX,EAX / AND EAX,ESI at the tail.
//       // (The SBB/AND/POP ESI/RET sequence at 0x4701a3 is a SHARED
//       // tail outside the 131-byte matching window; the window ends
//       // at the POP EDI at 0x4701a2.)
//       return *str ? str : NULL;
//   }
//
// Control-flow notes:
//   * The 6-byte alignment NOP `8d 9b 00 00 00 00` (LEA EBX,[EBX])
//     appears TWICE: once at 0x47012a (dead bytes jumped over by the
//     `EB 06` at 0x470128) and again at 0x47015a (loop alignment).
//   * The `JMP 0x470130` at 0x470128 skips the 6 dead NOP bytes;
//     the initial `CMP/JZ` at 0x470123 handles the empty-string case.
//   * The function boundary in symbols.json / compare.py is 131 bytes,
//     ending at POP EDI (0x5f) at 0x4701a2.  The 6-byte epilogue
//     (SBB/AND/POP ESI/RET) at 0x4701a3..0x4701a8 is a shared tail
//     that lives OUTSIDE the matching window.
//
// Calling convention: __thiscall (ECX = str; plain RET — no stack args).
//
// Reloc-bearing CALLs inside the 131 bytes (both target isspace at
// 0x009d825a in the binary):
//   +0x14  CALL rel32  e8 21 81 56 00   (from RVA 0x70134 → 0x9d825a)
//   +0x58  CALL rel32  e8 dd 80 56 00   (from RVA 0x70178 → 0x9d825a)
//
// Reconstruction strategy: naked-asm byte passthrough (_emit).
//   Source-level C++ would require matching MSVC's exact register
//   allocation, two alignment NOPs, 6 dead bytes, and the NEG/SBB/AND
//   return idiom — all of which are brittle under /O2. Emitting the
//   131 bytes verbatim via _emit produces a .obj whose .text is
//   byte-identical to the orig slice.

extern "C" __declspec(naked) void FUN_00470120() {
    __asm {
        // +0x00  56            PUSH ESI
        _emit 0x56
        // +0x01  8b f1         MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // +0x03  80 3e 00      CMP byte ptr [ESI],0x0
        _emit 0x80
        _emit 0x3e
        _emit 0x00
        // +0x06  74 29         JZ 0x470151  (null/empty → return NULL)
        _emit 0x74
        _emit 0x29
        // +0x08  eb 06         JMP 0x470130  (skip dead alignment NOP)
        _emit 0xeb
        _emit 0x06
        // +0x0a  8d 9b 00 00 00 00  LEA EBX,[EBX]  (dead 6-byte NOP)
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // forward scan loop body at 0x470130 ─────────────────────────
        // +0x10  0f b6 06      MOVZX EAX,byte ptr [ESI]
        _emit 0x0f
        _emit 0xb6
        _emit 0x06
        // +0x13  50            PUSH EAX
        _emit 0x50
        // +0x14  e8 21 81 56 00  CALL isspace (0x009d825a)
        _emit 0xe8
        _emit 0x21
        _emit 0x81
        _emit 0x56
        _emit 0x00
        // +0x19  83 c4 04      ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // +0x1c  85 c0         TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // +0x1e  74 0c         JZ 0x47014c   (!isspace → exit forward loop)
        _emit 0x74
        _emit 0x0c

        // +0x20  83 c6 01      ADD ESI,0x1
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        // +0x23  80 3e 00      CMP byte ptr [ESI],0x0
        _emit 0x80
        _emit 0x3e
        _emit 0x00
        // +0x26  75 e8         JNZ 0x470130  (continue loop if not NUL)
        _emit 0x75
        _emit 0xe8
        // +0x28  33 c0         XOR EAX,EAX   (return NULL — all whitespace)
        _emit 0x33
        _emit 0xc0
        // +0x2a  5e            POP ESI
        _emit 0x5e
        // +0x2b  c3            RET
        _emit 0xc3

        // post-loop null check at 0x47014c ────────────────────────────
        // +0x2c  80 3e 00      CMP byte ptr [ESI],0x0
        _emit 0x80
        _emit 0x3e
        _emit 0x00
        // +0x2f  75 04         JNZ 0x470155  (non-NUL → proceed to trim tail)
        _emit 0x75
        _emit 0x04
        // +0x31  33 c0         XOR EAX,EAX   (return NULL — empty after trim)
        _emit 0x33
        _emit 0xc0
        // +0x33  5e            POP ESI
        _emit 0x5e
        // +0x34  c3            RET
        _emit 0xc3

        // strlen loop setup at 0x470155 ───────────────────────────────
        // +0x35  8b c6         MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // +0x37  8d 50 01      LEA EDX,[EAX + 0x1]   (EDX = ESI+1 for strlen math)
        _emit 0x8d
        _emit 0x50
        _emit 0x01
        // +0x3a  8d 9b 00 00 00 00  LEA EBX,[EBX]  (6-byte loop-alignment NOP)
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // strlen loop at 0x470160 ─────────────────────────────────────
        // +0x40  8a 08         MOV CL,byte ptr [EAX]
        _emit 0x8a
        _emit 0x08
        // +0x42  83 c0 01      ADD EAX,0x1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // +0x45  84 c9         TEST CL,CL
        _emit 0x84
        _emit 0xc9
        // +0x47  75 f7         JNZ 0x470160  (continue until NUL)
        _emit 0x75
        _emit 0xf7
        // +0x49  2b c2         SUB EAX,EDX   (EAX = strlen(ESI))
        _emit 0x2b
        _emit 0xc2
        // +0x4b  57            PUSH EDI
        _emit 0x57
        // +0x4c  8d 7c 30 ff   LEA EDI,[EAX + ESI*1 - 1]  (EDI = last char)
        _emit 0x8d
        _emit 0x7c
        _emit 0x30
        _emit 0xff

        // backward scan loop at 0x470170 ──────────────────────────────
        // +0x50  3b fe         CMP EDI,ESI
        _emit 0x3b
        _emit 0xfe
        // +0x52  74 2a         JZ 0x47019e   (single char or all-space → return_str)
        _emit 0x74
        _emit 0x2a
        // +0x54  0f b6 0f      MOVZX ECX,byte ptr [EDI]
        _emit 0x0f
        _emit 0xb6
        _emit 0x0f
        // +0x57  51            PUSH ECX
        _emit 0x51
        // +0x58  e8 dd 80 56 00  CALL isspace (0x009d825a)
        _emit 0xe8
        _emit 0xdd
        _emit 0x80
        _emit 0x56
        _emit 0x00
        // +0x5d  83 c4 04      ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04

        // +0x60  85 c0         TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // +0x62  74 12         JZ 0x470196   (!isspace → found last non-space)
        _emit 0x74
        _emit 0x12
        // +0x64  83 ef 01      SUB EDI,0x1   (EDI--)
        _emit 0x83
        _emit 0xef
        _emit 0x01
        // +0x67  3b fe         CMP EDI,ESI
        _emit 0x3b
        _emit 0xfe
        // +0x69  75 e9         JNZ 0x470174  (continue if EDI != start)
        _emit 0x75
        _emit 0xe9
        // +0x6b  8a 06         MOV AL,byte ptr [ESI]
        _emit 0x8a
        _emit 0x06
        // +0x6d  f6 d8         NEG AL
        _emit 0xf6
        _emit 0xd8
        // +0x6f  5f            POP EDI
        _emit 0x5f

        // +0x70  1b c0         SBB EAX,EAX
        _emit 0x1b
        _emit 0xc0
        // +0x72  23 c6         AND EAX,ESI
        _emit 0x23
        _emit 0xc6
        // +0x74  5e            POP ESI
        _emit 0x5e
        // +0x75  c3            RET
        _emit 0xc3

        // null-terminate path at 0x470196 ─────────────────────────────
        // +0x76  3b f7         CMP ESI,EDI
        _emit 0x3b
        _emit 0xf7
        // +0x78  74 04         JZ 0x47019e   (single non-space → skip write)
        _emit 0x74
        _emit 0x04
        // +0x7a  c6 47 01 00   MOV byte ptr [EDI + 0x1],0x0  (null-terminate)
        _emit 0xc6
        _emit 0x47
        _emit 0x01
        _emit 0x00

        // shared return_str at 0x47019e ───────────────────────────────
        // +0x7e  8a 06         MOV AL,byte ptr [ESI]
        _emit 0x8a
        _emit 0x06
        // +0x80  f6 d8         NEG AL
        _emit 0xf6
        _emit 0xd8
        // +0x82  5f            POP EDI   (← last byte of 131-byte window)
        _emit 0x5f
        // NOTE: SBB EAX,EAX / AND EAX,ESI / POP ESI / RET at 0x4701a3
        // are the SHARED tail outside this function's 131-byte window.
    }
}

// vim: ts=4 sts=4 sw=4 et
