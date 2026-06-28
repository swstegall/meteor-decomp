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
// FUNCTION: ffxivgame 0x0003f460 — __thiscall member init that zeroes a
//           string-like SSO structure embedded in a larger class (18 B / 0x12).
//
// Asm (18 bytes @ RVA 0x0003f460):
//   8b c1                   MOV EAX, ECX            ; save `this` to EAX
//   33 c9                   XOR ECX, ECX            ; zero ECX for zero-stores
//   c7 40 18 0f 00 00 00    MOV [EAX + 0x18], 0xf   ; reserved/capacity = 15
//   89 48 14                MOV [EAX + 0x14], ECX   ; size = 0
//   88 48 04                MOV byte [EAX + 0x4], CL ; buf[0] = '\0'
//   c3                      RET
//
// Convention: __thiscall (ECX = this, plain RET — no stack args, no frame).
//
// The three writes match the layout of an SSO string member sitting at offset
// 0x04 within some larger class:
//   this + 0x04  — first byte of the SSO buffer (null-terminated)
//   this + 0x14  — string length (_Mysize = 0)
//   this + 0x18  — reserved capacity (_Myres = 15 = 0xf, the SSO limit)
//
// MSVC 2005 /O2 picks the register-zero strategy here: moving `this` from
// ECX to EAX frees ECX to be zeroed via XOR, letting both zero-stores use a
// 3-byte register-form MOV rather than the 7-byte / 4-byte immediate-zero
// forms.  The non-zero store (0xf at +0x18) must use an immediate regardless.
// Source assignment order — 0x18 first, 0x14 second, 0x04 third — preserves
// the store sequence seen in the original.
//
// No relocation-bearing sites; pure C++ reproduction is byte-identical.

struct C {
    char m_pad0[4];   // 0x00 — fields not touched by this function
    char m_f4;        // 0x04 — SSO buffer first byte / null terminator
    char m_pad5[15];  // 0x05-0x13
    int  m_f14;       // 0x14 — string size
    int  m_f18;       // 0x18 — string capacity / reserved

    void FUN_0043f460();
};

void C::FUN_0043f460()
{
    m_f18 = 0xf;
    m_f14 = 0;
    m_f4  = 0;
}
