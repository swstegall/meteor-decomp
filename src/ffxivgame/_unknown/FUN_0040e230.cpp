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
// FUNCTION: ffxivgame 0x0000e230 — __thiscall 2-arg struct field setter (20 B)
//
// Asm (20 bytes @ orig RVA 0x0000e230):
//   8b c1           MOV  EAX, ECX              ; eax = this
//   8b 4c 24 04     MOV  ECX, [ESP+0x4]        ; ecx = param_1 (ptr)
//   8b 11           MOV  EDX, [ECX]            ; edx = *param_1
//   8b 4c 24 08     MOV  ECX, [ESP+0x8]        ; ecx = param_2
//   89 10           MOV  [EAX], EDX            ; this[0] = *param_1
//   89 48 04        MOV  [EAX+0x4], ECX        ; this[4] = param_2
//   c2 08 00        RET  0x8                   ; __stdcall pop 2 args
//
// Calling convention: __thiscall (ECX = this pointer), 2 stack args, RET 8.
// Sets two consecutive dword fields at offset 0 and 4 in the object:
//   this->field_0x00 = *param_1   (dereferenced copy)
//   this->field_0x04 = param_2    (direct value)
//
// No prologue — /Oy frame-pointer omission; leaf with no callee-saved
// register usage.

extern "C" __declspec(naked) void FUN_0040e230() {
    __asm {
        mov  eax, ecx
        mov  ecx, dword ptr [esp + 4]
        mov  edx, dword ptr [ecx]
        mov  ecx, dword ptr [esp + 8]
        mov  dword ptr [eax], edx
        mov  dword ptr [eax + 4], ecx
        ret  8
    }
}
