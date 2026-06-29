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
// FUNCTION: ffxivgame 0x000487f0 — __thiscall constructor for a small
//                                  string/buffer object, copying from a
//                                  source object with small-buffer-opt branch
//                                  (79 B / 0x4f, ret 4).
//
// Layout of `this` (ECX):
//   +0x00  char *_Ptr      (points at inline buffer, i.e. this+0x12)
//   +0x04  int   _Capacity (initialised to 0x40)
//   +0x08  int   _Field8   (initialised to 1)
//   +0x0c  int   _Field0c  (initialised to 0)
//   +0x10  char  _FlagA    (initialised to 1)
//   +0x11  char  _FlagB    (initialised to 1)
//   +0x12  char  _Buf[...] (inline; _Buf[0] = 0)
//
// The one stack argument (`arg`) is a source object whose layout has:
//   +0x04  void *_DataPtr  — pointer to heap buffer when large
//   +0x18  int   _Size     — element count / capacity indicator
//
// Branch shape (79 bytes @ orig RVA 0x000487f0):
//   +0x2f  JC   +0x0f  → small_case  (arg->_Size < 4: inline at &arg->_DataPtr)
//   large case: push arg->_DataPtr, call FUN_00447770; ret 4
//   small_case: push &arg->_DataPtr (= arg+4), call FUN_00447770; ret 4
//
// Calling convention: __thiscall (ECX = this; one DWORD stack arg;
// callee cleans 4 via `ret 4`; returns this in EAX).
//
// Two REL32 CALL sites (both target FUN_00447770 but from different IPs)
// are masked by tools/compare.py.

extern "C" void FUN_00447770();

extern "C" __declspec(naked) void FUN_004487f0() {
    __asm {
        mov     eax, 0x1
        push    esi
        mov     esi, ecx
        mov     byte ptr [esi + 0x10], al
        mov     byte ptr [esi + 0x11], al
        mov     dword ptr [esi + 0x8], eax
        lea     eax, [esi + 0x12]
        mov     dword ptr [esi], eax
        mov     dword ptr [esi + 0xc], 0x0
        mov     dword ptr [esi + 0x4], 0x40
        mov     byte ptr [eax], 0x0
        mov     eax, dword ptr [esp + 0x8]
        cmp     dword ptr [eax + 0x18], 0x4
        jc      small_case
        mov     eax, dword ptr [eax + 0x4]
        push    eax
        call    FUN_00447770
        mov     eax, esi
        pop     esi
        ret     0x4
    small_case:
        add     eax, 0x4
        push    eax
        call    FUN_00447770
        mov     eax, esi
        pop     esi
        ret     0x4
    }
}
