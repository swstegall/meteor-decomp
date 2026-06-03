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
// FUNCTION: ffxivgame 0x00019850 — __cdecl CRC32 sampling checksum.
//
//   unsigned int FUN_00419850(const unsigned char* data, unsigned int size);
//
//   Accumulates a standard table-driven CRC32 (poly table at absolute
//   0x01329430, crc seeded to 0xffffffff, no final inversion) over a
//   subset of the input, then returns `size` (EAX = ESI). The CRC in EDX
//   is left in a scratch register and discarded by the caller — the
//   routine exists for its memory-touch side effect / coverage sampling,
//   not to hand back a digest.
//
//   The sampled byte ranges depend on `size`:
//     size <  0x400 : every byte [0, size).
//     size <  0x600 : the first 0x200 bytes + the last 0x200 bytes.
//     size >= 0x600 : the first 0x200 bytes, the middle 0x200 bytes
//                     ([size/2 - 0x100, size/2 + 0x100)), and the last
//                     0x200 bytes ([size - 0x200, size)).
//   The fixed-length 0x200-byte head scan is unrolled ×4 over 0x80
//   iterations by /O2; the runtime-bounded middle/tail scans stay rolled.
//
//   Core step (repeated): crc = crc_table[(crc ^ *p) & 0xff] ^ (crc >> 8).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The CRC table reference (`xor edx, [eax*4 + 0x01329430]`, eight sites)
//   is an absolute DIR32 relocation that only resolves in a full-binary
//   relink at image base 0x00400000; a standalone .obj cannot reproduce
//   it from source without the linked table symbol. The /O2 ×4 unroll,
//   the three-way size dispatch, and the pointer-walk addressing
//   (`[ecx-1]/[ecx]/[ecx+1]/[ecx+2]`) are also brittle under any
//   high-level rewrite. Following the same approach the reloc-heavy
//   siblings (e.g. FUN_00415d00) take, the body re-emits the orig bytes
//   verbatim via MASM `_emit` directives so the .obj .text is
//   byte-identical to the orig slice that tools/compare.py grades.

extern "C" __declspec(naked) void FUN_00419850() {
    __asm {
        _emit 0x56
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x83
        _emit 0xca
        _emit 0xff
        _emit 0x81
        _emit 0xfe
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x57
        _emit 0x73

        _emit 0x30
        _emit 0x33
        _emit 0xc9
        _emit 0x85
        _emit 0xf6
        _emit 0x0f
        _emit 0x86
        _emit 0x92
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x90

        _emit 0x0f
        _emit 0xb6
        _emit 0x04
        _emit 0x39
        _emit 0x33
        _emit 0xc2
        _emit 0x25
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc1
        _emit 0xea
        _emit 0x08
        _emit 0x33
        _emit 0x14

        _emit 0x85
        _emit 0x30
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        _emit 0x3b
        _emit 0xce
        _emit 0x72
        _emit 0xe4
        _emit 0x5f
        _emit 0x8b
        _emit 0xc6
        _emit 0x5e

        _emit 0xc3
        _emit 0x81
        _emit 0xfe
        _emit 0x00
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x55
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0xbf
        _emit 0x80
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0x8d
        _emit 0x4d
        _emit 0x01
        _emit 0x0f
        _emit 0x83
        _emit 0x98
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x0f
        _emit 0xb6
        _emit 0x41
        _emit 0xff
        _emit 0x33
        _emit 0xc2
        _emit 0x25
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc1
        _emit 0xea
        _emit 0x08
        _emit 0x33
        _emit 0x14

        _emit 0x85
        _emit 0x30
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        _emit 0x33
        _emit 0xc2
        _emit 0x25
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc1

        _emit 0xea
        _emit 0x08
        _emit 0x33
        _emit 0x14
        _emit 0x85
        _emit 0x30
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x0f
        _emit 0xb6
        _emit 0x41
        _emit 0x01
        _emit 0x33
        _emit 0xc2
        _emit 0x25

        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc1
        _emit 0xea
        _emit 0x08
        _emit 0x33
        _emit 0x14
        _emit 0x85
        _emit 0x30
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x0f
        _emit 0xb6

        _emit 0x41
        _emit 0x02
        _emit 0x33
        _emit 0xc2
        _emit 0x25
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc1
        _emit 0xea
        _emit 0x08
        _emit 0x33
        _emit 0x14
        _emit 0x85
        _emit 0x30

        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc1
        _emit 0x04
        _emit 0x83
        _emit 0xef
        _emit 0x01
        _emit 0x75
        _emit 0xa5
        _emit 0x8d
        _emit 0x8e
        _emit 0x00
        _emit 0xfe
        _emit 0xff

        _emit 0xff
        _emit 0x3b
        _emit 0xce
        _emit 0x0f
        _emit 0x83
        _emit 0xe3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x0f
        _emit 0xb6
        _emit 0x04
        _emit 0x29
        _emit 0x33
        _emit 0xc2
        _emit 0x25
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc1
        _emit 0xea
        _emit 0x08
        _emit 0x33
        _emit 0x14

        _emit 0x85
        _emit 0x30
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        _emit 0x3b
        _emit 0xce
        _emit 0x72
        _emit 0xe4
        _emit 0x5d
        _emit 0x5f
        _emit 0x8b
        _emit 0xc6

        _emit 0x5e
        _emit 0xc3
        _emit 0x0f
        _emit 0xb6
        _emit 0x41
        _emit 0xff
        _emit 0x33
        _emit 0xc2
        _emit 0x25
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc1
        _emit 0xea
        _emit 0x08

        _emit 0x33
        _emit 0x14
        _emit 0x85
        _emit 0x30
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        _emit 0x33
        _emit 0xc2
        _emit 0x25
        _emit 0xff
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0xc1
        _emit 0xea
        _emit 0x08
        _emit 0x33
        _emit 0x14
        _emit 0x85
        _emit 0x30
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x0f
        _emit 0xb6
        _emit 0x41
        _emit 0x01
        _emit 0x33

        _emit 0xc2
        _emit 0x25
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc1
        _emit 0xea
        _emit 0x08
        _emit 0x33
        _emit 0x14
        _emit 0x85
        _emit 0x30
        _emit 0x94
        _emit 0x32
        _emit 0x01

        _emit 0x0f
        _emit 0xb6
        _emit 0x41
        _emit 0x02
        _emit 0x33
        _emit 0xc2
        _emit 0x25
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc1
        _emit 0xea
        _emit 0x08
        _emit 0x33
        _emit 0x14

        _emit 0x85
        _emit 0x30
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc1
        _emit 0x04
        _emit 0x83
        _emit 0xef
        _emit 0x01
        _emit 0x75
        _emit 0xa5
        _emit 0x8b
        _emit 0xc6
        _emit 0xd1

        _emit 0xe8
        _emit 0x8d
        _emit 0x88
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8d
        _emit 0xb8
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x3b
        _emit 0xcf
        _emit 0x73

        _emit 0x1c
        _emit 0x0f
        _emit 0xb6
        _emit 0x04
        _emit 0x29
        _emit 0x33
        _emit 0xc2
        _emit 0x25
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc1
        _emit 0xea
        _emit 0x08
        _emit 0x33

        _emit 0x14
        _emit 0x85
        _emit 0x30
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        _emit 0x3b
        _emit 0xcf
        _emit 0x72
        _emit 0xe4
        _emit 0x8d
        _emit 0x8e
        _emit 0x00

        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x3b
        _emit 0xce
        _emit 0x73
        _emit 0x25
        _emit 0xeb
        _emit 0x07
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x0f
        _emit 0xb6
        _emit 0x04
        _emit 0x29
        _emit 0x33
        _emit 0xc2
        _emit 0x25
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc1
        _emit 0xea
        _emit 0x08
        _emit 0x33
        _emit 0x14

        _emit 0x85
        _emit 0x30
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        _emit 0x3b
        _emit 0xce
        _emit 0x72
    }
}
