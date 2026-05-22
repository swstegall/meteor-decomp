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
// FUNCTION: ffxivgame 0x0000e7f0 — query total virtual memory (21 B)
//
// Allocates a MEMORYSTATUS struct on the stack, calls GlobalMemoryStatus,
// and returns the dwTotalVirtual field.
//
// Calling convention: __cdecl — no args, SIZE_T return, plain RET.
// Frame: SUB ESP,0x20 for the 32-byte MEMORYSTATUS struct; /Oy frame-ptr omitted.
//
// Asm (21 bytes @ orig RVA 0x0000e7f0):
//   83 ec 20              SUB  ESP, 0x20             ; allocate MEMORYSTATUS (32 B)
//   8d 04 24              LEA  EAX, [ESP]             ; &ms
//   50                    PUSH EAX                    ; arg = &ms
//   ff 15 60 e1 f3 00     CALL dword ptr [0x00f3e160] ; GlobalMemoryStatus (IAT)
//   8b 44 24 18           MOV  EAX, [ESP+0x18]        ; ms.dwTotalVirtual
//   83 c4 20              ADD  ESP, 0x20              ; restore stack
//   c3                    RET

#include <windows.h>

SIZE_T FUN_0040e7f0()
{
    MEMORYSTATUS ms;
    GlobalMemoryStatus(&ms);
    return ms.dwTotalVirtual;
}
