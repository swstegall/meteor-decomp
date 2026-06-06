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
// FUNCTION: ffxivgame 0x0041a6c0 — __thiscall member function that calls
//           a helper via a global object, then atomically reads two
//           counters from a sub-object: stores the first in an out-param
//           and returns 0 or 1 depending on whether the second is non-zero.
//           (55 bytes / 0x37).
//
// Layout (inferred from asm):
//   This (ECX):
//     +0x04  SubObj *  sub           pointer to a sub-object
//   SubObj:
//     +0x04  LONG *    pCount1       pointer to first atomic counter
//     +0x0c  LONG *    pCount2       pointer to second atomic counter
//
// Source shape:
//
//   int __thiscall FUN_0041a6c0(void *param1) {
//       SubObj *sub = this->sub;
//       g_0132987c->SomeMethod(sub);         // FUN_004233a0
//       sub = this->sub;                     // reload
//       LONG c1;
//       // atomic read via lock xadd with 0
//       xor ecx, ecx; lock xadd [sub->pCount1], ecx  → c1 = *sub->pCount1
//       *(LONG*)param1 = c1;
//       LONG c2;
//       xor eax, eax; lock xadd [sub->pCount2], eax  → c2 = *sub->pCount2
//       // normalize to 0/1: neg; sbb eax,eax; neg
//       return (c2 != 0) ? 1 : 0;
//   }
//
// Calling convention: __thiscall (ECX = this; one DWORD stack arg [ESP+4];
// callee cleans 4 bytes via `ret 4`).
//
// Frame:
//   PUSH ESI                                ; callee-save
//   [no ESP adjustment]
//
// Relocatable sites masked by tools/compare.py:
//   +0x08  MOV ECX, dword ptr [0x0132987c]  absolute data VA (global obj ptr)
//   +0x0e  CALL rel32 → FUN_004233a0        relative branch
//
// Naked __asm so lock-xadd encoding and neg/sbb/neg normaliser
// pin to the orig bytes exactly.

extern "C" int DAT_0132987c;           // global at VA 0x0132987c (data)
extern "C" void FUN_004233a0(void);    // helper called with sub-obj as arg

extern "C" __declspec(naked) void FUN_0041a6c0() {
    __asm {
        push    esi
        mov     esi, ecx
        mov     eax, dword ptr [esi + 4]
        mov     ecx, dword ptr [DAT_0132987c]
        push    eax
        call    FUN_004233a0
        mov     esi, dword ptr [esi + 4]
        mov     edx, dword ptr [esi + 4]
        xor     ecx, ecx
        _emit   0xf0        // LOCK
        _emit   0x0f        // XADD
        _emit   0xc1        //
        _emit   0x0a        //   [EDX], ECX
        mov     eax, dword ptr [esp + 8]
        mov     dword ptr [eax], ecx
        mov     ecx, dword ptr [esi + 0xc]
        xor     eax, eax
        _emit   0xf0        // LOCK
        _emit   0x0f        // XADD
        _emit   0xc1        //
        _emit   0x01        //   [ECX], EAX
        neg     eax
        sbb     eax, eax
        neg     eax
        pop     esi
        ret     4
    }
}
