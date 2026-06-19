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
// FUNCTION: ffxivgame 0x00442740 — Component::Sound::SoundCache ctor (35 B)
//
// __thiscall constructor.  Sets the SoundCache vftable pointer and
// zero-initialises seven DWORD fields plus one BYTE field.  The member
// at offset 0x08 (an embedded object) is NOT touched here; its own
// initialisation is performed separately (confirmed by FUN_00442770 which
// calls a function via LEA ECX,[ESI+0x8]).
//
// Calling convention: __thiscall (ECX = this), void return, plain RET.
// Frame: none (/Oy — no locals; no function calls).
//
// MSVC 2005 /O2 /Oy register idiom for many zero-writes:
//   MOV EAX, ECX          ; save `this` to EAX, frees ECX
//   XOR ECX, ECX          ; zero ECX — reused as fill constant (saves 4+ bytes)
//   MOV [EAX],     vftbl  ; vftable ptr (relocated immediate)
//   MOV [EAX+0x04],ECX   ; m_ptr   = 0
//   [EAX+0x08] skipped   ; m_embObj — NOT in initialiser list
//   MOV [EAX+0x0C],ECX   ; m_C     = 0
//   MOV [EAX+0x10],ECX   ; m_10    = 0
//   MOV [EAX+0x14],ECX   ; m_14    = 0
//   MOV [EAX+0x18],ECX   ; m_18    = 0
//   MOV [EAX+0x1C],ECX   ; m_1C    = 0
//   MOV [EAX+0x20],ECX   ; m_20    = 0
//   MOV byte[EAX+0x24],CL ; m_24   = 0
//   RET

class FUN_00442740_SoundCache {
public:
    virtual void fn0();
    virtual void fn1();
    virtual void fn2();
    virtual void fn3();

    int  m_ptr;     // 0x04 — pointer to owner, zeroed in ctor
    int  m_embObj;  // 0x08 — embedded object, NOT initialised here
    int  m_C;       // 0x0C — zeroed
    int  m_10;      // 0x10 — zeroed
    int  m_14;      // 0x14 — zeroed
    int  m_18;      // 0x18 — zeroed
    int  m_1C;      // 0x1C — zeroed
    int  m_20;      // 0x20 — zeroed
    char m_24;      // 0x24 — zeroed

    FUN_00442740_SoundCache();
};

FUN_00442740_SoundCache::FUN_00442740_SoundCache()
    : m_ptr(0)
    , m_C(0)
    , m_10(0)
    , m_14(0)
    , m_18(0)
    , m_1C(0)
    , m_20(0)
    , m_24(0)
{
}
