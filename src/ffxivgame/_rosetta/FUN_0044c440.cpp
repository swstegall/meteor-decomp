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
// FUNCTION: ffxivgame 0x0044c440 — __thiscall accessor: fetch a base pointer
//                                   from the object's first virtual slot, then
//                                   index it as a 4-byte-element array
//                                   (17 B / 0x11).
//
// Behaviour read from the disassembly at orig RVA 0x0004c440:
//
//   __thiscall int* FUN_0044c440(C* this, int index) {
//       int* base = this->vtbl[1]();   // virtual call, slot at vtbl+0x4
//       return base + index;           // EAX + index*4 (4-byte elements)
//   }
//
//   Calling convention: __thiscall (ECX = this, one 4-byte stack arg `index`,
//   callee pops via `RET 4`). The virtual callee at vtbl+0x4 is invoked with
//   ECX still holding `this` and returns the array base in EAX; the index is
//   loaded from [ESP+4] only after the call returns.
//
//   Asm shape (17 bytes total):
//
//     8b 01              mov  eax, [ecx]          ; eax = this->vtbl
//     8b 50 04           mov  edx, [eax+4]        ; edx = vtbl[1]
//     ff d2              call edx                 ; __thiscall virtual call
//     8b 4c 24 04        mov  ecx, [esp+4]        ; ecx = index
//     8d 04 88           lea  eax, [eax+ecx*4]    ; eax = base + index*4
//     c2 04 00           ret  4                   ; __thiscall epilogue
//
//   No reloc-bearing sites (the CALL targets a register, not a rel32), so the
//   naked body below re-emits the orig 17 bytes verbatim.

extern "C" __declspec(naked) void FUN_0044c440() {
    __asm {
        mov  eax, dword ptr [ecx]
        mov  edx, dword ptr [eax + 0x4]
        call edx
        mov  ecx, dword ptr [esp + 0x4]
        lea  eax, [eax + ecx*4]
        ret  0x4
    }
}
