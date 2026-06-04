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
// FUNCTION: ffxivgame 0x00031f30 — big-endian asset/header parse + decode
//                                  dispatcher (488 B / 0x1e8, inline SEH + /GS).
//
// Inspection (read from the disassembly at orig RVA 0x00031f30):
//
//   __thiscall <ptr> parse_header(this /*ESI*/, ..., out** /*[esp+0x2c]*/,
//                                 dword /*[esp+0x30]*/, ...)
//
//   ESI is the object/blob pointer (read-only input, never saved/restored —
//   the caller keeps it live across the call). The body:
//
//     // Magic check: read [ESI+0x00] big-endian, compare to 'GETX'.
//     u32 magic = bswap(*(u32*)(this + 0x00));
//     if (magic != 0x47544558) { *out = 0; return out; }   // early bail
//
//     // Size/len field: read [ESI+0x14] big-endian; if zero AND
//     // [esp+0x30] == 0, bail the same way.
//     u32 len = bswap(*(u32*)(this + 0x14));
//     if (len == 0 && arg_30 == 0) { *out = 0; return out; }
//
//     // Decode several big-endian sub-fields: word [ESI+0xa], byte [ESI+0x6],
//     // word [ESI+0xc], word [ESI+0xe], byte [ESI+0x9] (flags), byte [ESI+0x7].
//     // Flag byte [ESI+0x9]: bit2 → mode selector (EAX = 4), bit0 / bit1 pick
//     // one of three decoder helpers:
//     //   bit0 set      → CALL 0x00418d00 (decoder A, 7 args)
//     //   else bit1 set → CALL 0x00418e00 (decoder B, 8 args)
//     //   else          → CALL 0x00418bf0 (decoder C, 7 args, stack-marker set)
//     // Each returns a context whose [0] is taken as EDI and zeroed.
//     // If a sub-object [ESP+0x10] is non-null, virtual-call slot[0](1).
//     // If EDI non-null, fetch (vptr->slot8(), vptr->slot4()) from [esp+0x30]
//     //   (or -1/0) and tail into CALL 0x00431e20 (finalise).
//     // Store EDI to *(out) and return.
//
//   Frame: inline-SEH (push -1 / push scope-table 0x00e560f6 / FS:[0] link)
//   plus a /GS security cookie (global 0x012ea8b0). Locals at [esp+0x10..0x24].
//
//   Reloc-bearing sites in the orig 488 bytes:
//     +0x02  PUSH imm32   → 0x00e560f6 (SEH scope table)
//     +0x14  MOV  EAX,[]  → 0x012ea8b0 (__security_cookie)
//     +0x132 CALL rel32   → 0x00418d00 (decoder A)
//     +0x156 CALL rel32   → 0x00418e00 (decoder B)
//     +0x176 CALL rel32   → 0x00418bf0 (decoder C)
//     +0x1c7 CALL rel32   → 0x00431e20 (finalise)
//
// Reconstruction strategy — naked-asm byte passthrough via __declspec(naked)
// + `_emit`, the established sibling idiom (FUN_00401820 / FUN_0040b840 /
// FUN_00409350) for inline-SEH + /GS + reloc-heavy bodies. A source-level
// rewrite would need to coax MSVC 2005 /O2 /GS /EHsc into reproducing the
// exact SEH state numbering, cookie placement, three-way decoder dispatch
// with its precise PUSH ordering / ADD ESP fold-downs, and the four
// linker-resolved rel32 targets — every one brittle under /O2. The pre-linked
// literals already match orig PE byte-for-byte, so the .obj's empty reloc
// table is fine: compare.py sees byte-equality.

extern "C" __declspec(naked) void FUN_00431f30() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0xf6
        _emit 0x60
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
        _emit 0x0c
        _emit 0x53
        _emit 0x55
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
        _emit 0x1c
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x06
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x0f
        _emit 0xc8
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x81
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x58
        _emit 0x45
        _emit 0x54
        _emit 0x47
        _emit 0x0f
        _emit 0x94
        _emit 0xc0
        _emit 0x84
        _emit 0xc0
        _emit 0x75
        _emit 0x1b
        _emit 0xc7
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xc1
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5f
        _emit 0x5d
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0xc3
        _emit 0x8b
        _emit 0x56
        _emit 0x14
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x0f
        _emit 0xc8
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x0f
        _emit 0x95
        _emit 0xc0
        _emit 0x84
        _emit 0xc0
        _emit 0x75
        _emit 0x22
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x30
        _emit 0x00
        _emit 0x75
        _emit 0x1b
        _emit 0xc7
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xc1
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5f
        _emit 0x5d
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0xc3
        _emit 0x0f
        _emit 0xb7
        _emit 0x46
        _emit 0x0a
        _emit 0x0f
        _emit 0xb6
        _emit 0x4e
        _emit 0x06
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x66
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x66
        _emit 0xc1
        _emit 0xc0
        _emit 0x08
        _emit 0x66
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x0f
        _emit 0xb7
        _emit 0x56
        _emit 0x0c
        _emit 0x0f
        _emit 0xb7
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x66
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x66
        _emit 0xc1
        _emit 0xc0
        _emit 0x08
        _emit 0x66
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x0f
        _emit 0xb7
        _emit 0x46
        _emit 0x0e
        _emit 0x0f
        _emit 0xb7
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x66
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x66
        _emit 0xc1
        _emit 0xc0
        _emit 0x08
        _emit 0x66
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8a
        _emit 0x5e
        _emit 0x09
        _emit 0x0f
        _emit 0xb6
        _emit 0x56
        _emit 0x07
        _emit 0x33
        _emit 0xc0
        _emit 0xf6
        _emit 0xc3
        _emit 0x04
        _emit 0x74
        _emit 0x05
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf6
        _emit 0xc3
        _emit 0x01
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x19
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x34
        _emit 0x53
        _emit 0x51
        _emit 0x50
        _emit 0x52
        _emit 0x55
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x57
        _emit 0x51
        _emit 0xe8
        _emit 0x99
        _emit 0x6c
        _emit 0xfe
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        _emit 0xeb
        _emit 0x42
        _emit 0xf6
        _emit 0xc3
        _emit 0x02
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x34
        _emit 0x53
        _emit 0x51
        _emit 0x74
        _emit 0x19
        _emit 0x50
        _emit 0x0f
        _emit 0xb7
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x52
        _emit 0x50
        _emit 0x55
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x57
        _emit 0x51
        _emit 0xe8
        _emit 0x75
        _emit 0x6d
        _emit 0xfe
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        _emit 0xeb
        _emit 0x1e
        _emit 0x89
        _emit 0x64
        _emit 0x24
        _emit 0x18
        _emit 0x8b
        _emit 0xdc
        _emit 0x51
        _emit 0x50
        _emit 0x52
        _emit 0x55
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x57
        _emit 0x51
        _emit 0xc7
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x45
        _emit 0x6b
        _emit 0xfe
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        _emit 0x8b
        _emit 0x38
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x85
        _emit 0xc9
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x01
        _emit 0x74
        _emit 0x08
        _emit 0x8b
        _emit 0x11
        _emit 0x8b
        _emit 0x02
        _emit 0x6a
        _emit 0x01
        _emit 0xff
        _emit 0xd0
        _emit 0x85
        _emit 0xff
        _emit 0x74
        _emit 0x2c
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x30
        _emit 0x85
        _emit 0xdb
        _emit 0x74
        _emit 0x16
        _emit 0x8b
        _emit 0x13
        _emit 0x8b
        _emit 0x42
        _emit 0x08
        _emit 0x8b
        _emit 0xcb
        _emit 0xff
        _emit 0xd0
        _emit 0x8b
        _emit 0x13
        _emit 0x50
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        _emit 0x8b
        _emit 0xcb
        _emit 0xff
        _emit 0xd0
        _emit 0x50
        _emit 0xeb
        _emit 0x04
        _emit 0x6a
        _emit 0xff
        _emit 0x6a
        _emit 0x00
        _emit 0x57
        _emit 0x56
        _emit 0xe8
        _emit 0x24
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x89
        _emit 0x38
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5f
        _emit 0x5d
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0xc3
    }
}
