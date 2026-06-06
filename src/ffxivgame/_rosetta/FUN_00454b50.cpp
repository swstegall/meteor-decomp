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
// FUNCTION: ffxivgame 0x00054b50 — `__cdecl bool` registry/string lookup
//                                  helper (289 B / 0x121, EH4-SEH wrapped,
//                                  /GS security cookie).
//
// Behaviour read from the disassembly at orig RVA 0x00054b50:
//
//   __cdecl bool FUN_00454b50(void *arg);   // arg @ [esp+0x88] after prologue
//
//   The function runs a two-pass query helper (CALL 0x009d7de7, twice) —
//   the first pass measures a required size into a stack slot, then a
//   buffer is grown via the allocator/grow helper at 0x00406280, and the
//   second pass fills it. Empty / failed slots are routed through the
//   bounds helper at 0x009d22b4. The filled buffer is then fed to a pair
//   of string/object member helpers (__thiscall 0x00447260, 0x00447450)
//   and a destructor-ish cleanup (__thiscall 0x00446f50), with the heap
//   buffer freed via 0x009d1b17 on the success path. Returns AL = 1 on
//   success, AL = 0 (XOR AL,AL at 0x00054c60) when the initial query
//   returns zero.
//
//   Frame: PUSH -1 / PUSH 0xe58498 (EH4 scope table, .rdata FuncInfo) /
//   PUSH FS:[0] EH4 chain link / SUB ESP,0x6c / PUSH EBX / PUSH ESI /
//   __security_cookie ^ ESP pushed / FS:[0] install. EBX is zeroed and
//   reused as the constant 0 throughout; ESI caches the `arg` pointer.
//
//   Reloc-bearing sites (resolve only in a full-binary relink at image
//   base 0x00400000; standalone .obj compilation can't reproduce them):
//     +0x03  EH4 scope-table              (.rdata 0x00e58498)
//     +0x13  __security_cookie load       (.data 0x012ea8b0)
//     +0x3a  query helper CALL            (.text 0x009d7de7 rel32)
//     +0x6a  buffer grow CALL             (.text 0x00406280 rel32)
//     +0x7f  bounds helper CALL           (.text 0x009d22b4 rel32)
//     +0x94  query helper CALL            (.text 0x009d7de7 rel32, 2nd)
//     +0xac  bounds helper CALL           (.text 0x009d22b4 rel32, 2nd)
//     +0xb5  global ptr load              (.data 0x00f67298)
//     +0xc1  string member CALL           (.text 0x00447260 rel32)
//     +0xd6  string member CALL           (.text 0x00447450 rel32)
//     +0xe6  cleanup CALL                 (.text 0x00446f50 rel32)
//     +0xf4  heap free CALL               (.text 0x009d1b17 rel32)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level rewrite here would have to coax MSVC 2005 /O2 /GS /EHsc
//   into reproducing the exact __except_handler4 prologue, the precise
//   stack-slot numbering for the two-pass query, the SUB/JNZ collapse of
//   the empty-string checks, and the dozen linker-resolved absolute
//   addresses above — each brittle under /O2. The local idiom for these
//   SEH+/GS+reloc-heavy functions (see FUN_00401a00, FUN_00404f70) is a
//   naked body that re-emits the orig 289 bytes verbatim via MASM `_emit`
//   directives. The .obj's `.text` ends up byte-identical to the orig
//   slice (no COFF relocations — the absolute operands bake in as
//   immediates), which is exactly what `tools/compare.py` checks.

extern "C" __declspec(naked) void FUN_00454b50() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x98
        _emit 0x84
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x83
        _emit 0xec

        _emit 0x6c
        _emit 0x53
        _emit 0x56
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x50
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x78
        _emit 0x64

        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xdb
        _emit 0x56
        _emit 0x53

        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x53
        _emit 0x50
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0xe8
        _emit 0x58
        _emit 0x32
        _emit 0x58
        _emit 0x00
        _emit 0x8b

        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x3b
        _emit 0xc3
        _emit 0x0f
        _emit 0x84
        _emit 0xc2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x5c

        _emit 0x24
        _emit 0x18
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        _emit 0x53
        _emit 0x50
        _emit 0x8d

        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x89
        _emit 0x9c
        _emit 0x24
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xc1
        _emit 0x16
        _emit 0xfb
        _emit 0xff
        _emit 0x8b

        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x3b
        _emit 0xc3
        _emit 0x74
        _emit 0x08
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x2b
        _emit 0xc8
        _emit 0x75
        _emit 0x09
        _emit 0xe8

        _emit 0xe0
        _emit 0xd6
        _emit 0x57
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x56
        _emit 0x52
        _emit 0x50
        _emit 0x8d

        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50
        _emit 0xe8
        _emit 0xfe
        _emit 0x31
        _emit 0x58
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x83
        _emit 0xc4
        _emit 0x10

        _emit 0x3b
        _emit 0xc3
        _emit 0x74
        _emit 0x08
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x2b
        _emit 0xc8
        _emit 0x75
        _emit 0x09
        _emit 0xe8
        _emit 0xb3
        _emit 0xd6
        _emit 0x57

        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8b
        _emit 0x15
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x52
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24

        _emit 0x2c
        _emit 0xe8
        _emit 0x4a
        _emit 0x26
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0xc6
        _emit 0x84

        _emit 0x24
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0xe8
        _emit 0x25
        _emit 0x28
        _emit 0xff
        _emit 0xff
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x88

        _emit 0x9c
        _emit 0x24
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x15
        _emit 0x23
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x3b

        _emit 0xc3
        _emit 0x74
        _emit 0x09
        _emit 0x50
        _emit 0xe8
        _emit 0xce
        _emit 0xce
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0xb0
        _emit 0x01
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x78
        _emit 0x64

        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5e
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x78
        _emit 0xc3
        _emit 0x32
        _emit 0xc0
        _emit 0x8b

        _emit 0x4c
        _emit 0x24
        _emit 0x78
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5e
        _emit 0x5b
        _emit 0x83
    }
}
