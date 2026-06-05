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
// FUNCTION: ffxivgame 0x00448840 — __thiscall constructor for a small
//                                  string/buffer object with an inline
//                                  64-byte backing store (55 bytes).
//
// Layout (inferred from the asm):
//   This (ECX):
//     +0x00  char *_Ptr      (points at inline buffer, i.e. this+0x12)
//     +0x04  int   _Capacity (0x40)
//     +0x08  int   _?        (1)
//     +0x0c  int   _Len      (0)
//     +0x10  char  _flagA    (1)
//     +0x11  char  _flagB    (1)
//     +0x12  char  _Buf[...] (inline; _Buf[0] = 0 → empty string)
//
// Source shape (inferred):
//
//   T *T::ctor(SomeArg arg) {
//       this->_flagA = 1;
//       this->_flagB = 1;
//       this->_field8 = 1;
//       this->_Ptr = this->_Buf;
//       this->_Len = 0;
//       this->_Capacity = 0x40;
//       this->_Buf[0] = 0;
//       FUN_00447770(arg);          // helper taking the single stack arg
//       return this;
//   }
//
// Calling convention: __thiscall (ECX = this; one DWORD stack arg;
// callee cleans 4 via `ret 4`; returns this in EAX).
//
// Naked __asm so the AL-reuse for the two byte stores + dword store, the
// LEA-into-EAX aliasing for the [eax] byte store, and the tail call pin
// to the orig encoding. The REL32 callsite (FUN_00447770) is masked by
// tools/compare.py.

extern "C" void FUN_00447770();

extern "C" __declspec(naked) void FUN_00448840() {
    __asm {
        push    esi
        mov     esi, ecx
        mov     eax, 0x1
        mov     byte ptr [esi + 0x10], al
        mov     byte ptr [esi + 0x11], al
        mov     dword ptr [esi + 0x8], eax
        lea     eax, [esi + 0x12]
        mov     dword ptr [esi], eax
        mov     dword ptr [esi + 0xc], 0x0
        mov     dword ptr [esi + 0x4], 0x40
        mov     byte ptr [eax], 0x0
        mov     eax, dword ptr [esp + 0x8]
        push    eax
        call    FUN_00447770
        mov     eax, esi
        pop     esi
        ret     0x4
    }
}
