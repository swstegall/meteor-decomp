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
// FUNCTION: ffxivgame 0x00064d50 — _OBJ_bsearch_ locale-free wrapper (36 B / 0x24)
//
// This is the MSVC CRT internal _OBJ_bsearch_ wrapper. It forwards five
// standard bsearch arguments to _OBJ_bsearch_ex_ (FUN_00464ca0) with a
// sixth argument of 0 (no extended flags / null locale).
//
// Calling convention: __cdecl (no frame pointer /Oy, plain RET, caller cleans).
// No callee-saved registers used; no locals; no stack frame.
//
// Asm (36 bytes @ orig RVA 0x00064d50):
//   8b 44 24 14   MOV EAX, [ESP+0x14]   ; load arg5 (compare) → EAX
//   8b 4c 24 10   MOV ECX, [ESP+0x10]   ; load arg4 (width)   → ECX
//   8b 54 24 0c   MOV EDX, [ESP+0x0c]   ; load arg3 (num)     → EDX
//   6a 00         PUSH 0                 ; inner arg6 = 0 (flags)
//   50            PUSH EAX               ; inner arg5 = compare
//   8b 44 24 10   MOV EAX, [ESP+0x10]   ; [ESP+0x10] now = orig [ESP+0x08] = base
//   51            PUSH ECX               ; inner arg4 = width
//   8b 4c 24 10   MOV ECX, [ESP+0x10]   ; [ESP+0x10] now = orig [ESP+0x04] = key
//   52            PUSH EDX               ; inner arg3 = num
//   50            PUSH EAX               ; inner arg2 = base
//   51            PUSH ECX               ; inner arg1 = key
//   e8 XX XX XX XX CALL FUN_00464ca0    ; _OBJ_bsearch_ex_(key,base,num,width,compare,0)
//   83 c4 18      ADD ESP, 0x18          ; clean 6 pushed args (6*4=24=0x18)
//   c3            RET

extern "C" void* __cdecl FUN_00464ca0(const void *key, const void *base,
                                       unsigned int num, unsigned int width,
                                       int (__cdecl *compare)(const void *, const void *),
                                       unsigned int flags);

extern "C" void* __cdecl FUN_00464d50(const void *key, const void *base,
                                       unsigned int num, unsigned int width,
                                       int (__cdecl *compare)(const void *, const void *))
{
    return FUN_00464ca0(key, base, num, width, compare, 0);
}
