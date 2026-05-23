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
// FUNCTION: ffxivgame 0x000114b0 — __thiscall bool query: inner->field_0x34 != 0
//
// Asm (14 bytes):
//   8b 41 04        MOV EAX, dword ptr [ECX + 0x4]      ; eax = this->inner
//   33 c9           XOR ECX, ECX                         ; ecx = 0 (this is now free)
//   39 48 34        CMP dword ptr [EAX + 0x34], ECX      ; inner->field_0x34 != 0?
//   0f 95 c1        SETNZ CL                             ; cl = (not zero)
//   8a c1           MOV AL, CL                           ; al = bool result
//   c3              RET                                  ; __thiscall, no stack args
//
// Reads this->inner (pointer at offset 4), then tests if inner->field_0x34 is
// non-zero. MSVC recycles ECX (which held `this`) as the zero register for the
// CMP to save one encoding byte vs CMP [mem], imm8.

extern "C" __declspec(naked) void FUN_004114b0()
{
    __asm {
        mov eax, dword ptr [ecx + 0x4]
        xor ecx, ecx
        cmp dword ptr [eax + 0x34], ecx
        setnz cl
        mov al, cl
        ret
    }
}
