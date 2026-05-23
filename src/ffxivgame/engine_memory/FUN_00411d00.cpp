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
// FUNCTION: ffxivgame 0x00011d00 — EnterCriticalSection on m_cs at offset 0x5c
//                                   (__thiscall, 11 bytes)
//
// Asm:
//   83 c1 5c                ADD ECX, 0x5c         ; this += 0x5c → &m_cs
//   51                      PUSH ECX              ; arg: lpCriticalSection
//   ff 15 6c e1 f3 00       CALL [EnterCriticalSection]
//   c3                      RET

#include <windows.h>

struct FUN_00411d00_C {
    char pad[0x5c];           // [+0x00..+0x5b]
    CRITICAL_SECTION m_cs;    // [+0x5c]

    void lock();
};

void FUN_00411d00_C::lock()
{
    EnterCriticalSection(&m_cs);
}
