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
// FUNCTION: ffxivgame 0x00043e40 — 2D-table slot lookup with null fallback
//                                   (__thiscall, this + 3 args, 40 B / 0x28)
//
// int __thiscall FUN_00443e40(void *this, int row, int col, int fallback)
//
// Computes a row-major index `(row << 5) + (col & 0x1f)` (32 columns per
// row), scales by the 0xBC-byte element stride, and adds the table base
// pointer at this->field_0x8. If the resulting element pointer is null,
// returns the `fallback` arg (3rd stack arg); otherwise returns the DWORD
// at element + 0x60.
//
// Frame: none (/Oy). Args read straight off [ESP+4..+0xc] before any push;
// the JZ keys off the ZF that the `ADD EAX,[ECX+8]` leaves behind, so the
// null test and the base-add fuse into one instruction. Both arms clean
// the 12 bytes of stack args with `RET 0xc`.
//
// No relocations — the byte sequence is fully fixed, so this is encoded
// as a __declspec(naked) verbatim passthrough.
//
// Asm (40 bytes @ orig RVA 0x00043e40):
//   8b 44 24 04           MOV EAX, [ESP+0x4]        ; row
//   8b 54 24 08           MOV EDX, [ESP+0x8]        ; col
//   c1 e0 05              SHL EAX, 0x5              ; row << 5
//   83 e2 1f              AND EDX, 0x1f             ; col & 0x1f
//   03 c2                 ADD EAX, EDX             ; index
//   69 c0 bc 00 00 00     IMUL EAX, EAX, 0xbc      ; * stride
//   03 41 08              ADD EAX, [ECX+0x8]        ; + this->field_8
//   74 06                 JZ  0x00443e61            ; null → fallback
//   8b 40 60              MOV EAX, [EAX+0x60]
//   c2 0c 00              RET 0xc
//   8b 44 24 0c           MOV EAX, [ESP+0xc]        ; fallback
//   c2 0c 00              RET 0xc

extern "C" __declspec(naked) void FUN_00443e40() {
    __asm {
        // 00043e40: 8b 44 24 04   MOV EAX, [ESP+0x4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00043e44: 8b 54 24 08   MOV EDX, [ESP+0x8]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 00043e48: c1 e0 05      SHL EAX, 0x5
        _emit 0xc1
        _emit 0xe0
        _emit 0x05
        // 00043e4b: 83 e2 1f      AND EDX, 0x1f
        _emit 0x83
        _emit 0xe2
        _emit 0x1f
        // 00043e4e: 03 c2         ADD EAX, EDX
        _emit 0x03
        _emit 0xc2
        // 00043e50: 69 c0 bc 00 00 00  IMUL EAX, EAX, 0xbc
        _emit 0x69
        _emit 0xc0
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00043e56: 03 41 08      ADD EAX, [ECX+0x8]
        _emit 0x03
        _emit 0x41
        _emit 0x08
        // 00043e59: 74 06         JZ 0x00443e61
        _emit 0x74
        _emit 0x06
        // 00043e5b: 8b 40 60      MOV EAX, [EAX+0x60]
        _emit 0x8b
        _emit 0x40
        _emit 0x60
        // 00043e5e: c2 0c 00      RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00043e61: 8b 44 24 0c   MOV EAX, [ESP+0xc]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00043e65: c2 0c 00      RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
