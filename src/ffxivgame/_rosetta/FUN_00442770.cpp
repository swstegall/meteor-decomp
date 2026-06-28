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
// FUNCTION: ffxivgame 0x00042770 — __thiscall conditional virtual-dispatch
//                                  with pointer clear, returns bool (55 bytes).
//
// Layout (inferred from asm):
//   This (ECX / ESI):
//     +0x04  void *       ptr          — pointer to some object; zeroed after call
//     +0x08  SubObject    sub          — sub-object passed as 'this' to FUN_00cc3b80
//     +0x24  byte         flag         — if non-zero, suppresses the virtual call
//
//   The object at *ptr has a vtable; slot 5 (offset +0x14) is the method called.
//
// Source shape (inferred):
//
//   bool Outer::method() {
//       FUN_00cc3b80(&this->sub);           // thiscall on sub-object at +0x08
//       void *ptr = this->ptr;              // this+0x04
//       if (ptr != nullptr && this->flag == 0) {
//           // virtual call: vtable[5](this, 0, 0, 0)
//           ((void (__thiscall *)(void *, void *, int, int, int))
//            (*(void ***)ptr)[5])(ptr, this, 0, 0, 0);
//           this->ptr = nullptr;
//       }
//       return this->ptr == nullptr;
//   }
//
// Calling convention: __thiscall (ECX = this; no stack args; callee saves
// ESI; `ret` with no size).
//
// Frame:
//   PUSH ESI       ; callee-save
//   [no ESP adjustment]
//
// Epilogue note: MSVC emits `XOR EAX, EAX` / `CMP [ESI+4], EAX` / `POP ESI`
// / `SETZ AL` / `RET`. The POP ESI is placed between CMP and SETZ — valid
// because POP does not touch EFLAGS.
//
// Naked __asm to pin the exact instruction sequence: the REL32 to
// FUN_00cc3b80 is masked by tools/compare.py.

extern "C" void FUN_00cc3b80();   // __thiscall sub-object method

extern "C" __declspec(naked) void FUN_00442770() {
    __asm {
        push    esi
        mov     esi, ecx
        lea     ecx, [esi + 0x8]
        call    FUN_00cc3b80
        mov     ecx, dword ptr [esi + 0x4]
        test    ecx, ecx
        jz      epilogue
        cmp     byte ptr [esi + 0x24], 0
        jnz     epilogue
        mov     eax, dword ptr [ecx]
        mov     edx, dword ptr [eax + 0x14]
        push    0
        push    0
        push    0
        push    esi
        call    edx
        mov     dword ptr [esi + 0x4], 0
    epilogue:
        xor     eax, eax
        cmp     dword ptr [esi + 0x4], eax
        pop     esi
        setz    al
        ret
    }
}
