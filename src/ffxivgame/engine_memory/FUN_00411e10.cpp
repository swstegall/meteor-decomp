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
// FUNCTION: ffxivgame 0x00011e10 — __thiscall, no stack args, 83 bytes
//           Five virtual-dispatch calls through two vtable interfaces.
//
// ESI = this (cached throughout); EDI = this->field_0x10 (cached across all calls).
// Sequence:
//   1. p1 = this->field_0x10; p1->vfunc_0b()          [vtable +0x2c, ECX=p1]
//   2. this->field_0x2c->field_0x34++
//   3. this->field_0x2c->field_0x30->field_0x28->vfunc_07()  [vtable +0x1c]
//   4. this->field_0x30++
//   5. this->field_0x2c->field_0x28->vfunc_07()        [vtable +0x1c]
//   6. p1->vfunc_0c()                                  [vtable +0x30, ECX=p1=EDI]
//   7. r = this->field_0x2c->field_0x28->vfunc_01()   [vtable +0x04]
//      ESI is repurposed to this->field_0x28 before the call
//   8. return r + this->field_0x28
//
// NOTE: Standard C++ cannot reproduce the ESI pre-load at step 7 — MSVC 2005
//       chose ESI (reusing the `this` register) rather than EDI (the dead p1
//       register).  Naked asm matches the exact byte sequence.
//
// Asm:
//   56                   PUSH ESI
//   8b f1                MOV ESI, ECX
//   57                   PUSH EDI
//   8b 7e 10             MOV EDI, [ESI+0x10]
//   8b 07                MOV EAX, [EDI]
//   8b 50 2c             MOV EDX, [EAX+0x2c]
//   8b cf                MOV ECX, EDI
//   ff d2                CALL EDX
//   8b 46 2c             MOV EAX, [ESI+0x2c]
//   83 40 34 01          ADD dword ptr [EAX+0x34], 1
//   8b 40 30             MOV EAX, [EAX+0x30]
//   8b 48 28             MOV ECX, [EAX+0x28]
//   8b 11                MOV EDX, [ECX]
//   8b 42 1c             MOV EAX, [EDX+0x1c]
//   ff d0                CALL EAX
//   83 46 30 01          ADD dword ptr [ESI+0x30], 1
//   8b 4e 2c             MOV ECX, [ESI+0x2c]
//   8b 49 28             MOV ECX, [ECX+0x28]
//   8b 11                MOV EDX, [ECX]
//   8b 42 1c             MOV EAX, [EDX+0x1c]
//   ff d0                CALL EAX
//   8b 17                MOV EDX, [EDI]
//   8b 42 30             MOV EAX, [EDX+0x30]
//   8b cf                MOV ECX, EDI
//   ff d0                CALL EAX
//   8b 4e 2c             MOV ECX, [ESI+0x2c]
//   8b 49 28             MOV ECX, [ECX+0x28]
//   8b 11                MOV EDX, [ECX]
//   8b 42 04             MOV EAX, [EDX+0x4]
//   8b 76 28             MOV ESI, [ESI+0x28]
//   ff d0                CALL EAX
//   5f                   POP EDI
//   03 c6                ADD EAX, ESI
//   5e                   POP ESI
//   c3                   RET

extern "C" __declspec(naked) int FUN_00411e10()
{
    __asm {
        push esi
        mov  esi, ecx
        push edi
        mov  edi, [esi+0x10]
        mov  eax, [edi]
        mov  edx, [eax+0x2c]
        mov  ecx, edi
        call edx
        mov  eax, [esi+0x2c]
        add  dword ptr [eax+0x34], 1
        mov  eax, [eax+0x30]
        mov  ecx, [eax+0x28]
        mov  edx, [ecx]
        mov  eax, [edx+0x1c]
        call eax
        add  dword ptr [esi+0x30], 1
        mov  ecx, [esi+0x2c]
        mov  ecx, [ecx+0x28]
        mov  edx, [ecx]
        mov  eax, [edx+0x1c]
        call eax
        mov  edx, [edi]
        mov  eax, [edx+0x30]
        mov  ecx, edi
        call eax
        mov  ecx, [esi+0x2c]
        mov  ecx, [ecx+0x28]
        mov  edx, [ecx]
        mov  eax, [edx+0x4]
        mov  esi, [esi+0x28]
        call eax
        pop  edi
        add  eax, esi
        pop  esi
        ret
    }
}
