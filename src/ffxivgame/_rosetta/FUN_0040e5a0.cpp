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
// FUNCTION: ffxivgame 0x0040e5a0 — teardown helper (35 B)
//
// Saves a byte flag from param_1+0xc, calls DeleteCriticalSection on the
// CRITICAL_SECTION at param_1+0x24, then — if the flag was nonzero —
// dispatches a __thiscall to FUN_0040df70 with this=*(param_1-4) and
// first arg=param_1.
//
// Calling convention: __cdecl (plain RET, caller cleans).
// Frame: none (/Oy — only callee-saves EBX+ESI used, no locals on stack).
//
// Register allocation (MSVC 2005 /O2 /Oy):
//   ESI = param_1   (kept live across DeleteCriticalSection)
//   BL  = flag      (byte from [ESI+0xc], kept across the API call)
//
// Asm (35 bytes @ orig RVA 0x0040e5a0):
//   53                   PUSH EBX
//   56                   PUSH ESI
//   8b 74 24 0c          MOV ESI, [ESP+0xc]        ; param_1
//   8a 5e 0c             MOV BL, byte ptr [ESI+0xc] ; flag
//   8d 46 24             LEA EAX, [ESI+0x24]        ; &cs
//   50                   PUSH EAX
//   ff 15 70 e1 f3 00    CALL [IAT:DeleteCriticalSection]
//   84 db                TEST BL, BL
//   74 09                JZ +9  → epilogue
//   8b 4e fc             MOV ECX, [ESI-4]           ; this = *(param_1-4)
//   56                   PUSH ESI                   ; arg = param_1
//   e8 b0 f9 ff ff       CALL FUN_0040df70 (__thiscall, RET 4)
//   5e                   POP ESI
//   5b                   POP EBX
//   c3                   RET

#include <windows.h>

struct FUN_0040df70_owner {
    void FUN_0040df70(void* param);
};

extern "C" void __cdecl FUN_0040e5a0(void* param_1)
{
    char flag = ((char*)param_1)[0x0c];
    DeleteCriticalSection((LPCRITICAL_SECTION)((char*)param_1 + 0x24));
    if (flag != '\0') {
        (*(FUN_0040df70_owner**)((char*)param_1 - 4))->FUN_0040df70(param_1);
    }
}
