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
// FUNCTION: ffxivgame 0x00038920 — thiscall wrapper: builds 8-byte stack struct
//                                   and calls FUN_004359d0 (38 B / 0x26)
//
// __thiscall FUN_00438920(this, void *arg1):
//
//   Allocates an 8-byte struct on the stack, initialises it as:
//     struct[+0] = 0x00f64970   (vtable-pointer / type tag — DIR32 reloc)
//     struct[+4] = arg1
//   then calls FUN_004359d0 with ECX = &struct (thiscall "this") and the
//   explicit argument = this->field4.  Return value is passed through from
//   FUN_004359d0 unchanged (EAX not touched after the call).
//
// Calling convention : __thiscall (ECX = this), 1 explicit stack arg.
// Frame             : no EBP frame; SUB ESP,8 allocates the temp struct.
// Return            : passthrough (EAX from FUN_004359d0).
//
// Asm (38 bytes @ orig RVA 0x00038920):
//   83 ec 08                  sub  esp, 8
//   8b 49 04                  mov  ecx, dword ptr [ecx+4]      ; this->field4
//   8b 44 24 0c               mov  eax, dword ptr [esp+0x0c]   ; arg1
//   51                        push ecx                          ; arg to 4359d0
//   8d 4c 24 04               lea  ecx, [esp+4]                ; ecx = &struct
//   c7 44 24 04 70 49 f6 00   mov  dword ptr [esp+4], 0xf64970 ; struct[0]
//   89 44 24 08               mov  dword ptr [esp+8], eax      ; struct[4]=arg1
//   e8 90 d0 ff ff            call FUN_004359d0                 ; (reloc)
//   83 c4 08                  add  esp, 8
//   c2 04 00                  ret  4
//
// Reconstruction: naked-asm byte passthrough.  The only linker-relocated
// field is the 4-byte relative displacement of the CALL to FUN_004359d0;
// compare.py masks those bytes automatically.

extern "C" void FUN_004359d0();   // forward declaration for the CALL relocation

extern "C" __declspec(naked) void FUN_00438920()
{
    __asm {
        sub  esp, 8
        mov  ecx, dword ptr [ecx+4]
        mov  eax, dword ptr [esp+0x0c]
        push ecx
        lea  ecx, [esp+4]
        mov  dword ptr [esp+4], 0x00f64970
        mov  dword ptr [esp+8], eax
        call FUN_004359d0
        add  esp, 8
        ret  4
    }
}
