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
// FUNCTION: ffxivgame 0x00064310 — FUN_00464310 (__cdecl, 26 B)
//
// Writes two zero bytes into the buffer pointed to by *ppBuf, advances
// the stored pointer by two, and returns 2 (the number of bytes emitted).
// Classic byte-stream serialisation helper: caller holds a `unsigned char*`
// cursor in *ppBuf; each "write-N-bytes" routine bumps it by N so the
// next call continues where this one left off.
//
// Calling convention: __cdecl (bare RET — caller cleans the single arg).
// Stack frame: none (leaf function; ECX / EAX are the only temporaries,
// both allocated as registers — no prologue / epilogue).
//
// Asm (26 bytes @ RVA 0x00064310):
//   8b 4c 24 04    MOV ECX, dword ptr [ESP+0x4]   ; ECX = ppBuf (arg1)
//   8b 01          MOV EAX, dword ptr [ECX]        ; EAX = *ppBuf (current buf ptr)
//   c6 00 00       MOV byte ptr [EAX], 0x0         ; *p = 0  (first zero byte)
//   83 c0 01       ADD EAX, 0x1                    ; p++ (post-increment)
//   c6 00 00       MOV byte ptr [EAX], 0x0         ; *p = 0  (second zero byte)
//   83 c0 01       ADD EAX, 0x1                    ; p++ (post-increment)
//   89 01          MOV dword ptr [ECX], EAX        ; *ppBuf = p (store advanced ptr)
//   b8 02 00 00 00 MOV EAX, 0x2                    ; return 2
//   c3             RET

int FUN_00464310(unsigned char **ppBuf)
{
    unsigned char *p = *ppBuf;
    *p++ = 0;
    *p++ = 0;
    *ppBuf = p;
    return 2;
}
