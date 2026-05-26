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
// FUNCTION: ffxivgame 0x0000eaf0 — query available physical memory (21 B)
//
// Returns the amount of available physical memory via GlobalMemoryStatus.
// Signature: SIZE_T FUN_0040eaf0(void)
//
// Calling convention: __cdecl — no args, SIZE_T return (EAX), plain RET.
// Frame: SUB ESP, 0x20 — 32-byte local for _MEMORYSTATUS struct.
//        No callee-saved registers pushed.
//
// Asm (21 bytes @ orig RVA 0x0000eaf0):
//   83 ec 20          SUB  ESP, 0x20           ; alloc 32-byte local
//   8d 04 24          LEA  EAX, [ESP]          ; EAX = &ms
//   50                PUSH EAX                 ; push &ms (GlobalMemoryStatus arg)
//   ff 15 60 e1 f3 00 CALL dword ptr [0xf3e160]; GlobalMemoryStatus (stdcall, pops arg)
//   8b 44 24 0c       MOV  EAX, [ESP+0xc]      ; EAX = ms.dwAvailPhys (offset 0xc)
//   83 c4 20          ADD  ESP, 0x20           ; restore frame
//   c3                RET
//
// _MEMORYSTATUS layout (winbase.h):
//   +0x00  DWORD dwLength
//   +0x04  DWORD dwMemoryLoad
//   +0x08  SIZE_T dwTotalPhys
//   +0x0c  SIZE_T dwAvailPhys
//   +0x10  SIZE_T dwTotalPageFile
//   +0x14  SIZE_T dwAvailPageFile
//   +0x18  SIZE_T dwTotalVirtual
//   +0x1c  SIZE_T dwAvailVirtual
//   (total: 32 bytes = 0x20)
//
// After SUB ESP,0x20 → LEA EAX,[ESP] takes address of ms at ESP.
// PUSH EAX moves ESP-4; GlobalMemoryStatus is __stdcall so RET 4 pops arg.
// After call returns, ESP is back to after SUB, so [ESP+0xc] = ms.dwAvailPhys.

#include <windows.h>

extern "C" SIZE_T FUN_0040eaf0()
{
    _MEMORYSTATUS ms;
    GlobalMemoryStatus(&ms);
    return ms.dwAvailPhys;
}
