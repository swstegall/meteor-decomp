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
// FUNCTION: ffxivgame 0x00045530 — FUN_00445530 (__thiscall, 24 B)
//
// Frameless __thiscall member function that initialises three fields of
// an object and null-terminates a buffer pointer stored at offset 0.
//
// ECX = this throughout (no explicit stack arguments → bare RET).
//
// Asm (24 bytes):
//   b8 01 00 00 00            MOV  EAX, 0x1
//   88 41 10                  MOV  byte ptr [ECX+0x10], AL   ; field_10 = 1
//   89 41 08                  MOV  dword ptr [ECX+0x8],  EAX ; field_8  = 1
//   8b 01                     MOV  EAX, dword ptr [ECX]      ; p = *this (buf ptr)
//   c7 41 0c 00 00 00 00      MOV  dword ptr [ECX+0xc], 0x0  ; field_c  = 0
//   c6 00 00                  MOV  byte ptr [EAX], 0x0        ; *p = '\0'
//   c3                        RET
//
// MSVC 2005 preloads 1 into EAX once and reuses AL / EAX for the byte
// and dword stores respectively — a standard constant-sharing optimisation.

extern "C" __declspec(naked) void FUN_00445530() {
    __asm {
        mov  eax, 1
        mov  byte ptr [ecx + 0x10], al
        mov  dword ptr [ecx + 0x8], eax
        mov  eax, dword ptr [ecx]
        mov  dword ptr [ecx + 0xc], 0
        mov  byte ptr [eax], 0
        ret
    }
}
