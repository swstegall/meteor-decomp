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
// FUNCTION: ffxivgame 0x0041ab00 — __thiscall cleanup helper (33 bytes)
//
// Conditionally releases a reference held in this->field_1c via a virtual
// method dispatch on the singleton at [0x01329920], then zeroes the field.
// Appears as a standalone extract of the inner "release field_1c" block
// embedded inside FUN_0041aa70 and similar destructor-style functions.
//
// Calling convention: __thiscall (ECX = this), no extra stack args, void
// return.  Callee-saves: ESI only (PUSH ESI / POP ESI bracket).
// Frame: none (no locals, no security cookie — too small to trigger /GS).
//
// Asm (33 bytes @ orig RVA 0x0001ab00):
//   56                    PUSH ESI
//   8b f1                 MOV  ESI, ECX            ; save this
//   8b 46 1c              MOV  EAX, [ESI+0x1c]     ; field_1c
//   85 c0                 TEST EAX, EAX            ; null check
//   74 15                 JZ   +0x15               ; → POP ESI / RET
//   8b 0d 20 99 32 01     MOV  ECX, [0x01329920]   ; global singleton ptr
//   8b 11                 MOV  EDX, [ECX]          ; vtable
//   50                    PUSH EAX                 ; arg = field_1c
//   8b 42 1c              MOV  EAX, [EDX+0x1c]     ; vtable slot 7
//   ff d0                 CALL EAX                 ; virtual release
//   c7 46 1c 00 00 00 00  MOV  dword ptr [ESI+0x1c], 0
//   5e                    POP  ESI
//   c3                    RET
//
// Relocation site (compare.py masks this 4-byte window):
//   +0x0b  MOV ECX, [0x01329920]   (absolute memory address)

extern "C" void* g_1ab00_singleton;   // [0x01329920]

extern "C" __declspec(naked) void FUN_0041ab00()
{
    __asm {
        push esi
        mov  esi, ecx
        mov  eax, dword ptr [esi + 0x1c]
        test eax, eax
        jz   done
        mov  ecx, dword ptr [g_1ab00_singleton]
        mov  edx, dword ptr [ecx]
        push eax
        mov  eax, dword ptr [edx + 0x1c]
        call eax
        mov  dword ptr [esi + 0x1c], 0
    done:
        pop  esi
        ret
    }
}
