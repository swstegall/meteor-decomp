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
// FUNCTION: ffxivgame 0x000640c0 — __cdecl single-pointer-arg helper that
//                                  returns 1 when the pointer is null, or
//                                  the int value at byte offset +8 otherwise
//                                  (18 B / 0x12).
//
// Asm (18 bytes @ 0x000640c0):
//   8b 44 24 04         MOV  EAX, dword ptr [ESP + 0x4]   ; load p
//   85 c0               TEST EAX, EAX                     ; null check
//   75 06               JNZ  0x004640ce                   ; if p != 0, skip
//   b8 01 00 00 00      MOV  EAX, 0x1                     ; return 1 (null case)
//   c3                  RET
//   8b 40 08            MOV  EAX, dword ptr [EAX + 0x8]   ; return p[2]
//   c3                  RET
//
// Calling convention: __cdecl (plain RET, caller cleans stack).
// One pointer argument loaded from [ESP+4] into EAX.
// Return type: int.
//
// Logic:
//   if (p == nullptr) return 1;
//   return p[2];   // byte offset 8 from base of *p

extern "C" int __cdecl FUN_004640c0(int* p)
{
    if (!p)
        return 1;
    return p[2];
}
