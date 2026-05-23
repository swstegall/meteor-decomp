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
// FUNCTION: ffxivgame 0x000129c0 — EnterCriticalSection on m_cs at offset 0x38
//                                   (__thiscall, 11 bytes)
//
// Asm:
//   83 c1 38                ADD ECX, 0x38         ; this += 0x38 → &m_cs
//   51                      PUSH ECX              ; arg: lpCriticalSection
//   ff 15 6c e1 f3 00       CALL [EnterCriticalSection]
//   c3                      RET

#include <windows.h>

struct FUN_004129c0_C {
    char pad[0x38];           // [+0x00..+0x37]
    CRITICAL_SECTION m_cs;    // [+0x38]

    void lock();
};

void FUN_004129c0_C::lock()
{
    EnterCriticalSection(&m_cs);
}
