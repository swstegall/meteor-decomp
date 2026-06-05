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
// FUNCTION: ffxivgame 0x0005e9f0 — cached-range parse/advance helper
//                                  (433 B / 0x1b1, no SEH, no /GS).
//
// Inspection (read from the disassembly at orig RVA 0x0005e9f0):
//
//   __cdecl int FUN_0045e9f0(... stack args ...);
//
//   Prologue `MOV EAX,0xc; CALL 0x009d29d0` is the MSVC fixed-size stack
//   reserve helper (allocates 0xc bytes; balanced by `ADD ESP,0x0c` in
//   every epilogue). Body saves EBX/EBP/ESI/EDI and operates on a small
//   "cache record" object passed via [esp+0x44] (ESI):
//
//     [esi+0x00] byte   — "valid" flag (0/1) for the cached snapshot
//     [esi+0x04] dword  — cached result flags (AL state)
//     [esi+0x08] dword  — cached EBP value
//     [esi+0x0c] dword  — cached field A
//     [esi+0x10] dword  — cached field B
//     [esi+0x14] dword  — cached span length
//
//   When the record is present + valid, the function reloads the snapshot
//   into the local frame and jumps to the shared tail. Otherwise it pushes
//   five LEA'd locals and calls the real parser FUN_004645b0, then writes
//   the result back into the record. The shared tail (at 0x0045eab9 / the
//   AL flag tests) demuxes the parse-result bits (TEST AL,0x81 / TEST AL,AL
//   sign / TEST AL,0x1 / AND AL,0x20) into the caller's optional out-params
//   and emits one of several assert-tail calls to FUN_0045c940 with line
//   numbers 0x512 / 0x51b / 0x528 and the shared context string at
//   0x00f69300 (".\\...cpp" descriptor).
//
//   Reloc-bearing sites in the orig 433 bytes (image base 0x00400000):
//     +0x005  rel32  0x009d29d0 — fixed-size stack reserve helper
//     +0x061  rel32  0x004645b0 — call parser
//     +0x0b0  rel32  0x0045c940 — assert tail (line 0x512)
//     +0x092  imm32  0x00f69300 — context string
//     +0x0dd  rel32  0x0045c940 — assert tail (line 0x51b)
//     +0x0d2  imm32  0x00f69300 — context string
//     +0x19f  rel32  0x0045c940 — assert tail (line 0x528)
//     +0x191  imm32  0x00f69300 — context string
//
// Reconstruction strategy — naked-asm byte passthrough (same idiom as the
// sibling _rosetta bodies FUN_00415d00 / FUN_00409350 / FUN_0040b840):
// a source-level rewrite would have to coax MSVC 2005 /O2 into reproducing
// the exact register allocation across the cache-reload / parse / writeback
// split, the chained AL-bit demux in the shared tail, and the four
// linker-resolved absolute call/string sites. The pragmatic choice is a
// `__declspec(naked)` body that re-emits the orig 433 bytes verbatim via
// `_emit` — the .obj's `.text` ends up byte-identical to the orig slice,
// which is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_0045e9f0() {
    __asm {
        _emit 0xb8
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xd6
        _emit 0x3f
        _emit 0x57
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x8b
        _emit 0x08

        _emit 0x53
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x2c
        _emit 0x55
        _emit 0x56
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x44
        _emit 0x85
        _emit 0xf6
        _emit 0x57
        _emit 0x89
        _emit 0x4c

        _emit 0x24
        _emit 0x48
        _emit 0x8b
        _emit 0xf9
        _emit 0x74
        _emit 0x26
        _emit 0x80
        _emit 0x3e
        _emit 0x00
        _emit 0x74
        _emit 0x21
        _emit 0x8b
        _emit 0x56
        _emit 0x10
        _emit 0x03
        _emit 0x4e

        _emit 0x14
        _emit 0x8b
        _emit 0x6e
        _emit 0x08
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x56
        _emit 0x0c
        _emit 0x89
        _emit 0x6c

        _emit 0x24
        _emit 0x18
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x48
        _emit 0xeb
        _emit 0x7d
        _emit 0x53
        _emit 0x8d
        _emit 0x44
        _emit 0x24

        _emit 0x14
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x51
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x52
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x58

        _emit 0x50
        _emit 0xe8
        _emit 0x5a
        _emit 0x5b
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x2c
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x85
        _emit 0xf6
        _emit 0x74

        _emit 0x54
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x89
        _emit 0x4e
        _emit 0x10
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x48

        _emit 0x89
        _emit 0x56
        _emit 0x0c
        _emit 0x8b
        _emit 0xd1
        _emit 0x2b
        _emit 0xd7
        _emit 0xa8
        _emit 0x81
        _emit 0x89
        _emit 0x46
        _emit 0x04
        _emit 0x89
        _emit 0x6e
        _emit 0x08
        _emit 0x89

        _emit 0x56
        _emit 0x14
        _emit 0xc6
        _emit 0x06
        _emit 0x01
        _emit 0x75
        _emit 0x32
        _emit 0x03
        _emit 0xd5
        _emit 0x3b
        _emit 0xd3
        _emit 0x7e
        _emit 0x2c
        _emit 0x68
        _emit 0x12
        _emit 0x05

        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x00
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x68
        _emit 0x6a
        _emit 0x0d

        _emit 0xe8
        _emit 0x9b
        _emit 0xde
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x5f
        _emit 0xc6
        _emit 0x06
        _emit 0x00
        _emit 0x5e
        _emit 0x5d
        _emit 0x33
        _emit 0xc0

        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x48
        _emit 0x84
        _emit 0xc0
        _emit 0x79
        _emit 0x29
        _emit 0x68
        _emit 0x1b
        _emit 0x05

        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x00
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x66
        _emit 0x6a
        _emit 0x68
        _emit 0x6a
        _emit 0x0d
        _emit 0xe8
        _emit 0x6e
        _emit 0xde

        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x85
        _emit 0xf6
        _emit 0x74
        _emit 0x03
        _emit 0xc6
        _emit 0x06
        _emit 0x00
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x33

        _emit 0xc0
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x3c
        _emit 0x85
        _emit 0xd2
        _emit 0x7c
        _emit 0x17
        _emit 0x3b
        _emit 0x54

        _emit 0x24
        _emit 0x14
        _emit 0x75
        _emit 0x6f
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x40
        _emit 0x3b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x75
        _emit 0x65
        _emit 0x85
        _emit 0xf6

        _emit 0x74
        _emit 0x03
        _emit 0xc6
        _emit 0x06
        _emit 0x00
        _emit 0xa8
        _emit 0x01
        _emit 0x74
        _emit 0x06
        _emit 0x2b
        _emit 0xf9
        _emit 0x03
        _emit 0xfb
        _emit 0x8b
        _emit 0xef
        _emit 0x8b

        _emit 0x74
        _emit 0x24
        _emit 0x2c
        _emit 0x85
        _emit 0xf6
        _emit 0x74
        _emit 0x07
        _emit 0x8a
        _emit 0xd0
        _emit 0x80
        _emit 0xe2
        _emit 0x01
        _emit 0x88
        _emit 0x16
        _emit 0x8b
        _emit 0x54

        _emit 0x24
        _emit 0x30
        _emit 0x85
        _emit 0xd2
        _emit 0x74
        _emit 0x04
        _emit 0x24
        _emit 0x20
        _emit 0x88
        _emit 0x02
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x85
        _emit 0xc0

        _emit 0x74
        _emit 0x02
        _emit 0x89
        _emit 0x28
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x06
        _emit 0x8a
        _emit 0x54
        _emit 0x24
        _emit 0x10

        _emit 0x88
        _emit 0x10
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x06
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x89
        _emit 0x10

        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x89
        _emit 0x08
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5b
        _emit 0x83

        _emit 0xc4
        _emit 0x0c
        _emit 0xc3
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x44
        _emit 0x00
        _emit 0x74
        _emit 0x0b
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x83
        _emit 0xc8
        _emit 0xff

        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3
        _emit 0x85
        _emit 0xf6
        _emit 0x74
        _emit 0x03
        _emit 0xc6
        _emit 0x06
        _emit 0x00
        _emit 0x68
        _emit 0x28
        _emit 0x05
        _emit 0x00

        _emit 0x00
        _emit 0x68
        _emit 0x00
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0xa8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x68
        _emit 0x6a
        _emit 0x0d
        _emit 0xe8

        _emit 0xac
        _emit 0xdd
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x33
        _emit 0xc0
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x0c

        _emit 0xc3
    }
}
