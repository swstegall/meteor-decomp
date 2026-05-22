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
// FUNCTION: ffxivgame 0x0000e210 — __thiscall 1-arg struct field init (20 B)
//
// Asm (20 bytes @ orig RVA 0x0000e210):
//   8b c1              MOV  EAX, ECX              ; eax = this
//   8b 4c 24 04        MOV  ECX, [ESP+0x4]        ; ecx = param_1 (ptr)
//   8b 11              MOV  EDX, [ECX]            ; edx = *param_1
//   89 10              MOV  [EAX], EDX            ; this[0] = *param_1
//   c7 40 04 00000000  MOV  dword ptr [EAX+0x4], 0 ; this[4] = 0
//   c2 04 00           RET  0x4                   ; __stdcall pop 1 arg
//
// Calling convention: __thiscall (ECX = this pointer), 1 stack arg, RET 4.
// Sets two consecutive dword fields at offset 0 and 4 in the object:
//   this->field_0x00 = *param_1   (dereferenced copy)
//   this->field_0x04 = 0          (zeroed)
//
// Sibling of FUN_0040e230 (2-arg variant, RET 8, sets both fields from args).
// No prologue — /Oy frame-pointer omission; leaf with no callee-saved
// register usage.

extern "C" __declspec(naked) void FUN_0040e210() {
    __asm {
        mov  eax, ecx
        mov  ecx, dword ptr [esp + 4]
        mov  edx, dword ptr [ecx]
        mov  dword ptr [eax], edx
        mov  dword ptr [eax + 4], 0
        ret  4
    }
}
