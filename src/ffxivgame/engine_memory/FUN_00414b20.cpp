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
// FUNCTION: ffxivgame 0x00014b20 — LeaveCriticalSection on m_cs at offset 0x08
//                                   (__thiscall, 11 bytes)
//
// Asm:
//   83 c1 08                ADD ECX, 0x08         ; this += 0x08 → &m_cs
//   51                      PUSH ECX              ; arg: lpCriticalSection
//   ff 15 68 e1 f3 00       CALL [LeaveCriticalSection]
//   c3                      RET

#include <windows.h>

struct FUN_00414b20_C {
    char pad[0x08];           // [+0x00..+0x07]
    CRITICAL_SECTION m_cs;    // [+0x08]

    void unlock();
};

void FUN_00414b20_C::unlock()
{
    LeaveCriticalSection(&m_cs);
}
