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
// FUNCTION: ffxivgame 0x00010430 — SQEX::CDev::Engine::Memory::Alternative::IDebugSpace
//           scalar-deleting destructor with vtable reset (28 B, includes RET 4)
//
// Scalar-deleting destructor for IDebugSpace.  MSVC /O2 hoists the
// `flags & 1` test to before the ESI save, giving the unusual ordering:
//   TEST before PUSH ESI / MOV ESI, ECX
//
// Asm (28 bytes @ RVA 0x00010430):
//   f6 44 24 04 01     TEST byte ptr [ESP+4], 1
//   56                 PUSH ESI
//   8b f1              MOV ESI, ECX
//   c7 06 <rel32>      MOV dword ptr [ESI], offset IDebugSpace_vftable
//   74 09              JZ no_delete
//   56                 PUSH ESI
//   e8 <rel32>         CALL free_func
//   83 c4 04           ADD ESP, 4
//  no_delete:
//   8b c6              MOV EAX, ESI
//   5e                 POP ESI
//   c2 04 00           RET 4

extern "C" int IDebugSpace_vftable;   // SQEX::CDev::Engine::Memory::Alternative::IDebugSpace::vftable
extern "C" void free_func();          // _free / operator delete forwarded to CRT free

extern "C" __declspec(naked) void FUN_00410430() {
    __asm {
        test byte ptr [esp+4], 1
        push esi
        mov  esi, ecx
        mov  dword ptr [esi], offset IDebugSpace_vftable
        jz   no_delete
        push esi
        call free_func
        add  esp, 4
    no_delete:
        mov  eax, esi
        pop  esi
        ret  4
    }
}
