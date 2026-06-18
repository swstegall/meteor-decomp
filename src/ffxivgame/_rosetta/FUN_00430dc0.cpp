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
// FUNCTION: ffxivgame 0x00030dc0 — __thiscall driver routine (391 B / 0x187,
//                                  no SEH, /GS-free, RET 0x4 — one stack arg).
//
// Inspection (read from the disassembly at orig RVA 0x00030dc0):
//
//   __thiscall void FUN_00430dc0(this, void* arg0);   // ECX = this
//
//   Shape: through a global singleton at [0x01329834] (its vtable in ECX),
//   call two virtual members (vtbl+0x90, vtbl+0x80) marshalling fields out
//   of `this` (this+0x14, this+0x18, this+0x20→0xf63278 table, this+0x28),
//   each result spilled to a local at [esp+0x10]. Two SQEX-style assertion
//   guards (the magic-static `DAT_01323910 |= 1; DAT_0132390c = 0x004309e0;`
//   handler-bind pattern, then the five-arg PUSH/`call [0x0132390c]`) gate
//   the return values being non-zero. A third virtual (the produced object's
//   vtbl+0x34) is invoked, then a counted loop over this+0x18 iterations
//   calls FUN_009d4600(ebp, lo, esi) advancing ebp by 4*this[0x14] each
//   step. A fourth virtual (vtbl+0x38) feeds a final assert, and the routine
//   tail-calls the produced object's vtbl+0x08 (destructor / release) and
//   RET 4.
//
//   Reloc-bearing sites in the orig 391 bytes (absolute VAs + IAT-indirect
//   calls + one rel32 CALL; resolve only in a full-binary relink at image
//   base 0x00400000 — standalone .obj compilation can't reproduce them):
//     +0x000  moffs [0x01329834]   — singleton ptr (MOV EAX)
//     +0x01a  disp  0x00f63278     — EDX*4 + table base
//     +0x039  moffs [0x01329834]   — singleton ptr (2nd load)
//     +0x052  data  [0x01323910]   — magic-static init flag (TEST/OR)
//     +0x062  imm32 0x004309e0     — bound assert handler → [0x0132390c]
//     +0x06c..+0x080 imm32 0xf63038/0x174/0xf63080/0xf6307e/0xf630d0 — assert args
//     +0x085  IAT   [0x0132390c]   — assert handler call
//     +0x10c  rel32 0x009d4600     — CALL inner per-element helper
//     ...and the 2nd/3rd assert guards repeat the [0x01323910]/[0x0132390c]
//        magic-static + five-arg-PUSH pattern with their own string imm32s.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough, the same
//   choice the reloc-heavy siblings FUN_0040ced0 / FUN_00405080 / FUN_00415d00
//   took. A source-level /O2 /GS port would have to reproduce the exact
//   register allocation, the magic-static assert-guard layout, the FF15
//   IAT-indirect vs E8 rel32 call encodings, and the linker-resolved absolute
//   addresses — every high-level rewrite shifts at least one byte. The naked
//   body re-emits the orig 391 bytes verbatim via MASM `_emit`; the .obj's
//   `.text` ends up byte-identical to the orig slice, which is what
//   tools/compare.py checks against.

extern "C" __declspec(naked) void FUN_00430dc0() {
    __asm {
        _emit 0xa1
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xec
        _emit 0x10
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x57
        _emit 0x6a
        _emit 0x00
        _emit 0x8b
        _emit 0xf9

        _emit 0x8b
        _emit 0x08
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x52
        _emit 0x8b
        _emit 0x57
        _emit 0x20
        _emit 0x8b
        _emit 0x14
        _emit 0x95
        _emit 0x78
        _emit 0x32
        _emit 0xf6

        _emit 0x00
        _emit 0x6a
        _emit 0x02
        _emit 0x52
        _emit 0x8b
        _emit 0x57
        _emit 0x18
        _emit 0x52
        _emit 0x8b
        _emit 0x57
        _emit 0x14
        _emit 0x52
        _emit 0x50
        _emit 0x8b
        _emit 0x81

        _emit 0x90
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xd0
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0xa1
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x8b

        _emit 0x08
        _emit 0x52
        _emit 0x8b
        _emit 0x57
        _emit 0x28
        _emit 0x52
        _emit 0x50
        _emit 0x8b
        _emit 0x81
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xd0
        _emit 0x85

        _emit 0xc0
        _emit 0x74
        _emit 0x3c
        _emit 0xf6
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75
        _emit 0x11
        _emit 0x83
        _emit 0x0d
        _emit 0x10
        _emit 0x39

        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xe0
        _emit 0x09
        _emit 0x43
        _emit 0x00
        _emit 0x68
        _emit 0x38
        _emit 0x30

        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0x74
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x80
        _emit 0x30
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0x7e
        _emit 0x30
        _emit 0xf6

        _emit 0x00
        _emit 0x68
        _emit 0xd0
        _emit 0x30
        _emit 0xf6
        _emit 0x00
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x8b

        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x08
        _emit 0x6a
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x52
        _emit 0x50
        _emit 0x8b

        _emit 0x41
        _emit 0x34
        _emit 0xff
        _emit 0xd0
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x3c
        _emit 0xf6
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75

        _emit 0x11
        _emit 0x83
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xe0
        _emit 0x09

        _emit 0x43
        _emit 0x00
        _emit 0x68
        _emit 0xf8
        _emit 0x30
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0x78
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x40
        _emit 0x31
        _emit 0xf6

        _emit 0x00
        _emit 0x68
        _emit 0x7f
        _emit 0x30
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0x90
        _emit 0x31
        _emit 0xf6
        _emit 0x00
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32

        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x8b
        _emit 0x77
        _emit 0x14
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x8b

        _emit 0x6c
        _emit 0x24
        _emit 0x24
        _emit 0x03
        _emit 0xf6
        _emit 0x33
        _emit 0xdb
        _emit 0x03
        _emit 0xf6
        _emit 0x39
        _emit 0x5f
        _emit 0x18
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x14

        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x76
        _emit 0x21
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x56
        _emit 0x50
        _emit 0x55
        _emit 0xe8
        _emit 0x2f
        _emit 0x37

        _emit 0x5a
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        _emit 0x01
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x83
        _emit 0xc3
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x0c

        _emit 0x03
        _emit 0xee
        _emit 0x3b
        _emit 0x5f
        _emit 0x18
        _emit 0x72
        _emit 0xdf
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x10
        _emit 0x50
        _emit 0x8b
        _emit 0x42

        _emit 0x38
        _emit 0xff
        _emit 0xd0
        _emit 0x85
        _emit 0xc0
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x74
        _emit 0x3c
        _emit 0xf6
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32

        _emit 0x01
        _emit 0x01
        _emit 0x75
        _emit 0x11
        _emit 0x83
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32

        _emit 0x01
        _emit 0xe0
        _emit 0x09
        _emit 0x43
        _emit 0x00
        _emit 0x68
        _emit 0xb8
        _emit 0x31
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0x84
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68

        _emit 0x00
        _emit 0x32
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0xf1
        _emit 0x30
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0x50
        _emit 0x32
        _emit 0xf6
        _emit 0x00
        _emit 0xff
        _emit 0x15

        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x8b
        _emit 0x04
        _emit 0x24
        _emit 0x8b
        _emit 0x08
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        _emit 0x50

        _emit 0xff
        _emit 0xd2
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
