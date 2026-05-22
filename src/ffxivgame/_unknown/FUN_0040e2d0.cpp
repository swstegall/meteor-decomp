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
// FUNCTION: ffxivgame 0x0000e2d0 — __thiscall two-field setter
//                                   stores param_1 at [this+0], param_2 at [this+4]
//                                   (18 B / 0x12)
//
// Calling convention: __thiscall (ECX = this, two DWORD stack args, RET 0x8).
// No prologue / no frame pointer. MSVC loads param_2 into EDX before saving
// ECX → EAX, then reloads param_1 into ECX to avoid a 3-register spill.
//
// Asm (18 bytes @ orig RVA 0x0000e2d0):
//   8b 54 24 08   MOV EDX, dword ptr [ESP + 0x8]   ; EDX = param_2
//   8b c1         MOV EAX, ECX                      ; EAX = this
//   8b 4c 24 04   MOV ECX, dword ptr [ESP + 0x4]   ; ECX = param_1
//   89 08         MOV dword ptr [EAX], ECX          ; this[0] = param_1
//   89 50 04      MOV dword ptr [EAX + 0x4], EDX   ; this[4] = param_2
//   c2 08 00      RET 0x8

extern "C" __declspec(naked) void FUN_0040e2d0() {
    __asm {
        mov edx, dword ptr [esp + 8]
        mov eax, ecx
        mov ecx, dword ptr [esp + 4]
        mov dword ptr [eax], ecx
        mov dword ptr [eax + 4], edx
        ret 8
    }
}
