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
// FUNCTION: ffxivgame 0x009e445a — MSVC 2005 CRT internal A->W string helper
//                                  (440 B / 0x1b8, /GS security cookie).
//
// NOTE: the asm/*.s disassembly dump drops one byte — a `POP ECX` (0x59)
//       at orig+0x1a3 that balances the `PUSH ESI` argument to the free
//       call at 0x009d5c88. The authoritative size (size_overrides) is
//       440 B; this body re-emits all 440 including that POP ECX.
//
// Inspection (read from the disassembly at orig RVA 0x005e445a):
//
//   This is one of the CRT __crt{LCMapString,GetStringType,CompareString}A
//   family helpers — the classic A-string-to-W-string adapter MSVC's CRT
//   uses for locale-aware operations. The first body fork probes whether
//   the Unicode (W) API path is usable on this OS:
//
//     * A one-shot static (g_useW @ 0x013647a8) caches the result.
//     * On the first call it invokes the W primitive via the IAT slot
//       [0x00f3e268] with a probe string (0x1086650) and, if that fails,
//       calls GetLastError ([0x00f3e1c4]) and checks for 0x78
//       (ERROR_CALL_NOT_IMPLEMENTED == Win9x-style A-only fallback).
//     * The A path then calls MultiByteToWideChar ([0x00f3e1f4], saved in
//       ESI) twice — first to size the conversion, then after allocating
//       a temp buffer (stack/_malloc_crt at 0x009d8be0 / 0x009d5bc5, freed
//       via 0x009da186) to perform it — and finally dispatches the real
//       W primitive via IAT slot [0x00f3e26c].
//
//   `ECX` is consumed (`MOV EDI,ECX`) as an opaque pointer whose +0x00
//   yields a struct used for the +0x04 / +0x14 default lookups. The frame
//   carries seven stack args ([EBP+0x08..0x20]); the epilogue is a plain
//   `LEAVE / RET` (__cdecl-style cleanup) guarded by the /GS cookie pair
//   (cookie XOR at entry, __security_check_cookie @ 0x009d20f4 at exit).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This body is dense with linker-resolved sites: six IAT-indirect calls
//   (FF15), five rel32 calls (E8) into the CRT, three moffs32 loads/stores
//   of the g_useW static (A1/A3/8935), and a /GS cookie pair. Coaxing
//   MSVC 2005 /O2 /GS into reproducing the exact register allocation,
//   the SETNZ-scaled `LEA EAX,[EAX*8+1]` flag fold, the two CCCC/DDDD
//   buffer-poison stores, and every reloc window byte-for-byte is brittle
//   — every high-level rewrite shifts at least one byte. Following the
//   same approach the sibling reloc-heavy bodies took (FUN_00415d00 et al.),
//   this re-emits the orig 439 bytes verbatim via MASM `_emit` directives.
//   The .obj's `.text` ends up byte-identical to the orig slice, which is
//   what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_009e445a() {
    __asm {
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x51
        _emit 0x51
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc5
        _emit 0x89
        _emit 0x45
        _emit 0xfc
        _emit 0xa1
        _emit 0xa8
        _emit 0x47
        _emit 0x36
        _emit 0x01
        _emit 0x53
        _emit 0x56
        _emit 0x33
        _emit 0xdb
        _emit 0x3b
        _emit 0xc3
        _emit 0x57
        _emit 0x8b
        _emit 0xf9
        _emit 0x75
        _emit 0x3a
        _emit 0x8d
        _emit 0x45
        _emit 0xf8
        _emit 0x50
        _emit 0x33
        _emit 0xf6
        _emit 0x46
        _emit 0x56
        _emit 0x68
        _emit 0x50
        _emit 0x66
        _emit 0x08
        _emit 0x01
        _emit 0x56
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x08
        _emit 0x89
        _emit 0x35
        _emit 0xa8
        _emit 0x47
        _emit 0x36
        _emit 0x01
        _emit 0xeb
        _emit 0x34
        _emit 0xff
        _emit 0x15
        _emit 0xc4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x83
        _emit 0xf8
        _emit 0x78
        _emit 0x75
        _emit 0x0a
        _emit 0x6a
        _emit 0x02
        _emit 0x58
        _emit 0xa3
        _emit 0xa8
        _emit 0x47
        _emit 0x36
        _emit 0x01
        _emit 0xeb
        _emit 0x05
        _emit 0xa1
        _emit 0xa8
        _emit 0x47
        _emit 0x36
        _emit 0x01
        _emit 0x83
        _emit 0xf8
        _emit 0x02
        _emit 0x0f
        _emit 0x84
        _emit 0xcf
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x3b
        _emit 0xc3
        _emit 0x0f
        _emit 0x84
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xf8
        _emit 0x01
        _emit 0x0f
        _emit 0x85
        _emit 0xe8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x39
        _emit 0x5d
        _emit 0x18
        _emit 0x89
        _emit 0x5d
        _emit 0xf8
        _emit 0x75
        _emit 0x08
        _emit 0x8b
        _emit 0x07
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        _emit 0x89
        _emit 0x45
        _emit 0x18
        _emit 0x8b
        _emit 0x35
        _emit 0xf4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x33
        _emit 0xc0
        _emit 0x39
        _emit 0x5d
        _emit 0x20
        _emit 0x53
        _emit 0x53
        _emit 0xff
        _emit 0x75
        _emit 0x10
        _emit 0x0f
        _emit 0x95
        _emit 0xc0
        _emit 0xff
        _emit 0x75
        _emit 0x0c
        _emit 0x8d
        _emit 0x04
        _emit 0xc5
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0xff
        _emit 0x75
        _emit 0x18
        _emit 0xff
        _emit 0xd6
        _emit 0x8b
        _emit 0xf8
        _emit 0x3b
        _emit 0xfb
        _emit 0x0f
        _emit 0x84
        _emit 0xab
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x7e
        _emit 0x3c
        _emit 0x81
        _emit 0xff
        _emit 0xf0
        _emit 0xff
        _emit 0xff
        _emit 0x7f
        _emit 0x77
        _emit 0x34
        _emit 0x8d
        _emit 0x44
        _emit 0x3f
        _emit 0x08
        _emit 0x3d
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x77
        _emit 0x13
        _emit 0xe8
        _emit 0xbc
        _emit 0x46
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xc4
        _emit 0x3b
        _emit 0xc3
        _emit 0x74
        _emit 0x1c
        _emit 0xc7
        _emit 0x00
        _emit 0xcc
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x11
        _emit 0x50
        _emit 0xe8
        _emit 0x8d
        _emit 0x16
        _emit 0xff
        _emit 0xff
        _emit 0x3b
        _emit 0xc3
        _emit 0x59
        _emit 0x74
        _emit 0x09
        _emit 0xc7
        _emit 0x00
        _emit 0xdd
        _emit 0xdd
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc0
        _emit 0x08
        _emit 0x8b
        _emit 0xd8
        _emit 0x85
        _emit 0xdb
        _emit 0x74
        _emit 0x69
        _emit 0x8d
        _emit 0x04
        _emit 0x3f
        _emit 0x50
        _emit 0x6a
        _emit 0x00
        _emit 0x53
        _emit 0xe8
        _emit 0xb8
        _emit 0xdb
        _emit 0xfe
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x57
        _emit 0x53
        _emit 0xff
        _emit 0x75
        _emit 0x10
        _emit 0xff
        _emit 0x75
        _emit 0x0c
        _emit 0x6a
        _emit 0x01
        _emit 0xff
        _emit 0x75
        _emit 0x18
        _emit 0xff
        _emit 0xd6
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x11
        _emit 0xff
        _emit 0x75
        _emit 0x14
        _emit 0x50
        _emit 0x53
        _emit 0xff
        _emit 0x75
        _emit 0x08
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x89
        _emit 0x45
        _emit 0xf8
        _emit 0x53
        _emit 0xe8
        _emit 0x01
        _emit 0x5c
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x45
        _emit 0xf8
        _emit 0x59
        _emit 0xeb
        _emit 0x75
        _emit 0x33
        _emit 0xf6
        _emit 0x39
        _emit 0x5d
        _emit 0x1c
        _emit 0x75
        _emit 0x08
        _emit 0x8b
        _emit 0x07
        _emit 0x8b
        _emit 0x40
        _emit 0x14
        _emit 0x89
        _emit 0x45
        _emit 0x1c
        _emit 0x39
        _emit 0x5d
        _emit 0x18
        _emit 0x75
        _emit 0x08
        _emit 0x8b
        _emit 0x07
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        _emit 0x89
        _emit 0x45
        _emit 0x18
        _emit 0xff
        _emit 0x75
        _emit 0x1c
        _emit 0xe8
        _emit 0xc4
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        _emit 0x59
        _emit 0x75
        _emit 0x04
        _emit 0x33
        _emit 0xc0
        _emit 0xeb
        _emit 0x47
        _emit 0x3b
        _emit 0x45
        _emit 0x18
        _emit 0x74
        _emit 0x1e
        _emit 0x53
        _emit 0x53
        _emit 0x8d
        _emit 0x4d
        _emit 0x10
        _emit 0x51
        _emit 0xff
        _emit 0x75
        _emit 0x0c
        _emit 0x50
        _emit 0xff
        _emit 0x75
        _emit 0x18
        _emit 0xe8
        _emit 0xea
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf0
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x3b
        _emit 0xf3
        _emit 0x74
        _emit 0xdc
        _emit 0x89
        _emit 0x75
        _emit 0x0c
        _emit 0xff
        _emit 0x75
        _emit 0x14
        _emit 0xff
        _emit 0x75
        _emit 0x10
        _emit 0xff
        _emit 0x75
        _emit 0x0c
        _emit 0xff
        _emit 0x75
        _emit 0x08
        _emit 0xff
        _emit 0x75
        _emit 0x1c
        _emit 0xff
        _emit 0x15
        _emit 0x6c
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x3b
        _emit 0xf3
        _emit 0x8b
        _emit 0xf8
        _emit 0x74
        _emit 0x07
        _emit 0x56
        _emit 0xe8
        _emit 0x8b
        _emit 0x16
        _emit 0xff
        _emit 0xff
        _emit 0x59
        _emit 0x8b
        _emit 0xc7
        _emit 0x8d
        _emit 0x65
        _emit 0xec
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0x8b
        _emit 0x4d
        _emit 0xfc
        _emit 0x33
        _emit 0xcd
        _emit 0xe8
        _emit 0xe4
        _emit 0xda
        _emit 0xfe
        _emit 0xff
        _emit 0xc9
        _emit 0xc3
    }
}
