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
// FUNCTION: ffxivgame 0x00010790 — EnterCriticalSection on m_cs at offset 0x2c
//                                   (__thiscall, 11 bytes)
//
// Asm:
//   83 c1 2c                ADD ECX, 0x2c         ; this += 0x2c → &m_cs
//   51                      PUSH ECX              ; arg: lpCriticalSection
//   ff 15 6c e1 f3 00       CALL [EnterCriticalSection]
//   c3                      RET

#include <windows.h>

struct FUN_00410790_C {
    char pad[0x2c];           // [+0x00..+0x2b]
    CRITICAL_SECTION m_cs;    // [+0x2c]

    void lock();
};

void FUN_00410790_C::lock()
{
    EnterCriticalSection(&m_cs);
}
