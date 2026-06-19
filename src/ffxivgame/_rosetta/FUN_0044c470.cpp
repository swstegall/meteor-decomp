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
// FUNCTION: ffxivgame 0x0044c470 — __thiscall accessor: fetch a base pointer
//                                   from the object's first virtual slot, then
//                                   index it as a 12-byte-element array
//                                   (20 B / 0x14).
//
// Behaviour read from the disassembly at orig RVA 0x0004c470:
//
//   This is a template sibling of FUN_0044c440 (FileData / 4-byte stride) and
//   FUN_0044c4a0 (char / 1-byte stride) — all three live in the
//   Sqex::File::ResourceSafePath::IHandledPtr<T>::operator[](int) family.
//   This instantiation is for T = Sqex::File::ResourceSafePath::FolderData,
//   whose sizeof is 12 bytes, giving the stride-12 LEA chain.
//
//   __thiscall FolderData* FUN_0044c470(C* this, int index) {
//       char* base = (char*)this->vtbl[1]();  // virtual call, slot at vtbl+0x4
//       return (FolderData*)(base + index * 12);  // EAX + index*12
//   }
//
//   Calling convention: __thiscall (ECX = this, one 4-byte stack arg `index`,
//   callee pops via `RET 4`). The virtual callee at vtbl+0x4 is invoked with
//   ECX still holding `this` and returns the array base in EAX; the index is
//   loaded from [ESP+4] only after the call returns.
//
//   Asm shape (20 bytes total):
//
//     8b 01              mov  eax, [ecx]          ; eax = this->vtbl
//     8b 50 04           mov  edx, [eax+4]        ; edx = vtbl[1]
//     ff d2              call edx                 ; __thiscall virtual call
//     8b 4c 24 04        mov  ecx, [esp+4]        ; ecx = index
//     8d 0c 49           lea  ecx, [ecx+ecx*2]    ; ecx = index*3
//     8d 04 88           lea  eax, [eax+ecx*4]    ; eax = base + index*12
//     c2 04 00           ret  4                   ; __thiscall epilogue
//
//   No reloc-bearing sites (the CALL targets a register, not a rel32), so the
//   naked body below re-emits the orig 20 bytes verbatim.

extern "C" __declspec(naked) void FUN_0044c470() {
    __asm {
        mov  eax, dword ptr [ecx]
        mov  edx, dword ptr [eax + 0x4]
        call edx
        mov  ecx, dword ptr [esp + 0x4]
        lea  ecx, [ecx + ecx*2]
        lea  eax, [eax + ecx*4]
        ret  0x4
    }
}
