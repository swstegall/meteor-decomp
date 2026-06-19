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
// FUNCTION: ffxivgame 0x00449390 — UTF-8 back-up-one-code-point (46 B / 0x2E)
//
//   void __stdcall FUN_00449390(char **pStr, int *pCount)
//
//   Decrements *pStr by 1, then backs up past any UTF-8 continuation bytes
//   (byte & 0xC0 == 0x80, i.e. bytes in range 0x80..0xBF), leaving *pStr
//   pointing at the leading byte of the previous code point.  Then
//   decrements *pCount by 1.
//
//   Calling convention: __stdcall (callee cleans 8 bytes via RET 8).
//   Frame: none — no register saves, no ESP adjustment.
//   No relocations: all branches are short relative jumps, no global refs.
//
// Asm (46 bytes @ orig RVA 0x00049390):
//   8b 44 24 04        MOV  EAX, [ESP+4]       ; EAX = pStr
//   83 00 ff           ADD  [EAX], -1           ; --(*pStr)
//   8b 08              MOV  ECX, [EAX]          ; ECX = *pStr (new pos)
//   8a 11              MOV  DL,  [ECX]          ; DL  = *(*pStr)
//   80 e2 c0           AND  DL,  0xc0
//   80 fa 80           CMP  DL,  0x80
//   75 11              JNZ  after_back          ; not a cont. byte → skip loop
// back_loop:
//   83 e9 01           SUB  ECX, 1              ; back up one more byte
//   8b d1              MOV  EDX, ECX
//   89 08              MOV  [EAX], ECX          ; *pStr = ECX
//   8a 12              MOV  DL,  [EDX]          ; DL  = *ECX
//   80 e2 c0           AND  DL,  0xc0
//   80 fa 80           CMP  DL,  0x80
//   74 ef              JZ   back_loop           ; still cont. byte → keep going
// after_back:
//   8b 44 24 08        MOV  EAX, [ESP+8]        ; EAX = pCount
//   83 00 ff           ADD  [EAX], -1           ; --(*pCount)
//   c2 08 00           RET  8

extern "C" __declspec(naked) void FUN_00449390() {
    __asm {
        mov  eax, dword ptr [esp + 4]
        add  dword ptr [eax], -1
        mov  ecx, dword ptr [eax]
        mov  dl, byte ptr [ecx]
        and  dl, 0xc0
        cmp  dl, 0x80
        jnz  after_back
    back_loop:
        sub  ecx, 1
        mov  edx, ecx
        mov  dword ptr [eax], ecx
        mov  dl, byte ptr [edx]
        and  dl, 0xc0
        cmp  dl, 0x80
        jz   back_loop
    after_back:
        mov  eax, dword ptr [esp + 8]
        add  dword ptr [eax], -1
        ret  8
    }
}
