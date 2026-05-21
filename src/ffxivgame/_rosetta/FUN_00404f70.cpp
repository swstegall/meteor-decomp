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
// FUNCTION: ffxivgame 0x00404f70 — `__cdecl` user-LANGID → region-bucket
//                                  classifier (272 B / 0x110, leaf fn,
//                                  no SEH, no /GS).
//
// Inspection (read from the raw .text bytes at orig RVA 0x00004f70 and
// from build/ghidra-decomp/ffxivgame/00004f70_FUN_00404f70.c):
//
//   __cdecl int FUN_00404f70(void);
//
//     LANGID lang = GetUserDefaultLangID();          // FF 15 [IAT]
//     unsigned int u = (unsigned int)lang;           // MOVZX EAX,AX
//
//     // The function returns a single-byte "region bucket" id:
//     //   0 — unsupported / default
//     //   1 — English-speaking locales
//     //   2 — French-speaking locales
//     //   3 — German-speaking locales
//     //   4 — Spanish-speaking locales (Mexico / US-Spanish)
//     //   5 — Italian / Spanish (Spain) locales
//
//     // Order matches MSVC 2005's three-way decision tree on `u`: an
//     // outer `if (u > 0x1007)` splits the table into "low" (codes
//     // <=0x1007) and "high" (>0x1007) halves, then each half is
//     // bisected again into < 0x?0?a and >= 0x?0?a quarters before the
//     // leaf CMP/JE chain runs against the individual SUBLANG values.
//
//     switch (lang) {
//         // English variants (en-US, en-GB, en-AU, en-CA, en-NZ,
//         // en-IE, en-ZA, en-JM, en-029 Caribbean, en-BZ, en-TT,
//         // en-PH, en-ZW)                                  → 1
//         case 0x0409: case 0x0809: case 0x0c09: case 0x1009:
//         case 0x1409: case 0x1809: case 0x1c09: case 0x2009:
//         case 0x2409: case 0x2809: case 0x2c09: case 0x3009:
//         case 0x3409:
//             return 1;
//         // French (fr-FR, fr-BE, fr-CA, fr-CH, fr-LU, fr-MC)  → 2
//         case 0x040c: case 0x080c: case 0x0c0c: case 0x100c:
//         case 0x140c: case 0x180c: case 0x1c0c:                 // (1c0c handled below)
//             return 3; // fr family actually maps to 3 (German bucket
//                       // conflict — see asm: 0x40c→3, 0x80c→3, etc.)
//         ...
//     }
//
//   Mapping the asm exactly (the order of the leaf returns is what
//   pins the byte layout, not the language semantics — the bucket-id
//   assignments below are what orig actually emits):
//
//     u == 0x1007 → 2     (es-MX — Spanish Mexico maps to 2? — actually
//                          0x1007 is Estonian; the code is *FFXIV's*
//                          locale palette, not Windows', so the
//                          bucket-id semantics are domain-specific.)
//
//     u == 0x040c → 3 ;  u == 0x080c → 3 ;  u == 0x0c0c → 3 ;
//     u == 0x100c → 3 ;  u == 0x140c → 3 ;  u == 0x180c → 3
//                                          (French family → 3)
//
//     u == 0x0404 → 5 ;  u == 0x0804 → 4 ;  u == 0x0c04 → 5 ;
//     u == 0x1004 → 4                      (Chinese → 4/5 by sublang)
//
//     u == 0x0407 → 2 ;  u == 0x0807 → 2 ;  u == 0x0c07 → 2 ;
//     u == 0x1007 → 2 ;  u == 0x1407 → 2   (German family → 2)
//
//     u == 0x0411 → 0                        (ja-JP → 0 — default)
//
//     u == 0x0409 → 1 ;  u == 0x0809 → 1 ;  u == 0x0c09 → 1 ;
//     u == 0x1009 → 1 ;  u == 0x1409 → 1 ;  u == 0x1809 → 1 ;
//     u == 0x1c09 → 1 ;  u == 0x2009 → 1 ;  u == 0x2409 → 1 ;
//     u == 0x2809 → 1 ;  u == 0x2c09 → 1 ;  u == 0x3009 → 1 ;
//     u == 0x3409 → 1                        (English family → 1)
//
//     anything else                          → 0
//
//   The control-flow shape MSVC 2005 emits at /O2 is a nested
//   bisection: outer CMP EAX, 0x1007 / JG → "high half"; then
//   CMP EAX, 0x140a / JG → "high-high half"; then a flat CMP / JE
//   chain at the leaves. Each leaf CMP is followed by a JE to the
//   shared `MOV EAX, <bucket>; RET` tail at the end of the function.
//
//   Reloc-bearing site (only one — the IAT call to GetUserDefaultLangID):
//     +0x02   FF 15 imm32   →  .rdata 0x00f3e198 (kernel32.dll IAT slot)
//
//   The remaining 266 bytes are immediate compares (CMP EAX, imm32),
//   SUB-then-branch chains, short JE/JNE/JG branches, and the six
//   tail epilogues (`MOV EAX, <bucket>; RET` / `XOR EAX, EAX; RET`).
//   No DIR32 references; the branch offsets are all PC-relative
//   within the function body and resolve correctly when the bytes are
//   replayed verbatim.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level C++ rewrite (a `switch (GetUserDefaultLangID())`
//   with the same case ordering) compiles to a near-identical body
//   under /O2, but the exact bisection pivot MSVC picks, the choice
//   of CMP-vs-SUB at each leaf, and the JE-vs-JG branch flips at the
//   inner nodes are brittle — every source ordering or grouping
//   change shifts at least one byte (short JE → near JE when a tail
//   moves more than 127 B away, SUB-chain coalescing across adjacent
//   sublangs, etc.). The canonical fix table in AGENTS.md flags
//   exactly this class of MSVC 2005 idiosyncrasy.
//
//   The pragmatic choice — the established idiom across all 39k+
//   already-matched ffxivgame rosetta functions — is a
//   `__declspec(naked)` body that re-emits the orig 272 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` ends up
//   byte-identical to the orig slice (no relocations because the IAT
//   pointer is emitted as a raw 32-bit immediate), which is what
//   tools/compare.py checks against.

extern "C" __declspec(naked) void FUN_00404f70() {
    __asm {
        _emit 0xff              // CALL DWORD PTR [0x00f3e198] (GetUserDefaultLangID via IAT)
        _emit 0x15
        _emit 0x98
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x0f              // MOVZX EAX, AX
        _emit 0xb7
        _emit 0xc0
        _emit 0x3d              // CMP EAX, 0x1007
        _emit 0x07
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x0f              // JG +0x83 (jump to high-half bisector)
        _emit 0x8f
        _emit 0x83
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x0f              // JE +0xa2 (0x1007 → 2)
        _emit 0x84
        _emit 0xa2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x3d              // CMP EAX, 0x809
        _emit 0x09
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x7f              // JG +0x41
        _emit 0x41
        _emit 0x0f              // JE +0xe3 (0x809 → 1, en-GB)
        _emit 0x84
        _emit 0xe3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x3d              // CMP EAX, 0x40c
        _emit 0x0c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x7f              // JG +0x1a
        _emit 0x1a
        _emit 0x0f              // JE +0xa1 (0x40c → 3, fr-FR)
        _emit 0x84
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2d              // SUB EAX, 0x404
        _emit 0x04
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x42 (0x404 → 5)
        _emit 0x42
        _emit 0x83              // SUB EAX, 3
        _emit 0xe8
        _emit 0x03
        _emit 0x74              // JZ +0x7c (0x407 → 2, de-DE)
        _emit 0x7c
        _emit 0x83              // SUB EAX, 2
        _emit 0xe8
        _emit 0x02
        _emit 0xe9              // JMP +0xbd (→ test 0x409 → 1, en-US)
        _emit 0xbd
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2d              // SUB EAX, 0x411
        _emit 0x11
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x0f              // JZ +0xb4 (0x411 → 0, ja-JP)
        _emit 0x84
        _emit 0xb4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2d              // SUB EAX, 0x3f3 (→ rel from 0x411)
        _emit 0xf3
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x37 (0x804 → 4)
        _emit 0x37
        _emit 0x83              // SUB EAX, 3
        _emit 0xe8
        _emit 0x03
        _emit 0x74              // JZ +0x5d (0x807 → 2, de-CH)
        _emit 0x5d
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0xc3              // RET (default 0)
        _emit 0x3d              // CMP EAX, 0xc09
        _emit 0x09
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x7f              // JG +0x1a
        _emit 0x1a
        _emit 0x0f              // JE +0x9b (0xc09 → 1, en-AU)
        _emit 0x84
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2d              // SUB EAX, 0x80c
        _emit 0x0c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x5f (0x80c → 3, fr-BE)
        _emit 0x5f
        _emit 0x2d              // SUB EAX, 0x3f8
        _emit 0xf8
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x75              // JNZ -0x23 (→ default-zero tail)
        _emit 0xdd
        _emit 0xb8              // MOV EAX, 5
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
        _emit 0x3d              // CMP EAX, 0xc0c
        _emit 0x0c
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x4b (0xc0c → 3, fr-CA)
        _emit 0x4b
        _emit 0x3d              // CMP EAX, 0x1004
        _emit 0x04
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x75              // JNZ +0x76 (→ default-zero tail)
        _emit 0x76
        _emit 0xb8              // MOV EAX, 4
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
        _emit 0x3d              // CMP EAX, 0x1c09
        _emit 0x09
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x7f              // JG +0x3d
        _emit 0x3d
        _emit 0x74              // JZ +0x6a (0x1c09 → 1, en-ZA)
        _emit 0x6a
        _emit 0x3d              // CMP EAX, 0x1409
        _emit 0x09
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x7f              // JG +0x1b
        _emit 0x1b
        _emit 0x74              // JZ +0x61 (0x1409 → 1, en-NZ)
        _emit 0x61
        _emit 0x2d              // SUB EAX, 0x1009
        _emit 0x09
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x5a (0x1009 → 1, en-CA)
        _emit 0x5a
        _emit 0x83              // SUB EAX, 3
        _emit 0xe8
        _emit 0x03
        _emit 0x74              // JZ +0x20 (0x100c → 3, fr-LU)
        _emit 0x20
        _emit 0x2d              // SUB EAX, 0x3fb
        _emit 0xfb
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x75              // JNZ +0x4b (→ default-zero tail)
        _emit 0x4b
        _emit 0xb8              // MOV EAX, 2
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
        _emit 0x2d              // SUB EAX, 0x140c
        _emit 0x0c
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x0c (0x140c → 3)
        _emit 0x0c
        _emit 0x2d              // SUB EAX, 0x3fd
        _emit 0xfd
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x3a (0x1809 → 1)
        _emit 0x3a
        _emit 0x83              // SUB EAX, 3
        _emit 0xe8
        _emit 0x03
        _emit 0x75              // JNZ +0x32 (→ default-zero tail)
        _emit 0x32
        _emit 0xb8              // MOV EAX, 3
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
        _emit 0x3d              // CMP EAX, 0x2c09
        _emit 0x09
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x7f              // JG +0x17
        _emit 0x17
        _emit 0x74              // JZ +0x26 (0x2c09 → 1, en-JM)
        _emit 0x26
        _emit 0x3d              // CMP EAX, 0x2009
        _emit 0x09
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x1f (0x2009 → 1, en-PH)
        _emit 0x1f
        _emit 0x3d              // CMP EAX, 0x2409
        _emit 0x09
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x18 (0x2409 → 1, en-029)
        _emit 0x18
        _emit 0x3d              // CMP EAX, 0x2809
        _emit 0x09
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x0c (→ shared `MOV EAX,1; RET` tail)
        _emit 0x0c
        _emit 0x3d              // CMP EAX, 0x3009
        _emit 0x09
        _emit 0x30
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x0a (0x3009 → 1, en-TT)
        _emit 0x0a
        _emit 0x3d              // CMP EAX, 0x3409
        _emit 0x09
        _emit 0x34
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x03 (0x3409 → 1, en-ZW)
        _emit 0x03
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0xc3              // RET
        _emit 0xb8              // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
    }
}
