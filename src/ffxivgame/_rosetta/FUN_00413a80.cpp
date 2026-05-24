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
// FUNCTION: ffxivgame 0x413a80 — member-vtable tail-jmp via EDX (member offset 0x18, slot 0x8)
//
// Asm: 8b 49 18 8b 01 8b 50 08 ff e2
//
//   8b 49 18   mov  ecx, [ecx + 0x18]   ; load member pointer at this+0x18
//   8b 01      mov  eax, [ecx]          ; load vtable pointer
//   8b 50 08   mov  edx, [eax + 0x8]    ; load vtable slot 0x8
//   ff e2      jmp  edx                  ; tail-call forwarded method

extern "C" __declspec(naked) void member_vtable_tailjmp_edx() {
    __asm {
        mov ecx, [ecx + 0x18]
        mov eax, [ecx]
        mov edx, [eax + 0x8]
        jmp edx
    }
}
