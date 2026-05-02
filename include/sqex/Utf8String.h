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
// Sqex::Misc::Utf8String — SE-internal string class with 64-byte SSO.
//
// Layout recovered from `Utf8String::Utf8String` at RVA 0x00047260
// (see src/ffxivgame/sqex/Utf8String.cpp). Public surface kept minimal
// here — only what dependent translation units need to construct,
// destruct, and pass instances by pointer. Internal helpers
// (Reserve, etc.) are intentionally *not* declared in the header so
// they remain TU-local for the matching pipeline.
//
// IMPORTANT: this header is consumed by callers that stack-allocate
// Utf8String. The class size MUST stay at 0x54 (84 B). If you change
// the field layout, also update every dependent caller's frame
// expectation.

#ifndef METEOR_DECOMP_SQEX_UTF8STRING_H
#define METEOR_DECOMP_SQEX_UTF8STRING_H

class Utf8String {
public:
    Utf8String(const char *data, unsigned length);
    ~Utf8String();

private:
    // Internal helper at RVA 0x00047010 — ensures m_data has room for
    // `size` bytes. The ctor calls Reserve(length+1, 1).
    void Reserve(unsigned size, int small_ok);

    char *m_data;             // +0x00 — points to inline buf or heap
    int   m_capacity;         // +0x04 — 0x40 for SSO mode
    int   m_size;             // +0x08 — bytes in use (incl. NUL)
    int   m_field_c;          // +0x0c — flags / encoding bits
    char  m_flag_10;          // +0x10 — set 1 by ctor
    char  m_flag_11;          // +0x11 — set 1 by ctor
    char  m_inline_buf[0x40]; // +0x12..+0x51
};
// sizeof(Utf8String) = 0x54 (84 B) — verified empirically by
// matching PackRead::ProcessChunk's 92-byte (0x5c) local frame.

#endif // METEOR_DECOMP_SQEX_UTF8STRING_H
