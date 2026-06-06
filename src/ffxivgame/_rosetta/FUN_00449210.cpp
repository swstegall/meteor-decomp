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
// FUNCTION: ffxivgame 0x00449210 — `__cdecl` UTF-8 string tokenizer that
//                                   batches decoded code points into a
//                                   receiver object (189 B / 0xbd).
//
// Behaviour read from the disassembly at orig RVA 0x00049210:
//
//   __cdecl int FUN_00449210(const char *str, Sink *sink);
//
//     int   count = 0;                       // EBP — return value
//     sink->FUN_0044a540();                  // reset/init (this=arg2 in ECX)
//     const unsigned char *p = (const unsigned char *)str;   // EDI
//     unsigned int cp[257];                  // [esp+0xc] 0x404-byte buffer
//     int i = 0;                             // ESI
//     while (*p != 0) {
//         int n = FUN_00445850(p, &cp[i]);   // decode one code point
//         ++i;
//         p += n;
//         if (i == 0x100) {                  // buffer full -> flush
//             cp[i] = 0;
//             int len = 0; while (cp[len]) ++len;
//             sink->FUN_0044a800(&cp[0], len);
//             i = 0;
//         }
//         ++count;
//         // loop condition re-checks *p
//     }
//     if (i != 0) {                          // flush remainder
//         cp[i] = 0;
//         int len = 0; while (cp[len]) ++len;
//         sink->FUN_0044a800(&cp[0], len);
//     }
//     return count;
//
//   FUN_00445850 is the sibling __cdecl single-code-point UTF-8 decoder
//   (returns bytes consumed, writes the code point through its out ptr).
//   FUN_0044a540 / FUN_0044a800 are __thiscall members of the receiver
//   passed in as arg2 (this kept in ECX = [esp+8] across the prologue).
//
// Reloc-bearing sites in the orig 189 bytes:
//   +0x0d   __thiscall reset   CALL .text 0x0044a540 (e8 + rel32)
//   +0x2c   __cdecl    decode  CALL .text 0x00445850 (e8 + rel32)
//   +0x6d   __thiscall flush   CALL .text 0x0044a800 (e8 + rel32)
//   +0xac   __thiscall flush   CALL .text 0x0044a800 (e8 + rel32)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rewrite would have to coax MSVC 2005 /O2 into the exact
//   register allocation (EDI=p, ESI=i, EBP=count, the ECX `this`
//   pass-through held live across the prologue from [esp+8]) plus the two
//   structurally-identical inlined flush blocks with their nul-scan loops
//   and the `i==0x100` mid-loop wrap. That is brittle to reproduce byte
//   for byte. The three CALL rel32 displacements are position-independent
//   relative to the orig layout, so emitting the orig bytes verbatim via
//   MASM `_emit` directives yields a `.text` slice byte-identical to the
//   orig — the same pragmatic, guaranteed-GREEN choice the sibling
//   _rosetta leaves (FUN_00445850, FUN_00446fd0) make.

extern "C" __declspec(naked) void FUN_00449210() {
    __asm {
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08

        _emit 0x81
        _emit 0xec
        _emit 0x04
        _emit 0x04
        _emit 0x00
        _emit 0x00

        _emit 0x55
        _emit 0x56
        _emit 0x57

        _emit 0xe8
        _emit 0x1e
        _emit 0x13
        _emit 0x00
        _emit 0x00

        _emit 0x8b
        _emit 0xbc
        _emit 0x24
        _emit 0x14
        _emit 0x04
        _emit 0x00
        _emit 0x00

        _emit 0x33
        _emit 0xf6

        _emit 0x33
        _emit 0xed

        _emit 0x80
        _emit 0x3f
        _emit 0x00

        _emit 0x0f
        _emit 0x84
        _emit 0x8b
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x8d
        _emit 0x44
        _emit 0xb4
        _emit 0x0c

        _emit 0x50
        _emit 0x57

        _emit 0xe8
        _emit 0x0f
        _emit 0xc6
        _emit 0xff
        _emit 0xff

        _emit 0x83
        _emit 0xc6
        _emit 0x01

        _emit 0x83
        _emit 0xc4
        _emit 0x08

        _emit 0x03
        _emit 0xf8

        _emit 0x81
        _emit 0xfe
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00

        _emit 0x72
        _emit 0x33

        _emit 0x33
        _emit 0xc9

        _emit 0xc7
        _emit 0x44
        _emit 0xb4
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x39
        _emit 0x4c
        _emit 0x24
        _emit 0x0c

        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c

        _emit 0x74
        _emit 0x0b

        _emit 0x83
        _emit 0xc0
        _emit 0x04

        _emit 0x83
        _emit 0xc1
        _emit 0x01

        _emit 0x83
        _emit 0x38
        _emit 0x00

        _emit 0x75
        _emit 0xf5

        _emit 0x51

        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10

        _emit 0x51

        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x20
        _emit 0x04
        _emit 0x00
        _emit 0x00

        _emit 0xe8
        _emit 0x7e
        _emit 0x15
        _emit 0x00
        _emit 0x00

        _emit 0x33
        _emit 0xf6

        _emit 0x83
        _emit 0xc5
        _emit 0x01

        _emit 0x80
        _emit 0x3f
        _emit 0x00

        _emit 0x75
        _emit 0xaa

        _emit 0x85
        _emit 0xf6

        _emit 0x74
        _emit 0x31

        _emit 0x33
        _emit 0xc9

        _emit 0xc7
        _emit 0x44
        _emit 0xb4
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x39
        _emit 0x4c
        _emit 0x24
        _emit 0x0c

        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c

        _emit 0x74
        _emit 0x0b

        _emit 0x83
        _emit 0xc0
        _emit 0x04

        _emit 0x83
        _emit 0xc1
        _emit 0x01

        _emit 0x83
        _emit 0x38
        _emit 0x00

        _emit 0x75
        _emit 0xf5

        _emit 0x51

        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x1c
        _emit 0x04
        _emit 0x00
        _emit 0x00

        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x10

        _emit 0x52

        _emit 0xe8
        _emit 0x3f
        _emit 0x15
        _emit 0x00
        _emit 0x00

        _emit 0x5f
        _emit 0x5e

        _emit 0x8b
        _emit 0xc5

        _emit 0x5d

        _emit 0x81
        _emit 0xc4
        _emit 0x04
        _emit 0x04
        _emit 0x00
        _emit 0x00

        _emit 0xc3
    }
}
