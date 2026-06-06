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
// FUNCTION: ffxivgame 0x00070c60 — _X509v3_get_ext_count  (__cdecl, 18 B / 0x12)
//
// OpenSSL `X509v3_get_ext_count`: returns 0 if the extension stack is NULL,
// otherwise tail-calls `sk_num` (RVA 0x00064030) to return the element count.
//
// Asm (18 bytes @ 0x00070c60):
//   8b 44 24 04      MOV EAX, dword ptr [ESP+0x4]   ; x = arg1
//   85 c0            TEST EAX, EAX                  ; if x == NULL
//   75 01            JNZ +1                          ; skip RET if non-NULL
//   c3               RET                             ; return 0 (EAX == 0)
//   89 44 24 04      MOV dword ptr [ESP+0x4], EAX    ; re-establish arg for callee
//   e9 be 33 ff ff   JMP 0x00464030                  ; tail call: sk_num(x)
//
// Calling convention: __cdecl (plain RET — caller cleans the single arg).
// No stack frame (compiled with /Oy). No callee-saved registers touched.
//
// The MOV [ESP+4], EAX before the JMP is MSVC 2005's tail-call lowering:
// the compiler re-stores the argument register into the stack slot before
// jumping to the callee, even though EAX already equals [ESP+4] at that
// point (the load at entry made them equal). The callee (sk_num) reads
// [ESP+4] as its own first argument and uses the same return address.
//
// Reloc-bearing site (wildcarded by compare.py):
//   +0x0e  rel32  0x00064030 — sk_num

extern "C" int __cdecl FUN_00464030(const void *st);  // sk_num

extern "C" int __cdecl FUN_00470c60(const void *x)
{
    if (!x)
        return 0;
    return FUN_00464030(x);
}
