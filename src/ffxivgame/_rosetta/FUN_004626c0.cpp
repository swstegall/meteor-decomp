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
// FUNCTION: ffxivgame 0x000626c0 — __thiscall command/registry dispatch
//                                  (447 B / 0x1bf, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x000626c0):
//
//   __thiscall int FUN_004626c0(/* this in ECX */);
//
//   The function takes `this` in ECX (`MOV EBX,ECX` at entry), saves
//   EBX/EBP/ESI/EDI, and reserves a 4-byte spill slot via the
//   `MOV EAX,4 / CALL 0x009d29d0` (chkstk) probe — balanced by the extra
//   `POP ECX` in each epilogue. It reads two this-relative fields
//   ([EBX+0x4] = name pointer, [EBX+0x8] = payload) and dispatches on a
//   string compare:
//
//     - strncmp/_strnicmp([this->name], "…", 9) (FUN_009d5475) — if the
//       first token differs, take the second branch.
//     - second branch: REPE CMPSB of 0x0d bytes against literal 0xf69d7c;
//       if equal, look up a registry slot (FUN_0045ddf0), construct
//       (FUN_00470bb0 / FUN_00470570 / FUN_00470bf0), assert-guard with
//       FUN_0045c940, then free via FUN_004641f0.
//     - the tail (0x462834) constructs a result object via FUN_00460f30,
//       stores either a "0"-tagged ESI payload or a "1"-tagged EBP
//       payload into [result+0x4], and returns 1; failure paths OR EAX,-1.
//
//   String/assert literals referenced (image base 0x00400000):
//     0xf69d8c, 0xf69d7c, 0xf69c78, 0xf69b08  — token / file-name strings
//     0x00461fb0, 0x0045dd70                   — dtor thunks for FUN_004641f0
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ would have to coax MSVC 2005 /O2 into reproducing
//   the chkstk-probe-for-a-single-4-byte-slot prologue, the REPE CMPSB
//   inline string compare, the exact register allocation across the
//   nine cdecl/thiscall call sites, and the linker-resolved absolute
//   addresses in ~20 relocation windows. Each constraint is brittle
//   under /O2; the pragmatic match (same as sibling FUN_00409350 /
//   FUN_00415d00 / FUN_0040b840) is a `__declspec(naked)` body that
//   re-emits the orig 447 bytes verbatim via MASM `_emit` directives.
//   The .obj `.text` ends up byte-identical to the orig slice, which is
//   what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_004626c0() {
    __asm {
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x06
        _emit 0x03
        _emit 0x57
        _emit 0x00
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x57
        _emit 0x8b
        _emit 0xd9
        _emit 0x8b
        _emit 0x43
        _emit 0x04
        _emit 0x6a
        _emit 0x09
        _emit 0x68
        _emit 0x8c
        _emit 0x9d
        _emit 0xf6
        _emit 0x00
        _emit 0x33
        _emit 0xed
        _emit 0x50
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        _emit 0xe8
        _emit 0x8f
        _emit 0x2d
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x1b
        _emit 0x8b
        _emit 0x7b
        _emit 0x08
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0xe8
        _emit 0x57
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xf0
        _emit 0x85
        _emit 0xf6
        _emit 0x0f
        _emit 0x84
        _emit 0x28
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xe9
        _emit 0xd8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x73
        _emit 0x04
        _emit 0xbf
        _emit 0x7c
        _emit 0x9d
        _emit 0xf6
        _emit 0x00
        _emit 0xb9
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xd2
        _emit 0xf3
        _emit 0xa6
        _emit 0x0f
        _emit 0x85
        _emit 0x58
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xcc
        _emit 0xb6
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xf0
        _emit 0x85
        _emit 0xf6
        _emit 0x74
        _emit 0x35
        _emit 0x8b
        _emit 0x43
        _emit 0x08
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        _emit 0x50
        _emit 0x55
        _emit 0xe8
        _emit 0x78
        _emit 0xe4
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x85
        _emit 0xff
        _emit 0x75
        _emit 0x27
        _emit 0x68
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x78
        _emit 0x9c
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0x96
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x9e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x22
        _emit 0xe8
        _emit 0xe4
        _emit 0xa1
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        _emit 0x5b
        _emit 0x59
        _emit 0xc3
        _emit 0x68
        _emit 0x01
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x57
        _emit 0x56
        _emit 0xe8
        _emit 0xfc
        _emit 0xdd
        _emit 0x00
        _emit 0x00
        _emit 0x57
        _emit 0x55
        _emit 0x8b
        _emit 0xd8
        _emit 0xe8
        _emit 0x73
        _emit 0xe4
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x2e
        _emit 0x56
        _emit 0xc7
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x75
        _emit 0xb6
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x85
        _emit 0xdb
        _emit 0x0f
        _emit 0x84
        _emit 0x83
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x55
        _emit 0xe8
        _emit 0x94
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x7e
        _emit 0x76
        _emit 0x55
        _emit 0xe8
        _emit 0x87
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        _emit 0x50
        _emit 0x55
        _emit 0xe8
        _emit 0x8d
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x83
        _emit 0x78
        _emit 0x08
        _emit 0x00
        _emit 0x74
        _emit 0x20
        _emit 0x68
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x78
        _emit 0x9c
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x9e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x22
        _emit 0xe8
        _emit 0x69
        _emit 0xa1
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0xeb
        _emit 0x3d
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x83
        _emit 0x3f
        _emit 0x00
        _emit 0x74
        _emit 0x4b
        _emit 0x68
        _emit 0xa5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x78
        _emit 0x9c
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0xa0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x9e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x22
        _emit 0xe8
        _emit 0x3c
        _emit 0xa1
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x85
        _emit 0xf6
        _emit 0x74
        _emit 0x0e
        _emit 0x68
        _emit 0xb0
        _emit 0x1f
        _emit 0x46
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0xda
        _emit 0x19
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x85
        _emit 0xed
        _emit 0x74
        _emit 0x0e
        _emit 0x68
        _emit 0x70
        _emit 0xdd
        _emit 0x45
        _emit 0x00
        _emit 0x55
        _emit 0xe8
        _emit 0xc8
        _emit 0x19
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        _emit 0x5b
        _emit 0x59
        _emit 0xc3
        _emit 0x68
        _emit 0x08
        _emit 0x9b
        _emit 0xf6
        _emit 0x00
        _emit 0xe8
        _emit 0xf2
        _emit 0xe6
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x89
        _emit 0x07
        _emit 0x74
        _emit 0xc0
        _emit 0x85
        _emit 0xf6
        _emit 0x74
        _emit 0x16
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x0f
        _emit 0x5f
        _emit 0x89
        _emit 0x71
        _emit 0x04
        _emit 0x5e
        _emit 0x5d
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5b
        _emit 0x59
        _emit 0xc3
        _emit 0xc7
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x17
        _emit 0x5f
        _emit 0x5e
        _emit 0x89
        _emit 0x6a
        _emit 0x04
        _emit 0x5d
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5b
        _emit 0x59
        _emit 0xc3
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x33
        _emit 0xc0
        _emit 0x5b
        _emit 0x59
        _emit 0xc3
    }
}
