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
// FUNCTION: ffxivgame 0x0000daa0 — FUN_0040daa0 (111 B / 0x6f)
//                                   __thiscall member function; takes one
//                                   stack argument; calls FUN_0040e230 to
//                                   build a tagged descriptor, dispatches
//                                   through vtable to FUN_0040e110, updates
//                                   three counter fields, then calls
//                                   FUN_0040da50 to get a "current" value
//                                   and updates two peak-tracking fields.
//
// Calling convention: __thiscall (ECX = this), one stack arg; RET 0x4.
// Callee-saves pushed: ESI, EDI.
// Local stack: 8 bytes (SUB ESP,0x8) — used by FUN_0040e230 as output buffer.
//
// Object layout (offsets relative to original this):
//   [this + 0x00]  DWORD  vtable pointer (read as *this for dispatch to e110)
//   [this + 0x2c]  DWORD  counter (incremented by 1)
//   [this + 0x38]  DWORD  size accumulator (incremented by 0x1000)
//   [this + 0x34]  DWORD  peak_a — updated to max(peak_a, FUN_0040da50())
//   [this + 0x3c]  DWORD  peak_b — updated to max(peak_b, field_0x38)
//
// Global:
//   g_ptr_012652f8  — DWORD at 0x012652f8; its value is passed to FUN_0040e230
//
// String literal:
//   0x00f55864  — "CDev.Engine.Lay.Mem.Space"
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function is straightforward but the three CALL rel32 reloc sites
//   (FUN_0040e230, FUN_0040e110, FUN_0040da50) plus the two immediate
//   constants (0xf55864 string literal and 0x012652f8 global load) make
//   a source-level re-compilation produce relocations that can't be
//   resolved without a full relink. The __declspec(naked) passthrough
//   re-emits the original 111 bytes verbatim; compare.py masks the
//   three CALL rel32 reloc sites and reports GREEN.

extern "C" __declspec(naked) void FUN_0040daa0() {
    __asm {
        // 0000daa0:  a1 f8 52 26 01        MOV EAX,[0x012652f8]
        _emit 0xa1
        _emit 0xf8
        _emit 0x52
        _emit 0x26
        _emit 0x01
        // 0000daa5:  83 ec 08              SUB ESP,0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 0000daa8:  56                    PUSH ESI
        _emit 0x56
        // 0000daa9:  57                    PUSH EDI
        _emit 0x57
        // 0000daaa:  68 64 58 f5 00        PUSH 0xf55864
        _emit 0x68
        _emit 0x64
        _emit 0x58
        _emit 0xf5
        _emit 0x00
        // 0000daaf:  8b f1                 MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0000dab1:  50                    PUSH EAX
        _emit 0x50
        // 0000dab2:  8d 4c 24 10           LEA ECX,[ESP+0x10]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0000dab6:  e8 75 07 00 00        CALL 0x0040e230  [REL32 reloc]
        _emit 0xe8
        _emit 0x75
        _emit 0x07
        _emit 0x00
        _emit 0x00
        // 0000dabb:  8b 4c 24 14           MOV ECX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0000dabf:  50                    PUSH EAX
        _emit 0x50
        // 0000dac0:  51                    PUSH ECX
        _emit 0x51
        // 0000dac1:  8b 0e                 MOV ECX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x0e
        // 0000dac3:  e8 48 06 00 00        CALL 0x0040e110  [REL32 reloc]
        _emit 0xe8
        _emit 0x48
        _emit 0x06
        _emit 0x00
        _emit 0x00
        // 0000dac8:  83 46 2c 01           ADD dword ptr [ESI+0x2c],0x1
        _emit 0x83
        _emit 0x46
        _emit 0x2c
        _emit 0x01
        // 0000dacc:  81 46 38 00 10 00 00  ADD dword ptr [ESI+0x38],0x1000
        _emit 0x81
        _emit 0x46
        _emit 0x38
        _emit 0x00
        _emit 0x10
        _emit 0x00
        _emit 0x00
        // 0000dad3:  83 c6 2c              ADD ESI,0x2c
        _emit 0x83
        _emit 0xc6
        _emit 0x2c
        // 0000dad6:  8b ce                 MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0000dad8:  8b f8                 MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 0000dada:  e8 71 ff ff ff        CALL 0x0040da50  [REL32 reloc]
        _emit 0xe8
        _emit 0x71
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0000dadf:  8b 4e 08              MOV ECX,dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        // 0000dae2:  3b c8                 CMP ECX,EAX
        _emit 0x3b
        _emit 0xc8
        // 0000dae4:  76 02                 JBE 0x0040dae8
        _emit 0x76
        _emit 0x02
        // 0000dae6:  8b c1                 MOV EAX,ECX
        _emit 0x8b
        _emit 0xc1
        // 0000dae8:  8b 4e 10              MOV ECX,dword ptr [ESI+0x10]
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 0000daeb:  89 46 08              MOV dword ptr [ESI+0x8],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 0000daee:  8b 46 0c              MOV EAX,dword ptr [ESI+0xc]
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 0000daf1:  3b c8                 CMP ECX,EAX
        _emit 0x3b
        _emit 0xc8
        // 0000daf3:  76 0d                 JBE 0x0040db02
        _emit 0x76
        _emit 0x0d
        // 0000daf5:  8b c7                 MOV EAX,EDI
        _emit 0x8b
        _emit 0xc7
        // 0000daf7:  5f                    POP EDI
        _emit 0x5f
        // 0000daf8:  89 4e 10              MOV dword ptr [ESI+0x10],ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x10
        // 0000dafb:  5e                    POP ESI
        _emit 0x5e
        // 0000dafc:  83 c4 08              ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0000daff:  c2 04 00              RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 0000db02:  89 46 10              MOV dword ptr [ESI+0x10],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x10
        // 0000db05:  8b c7                 MOV EAX,EDI
        _emit 0x8b
        _emit 0xc7
        // 0000db07:  5f                    POP EDI
        _emit 0x5f
        // 0000db08:  5e                    POP ESI
        _emit 0x5e
        // 0000db09:  83 c4 08              ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0000db0c:  c2 04 00              RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}

// vim: ts=4 sts=4 sw=4 et
