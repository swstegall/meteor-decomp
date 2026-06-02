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
// FUNCTION: ffxivgame 0x413d80 — member-vtable tail-jmp via EDX (member offset 0x8, slot 0x20)
//
// Asm: 8b 49 08 8b 01 8b 50 20 ff e2
//
//   8b 49 08   mov  ecx, [ecx + 0x8]   ; load member pointer at this+0x8
//   8b 01      mov  eax, [ecx]          ; load vtable pointer
//   8b 50 20   mov  edx, [eax + 0x20]   ; load vtable slot 0x20
//   ff e2      jmp  edx                  ; tail-call forwarded method

extern "C" __declspec(naked) void member_vtable_tailjmp_edx() {
    __asm {
        mov ecx, [ecx + 0x8]
        mov eax, [ecx]
        mov edx, [eax + 0x20]
        jmp edx
    }
}
