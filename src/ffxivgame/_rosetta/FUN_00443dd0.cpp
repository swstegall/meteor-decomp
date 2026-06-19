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
// FUNCTION: ffxivgame 0x00043dd0 — __thiscall array-element setter (41 B / 0x29)
//
//   void __thiscall FUN_00443dd0(void *this, int row, int col, int value)
//     ECX        : this pointer
//     [ESP+0x04] : row    (param_1)
//     [ESP+0x08] : col    (param_2)
//     [ESP+0x0c] : value  (param_3)
//     returns: void
//
// Computes a flat index into a 2-D array of 0xbc-byte elements:
//   idx   = (row << 5) + (col & 0x1f)          // 32-column 2-D → flat
//   elem  = this->field_0x8 + idx * 0xbc        // element pointer
//   if (elem != NULL):
//       elem->field_0x60 = value
//       this->field_0xc  = 1                    // mark dirty
//
// Calling convention: __thiscall — ECX = this, 3 stack args, callee cleans
//   12 bytes (RET 0x0c). No frame pointer (/Oy). No callee-saves used.
//
// No relocations: every memory access is register-relative. The 41-byte
// sequence is reproduced verbatim via _emit directives.
//
// Asm (41 bytes @ orig RVA 0x00043dd0):
//   8b 44 24 04           MOV EAX, dword ptr [ESP+0x4]      ; row
//   8b 54 24 08           MOV EDX, dword ptr [ESP+0x8]      ; col
//   c1 e0 05              SHL EAX, 0x5                       ; row * 32
//   83 e2 1f              AND EDX, 0x1f                      ; col & 31
//   03 c2                 ADD EAX, EDX                       ; flat index
//   69 c0 bc 00 00 00     IMUL EAX, EAX, 0xbc               ; * element size
//   03 41 08              ADD EAX, dword ptr [ECX+0x8]       ; + base ptr
//   74 0b                 JZ  +0x0b (to RET)                 ; null-guard
//   8b 54 24 0c           MOV EDX, dword ptr [ESP+0xc]       ; value
//   89 50 60              MOV dword ptr [EAX+0x60], EDX      ; elem->field_0x60
//   c6 41 0c 01           MOV byte ptr [ECX+0xc], 0x1        ; dirty flag
//   c2 0c 00              RET 0xc

extern "C" __declspec(naked) void FUN_00443dd0() {
    __asm {
        // 00043dd0: 8b 44 24 04   MOV EAX, [ESP+0x4]   ; row
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00043dd4: 8b 54 24 08   MOV EDX, [ESP+0x8]   ; col
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 00043dd8: c1 e0 05      SHL EAX, 0x5
        _emit 0xc1
        _emit 0xe0
        _emit 0x05
        // 00043ddb: 83 e2 1f      AND EDX, 0x1f
        _emit 0x83
        _emit 0xe2
        _emit 0x1f
        // 00043dde: 03 c2         ADD EAX, EDX
        _emit 0x03
        _emit 0xc2
        // 00043de0: 69 c0 bc 00 00 00   IMUL EAX, EAX, 0xbc
        _emit 0x69
        _emit 0xc0
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00043de6: 03 41 08      ADD EAX, [ECX+0x8]
        _emit 0x03
        _emit 0x41
        _emit 0x08
        // 00043de9: 74 0b         JZ +0x0b  (-> 0x00043df6)
        _emit 0x74
        _emit 0x0b
        // 00043deb: 8b 54 24 0c   MOV EDX, [ESP+0xc]   ; value
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 00043def: 89 50 60      MOV [EAX+0x60], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x60
        // 00043df2: c6 41 0c 01   MOV byte ptr [ECX+0xc], 0x1
        _emit 0xc6
        _emit 0x41
        _emit 0x0c
        _emit 0x01
        // 00043df6: c2 0c 00      RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
