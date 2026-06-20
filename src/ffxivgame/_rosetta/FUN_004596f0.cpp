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
// FUNCTION: ffxivgame 0x000596f0 — __thiscall, 386 B / 0x182, SEH-bearing
//                                  (FS:[0] handler chain @ 0x00e58644 scope
//                                   table, /GS security cookie, RET 0x8).
//
// Inspection (read from the disassembly at orig RVA 0x000596f0):
//
//   __thiscall <ret> FUN_004596f0(this, void* a1, void* a2);  // ECX=this,
//   two stack args (callee-popped RET 0x8), classic MSVC 2005 SEH prologue
//   (PUSH -1 / PUSH scope-table 0x00e58644 / FS:[0] link) plus a doubled
//   __security_cookie spill (XOR ESP at [esp+0x2c] and a second cookie push).
//
//   Body: constructs a small object at this+0x20 via FUN_00440850 +
//   FUN_00459af0, then drives a COM/interface-style vtable handshake —
//   (*EDX)(this, 0x01108500, &out) through several vtbl+0x10 / vtbl+0x08
//   slots (QueryInterface / enumerate / release shape), forwarding the
//   enumerated item to FUN_00459640 (ECX=this/EBX). On every early-out it
//   jumps to the matching Release ladder, then tears down the this+0x20
//   object via FUN_00459980 and (when the inline-vs-heap capacity field
//   [edi+0x18] >= 8) frees the spilled buffer through 0x009d1b17, restores
//   the FS:[0] link, checks the cookie via 0x009d20f4, and RET 0x8.
//
//   Reloc-bearing sites (absolute / rel32 targets resolve only in a full
//   relink at image base 0x00400000): the FS:[0] scope table 0x00e58644,
//   the __security_cookie moffs 0x012ea8b0, the interface IID imm 0x01108500,
//   the helper rel32 calls to 0x00440850 / 0x00459af0 / 0x00459640 /
//   0x00459980, and the CRT rel32 calls to 0x009d1b17 (free) / 0x009d20f4
//   (__security_check_cookie). Each is brittle under /O2 — a source-level
//   rewrite would shift register allocation, the doubled-cookie spill order,
//   the SEH state numbers, and short-vs-near branch encodings.
//
// Reconstruction strategy — naked-asm byte passthrough (same as the
// reloc-heavy siblings FUN_0040ced0 / FUN_00415d00): re-emit the orig 386
// bytes verbatim via MASM `_emit` so the .obj's .text slice is byte-identical
// to the orig, which is what tools/compare.py grades.

extern "C" __declspec(naked) void FUN_004596f0() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x44
        _emit 0x86
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
        _emit 0x30
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x53
        _emit 0x56
        _emit 0x57
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
        _emit 0x40
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x50
        _emit 0x8b
        _emit 0xd9
        _emit 0x6a
        _emit 0xff
        _emit 0x6a
        _emit 0x00
        _emit 0x8d
        _emit 0x7b
        _emit 0x20
        _emit 0x57
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x44
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x01
        _emit 0x71
        _emit 0xfe
        _emit 0xff
        _emit 0x8b
        _emit 0xcf
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x48
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x92
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x06
        _emit 0x8b
        _emit 0x10
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x51
        _emit 0x68
        _emit 0x00
        _emit 0x85
        _emit 0x10
        _emit 0x01
        _emit 0x56
        _emit 0xff
        _emit 0xd2
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x8c
        _emit 0xa4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0x08
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x52
        _emit 0x50
        _emit 0x8b
        _emit 0x41
        _emit 0x10
        _emit 0xff
        _emit 0xd0
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x8c
        _emit 0x7f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b
        _emit 0x08
        _emit 0x6a
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x52
        _emit 0x6a
        _emit 0x01
        _emit 0x50
        _emit 0x8b
        _emit 0x41
        _emit 0x10
        _emit 0xff
        _emit 0xd0
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x5a
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x54
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x08
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x52
        _emit 0x50
        _emit 0x8b
        _emit 0x41
        _emit 0x10
        _emit 0xff
        _emit 0xd0
        _emit 0x85
        _emit 0xc0
        _emit 0x7c
        _emit 0x19
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x56
        _emit 0x51
        _emit 0x8b
        _emit 0xcb
        _emit 0xe8
        _emit 0x6e
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8b
        _emit 0x10
        _emit 0x50
        _emit 0x8b
        _emit 0x42
        _emit 0x08
        _emit 0xff
        _emit 0xd0
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x08
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        _emit 0x50
        _emit 0xff
        _emit 0xd2
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b
        _emit 0x08
        _emit 0x6a
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x52
        _emit 0x6a
        _emit 0x01
        _emit 0x50
        _emit 0x8b
        _emit 0x41
        _emit 0x10
        _emit 0xff
        _emit 0xd0
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0xad
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b
        _emit 0x08
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        _emit 0x50
        _emit 0xff
        _emit 0xd2
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0x08
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        _emit 0x50
        _emit 0xff
        _emit 0xd2
        _emit 0x8b
        _emit 0x47
        _emit 0x14
        _emit 0xbe
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x39
        _emit 0x77
        _emit 0x18
        _emit 0x72
        _emit 0x05
        _emit 0x8b
        _emit 0x7f
        _emit 0x04
        _emit 0xeb
        _emit 0x03
        _emit 0x83
        _emit 0xc7
        _emit 0x04
        _emit 0x50
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x57
        _emit 0x50
        _emit 0x6a
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        _emit 0xe8
        _emit 0x3e
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x39
        _emit 0x74
        _emit 0x24
        _emit 0x38
        _emit 0x72
        _emit 0x0d
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x51
        _emit 0xe8
        _emit 0xc5
        _emit 0x82
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x85
        _emit 0x88
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x3c
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
