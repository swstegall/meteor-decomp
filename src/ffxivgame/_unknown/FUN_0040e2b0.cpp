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
// FUNCTION: ffxivgame 0x0000e2b0 — __thiscall setter: stores param_1 into
//                                   this->field_0 and zeroes this->field_4 (18 B)
//
// Calling convention: __thiscall (ECX = this, 1 stack arg via [ESP+4], RET 4).
// No prologue — /Oy omits frame pointer; no locals.
//
// Asm (18 bytes @ orig RVA 0x0000e2b0):
//   8b c1                    MOV EAX, ECX              ; EAX = this
//   8b 4c 24 04              MOV ECX, [ESP + 0x4]      ; ECX = param_1
//   89 08                    MOV dword ptr [EAX], ECX  ; this->field_0 = param_1
//   c7 40 04 00 00 00 00     MOV dword ptr [EAX+4], 0  ; this->field_4 = 0
//   c2 04 00                 RET 0x4                   ; __thiscall: pop 1 arg

extern "C" __declspec(naked) void FUN_0040e2b0() {
    __asm {
        mov eax, ecx
        mov ecx, dword ptr [esp + 4]
        mov dword ptr [eax], ecx
        mov dword ptr [eax + 4], 0
        ret 4
    }
}
