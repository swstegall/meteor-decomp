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
// FUNCTION: ffxivgame 0x000104d0 — SQEX::CDev::Engine::Memory::Alternative::Link
//           scalar-deleting destructor with doubly-linked-list unlink and vtable reset (46 B)
//
// Scalar-deleting destructor for Link.  MSVC /O2 hoists the `flags & 1`
// test to before the ESI save, giving the unusual ordering:
//   TEST before PUSH ESI / MOV ESI, ECX
//
// Before conditionally freeing, the destructor unlinks `this` from its
// intrusive doubly-linked list:
//   this->next->prev = this->prev   ([ESI+4]+8 = [ESI+8])
//   this->prev->next = this->next   ([ESI+8]+4 = [ESI+4])
//
// The free function at the CALL target is __cdecl (ADD ESP,4 follows the CALL).
//
// Asm (46 bytes @ RVA 0x000104d0):
//   f6 44 24 04 01     TEST byte ptr [ESP+4], 1
//   56                 PUSH ESI
//   8b f1              MOV ESI, ECX
//   8b 46 04           MOV EAX, dword ptr [ESI+4]
//   8b 4e 08           MOV ECX, dword ptr [ESI+8]
//   c7 06 <rel32>      MOV dword ptr [ESI], offset Link_vftable
//   89 48 08           MOV dword ptr [EAX+8], ECX
//   8b 56 08           MOV EDX, dword ptr [ESI+8]
//   8b 46 04           MOV EAX, dword ptr [ESI+4]
//   89 42 04           MOV dword ptr [EDX+4], EAX
//   74 09              JZ no_delete
//   56                 PUSH ESI
//   e8 <rel32>         CALL free_func
//  no_delete:
//   8b c6              MOV EAX, ESI
//   5e                 POP ESI
//   c2 04 00           RET 4

extern "C" int Link_vftable;    // SQEX::CDev::Engine::Memory::Alternative::Link::vftable
extern "C" void free_func();    // _free / operator delete forwarded to CRT free

extern "C" __declspec(naked) void FUN_004104d0() {
    __asm {
        test byte ptr [esp+4], 1
        push esi
        mov  esi, ecx
        mov  eax, dword ptr [esi+4]
        mov  ecx, dword ptr [esi+8]
        mov  dword ptr [esi], offset Link_vftable
        mov  dword ptr [eax+8], ecx
        mov  edx, dword ptr [esi+8]
        mov  eax, dword ptr [esi+4]
        mov  dword ptr [edx+4], eax
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
