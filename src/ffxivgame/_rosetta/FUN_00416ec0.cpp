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
// FUNCTION: ffxivgame 0x00016ec0 — field-sum * count + header size
//                                   (__thiscall, no args, int return, 35 B)
//
// Returns 0 if this->field_0x8 (count/length) is zero.
// Otherwise returns (byte_0x16 + byte_0x15 + byte_0x14) * field_0x8 + 0x18.
//
// Calling convention: __thiscall — ECX = this, plain RET (no stack cleanup).
// Frame: none (/Oy). ESI is a deferred callee-save: pushed just before first use.
//
// MSVC 2005 /O2 register allocation:
//   EDX = field_0x8   (loaded first for the zero-test; not callee-saved)
//   EAX = field_0x16  (first term; loaded before PUSH ESI)
//   ESI = field_0x15  (second term; ESI pushed just before this load)
//   ECX = field_0x14  (third term; last ECX dereference before this is clobbered)
//
// Asm (35 bytes @ orig RVA 0x00016ec0):
//   8b 51 08      MOV EDX, dword ptr [ECX+0x8]       ; load field_0x8
//   85 d2         TEST EDX, EDX
//   75 03         JNZ +3                             ; nonzero -> compute
//   33 c0         XOR EAX, EAX                       ; return 0
//   c3            RET
//   0f b6 41 16   MOVZX EAX, byte ptr [ECX+0x16]
//   56            PUSH ESI                           ; deferred callee-save
//   0f b6 71 15   MOVZX ESI, byte ptr [ECX+0x15]
//   0f b6 49 14   MOVZX ECX, byte ptr [ECX+0x14]    ; overwrites 'this'
//   03 c6         ADD EAX, ESI
//   03 c1         ADD EAX, ECX
//   0f af c2      IMUL EAX, EDX
//   83 c0 18      ADD EAX, 0x18
//   5e            POP ESI
//   c3            RET

struct FUN_00416ec0_class {
    char           pad_0[8];        // 0x00..0x07
    int            field_0x8;       // 0x08
    char           pad_1[8];        // 0x0C..0x13
    unsigned char  field_0x14;      // 0x14
    unsigned char  field_0x15;      // 0x15
    unsigned char  field_0x16;      // 0x16

    int FUN_00416ec0() const;
};

int FUN_00416ec0_class::FUN_00416ec0() const
{
    if (field_0x8 == 0)
        return 0;
    return (field_0x16 + field_0x15 + field_0x14) * field_0x8 + 0x18;
}
