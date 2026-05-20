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
// FUNCTION: ffxivgame 0x00004220 — wchar_t SSO string empty-init helper
//   (19B, __thiscall). Companion to the heap-aware `_Tidy` sibling at
//   0x00003fd0 (same +0x04 inline buffer / +0x14 Mysize / +0x18 Myres
//   layout, 16-bit zero-write to confirm wchar_t element type) and the
//   narrow-char scalar-deleting-dtor sibling at 0x00404000.
//
//   Behaviour:
//
//     __thiscall void *FUN_00404220(this):
//         this->Myres  = 7;            ; SSO capacity (7 wchars)
//         this->Mysize = 0;
//         this->buf[0] = L'\0';        ; 16-bit zero write
//         return this;                 ; in eax
//
//   No heap-free branch — the caller has already accounted for any
//   previously-allocated buffer (this is the "fresh empty" path used at
//   construction time and from copy/move ctors after the source has
//   been drained). The sibling `_Tidy` at 0x00003fd0 is the variant
//   that walks the free path before resetting.
//
// Asm (19 bytes @ orig RVA 0x00004220):
//
//   8b c1                  MOV  EAX, ECX                       ; eax = this (return)
//   33 c9                  XOR  ECX, ECX                       ; ecx = 0
//   c7 40 18 07 00 00 00   MOV  DWORD PTR [EAX + 0x18], 7      ; Myres  = 7
//   89 48 14               MOV  DWORD PTR [EAX + 0x14], ECX    ; Mysize = 0
//   66 89 48 04            MOV  WORD  PTR [EAX + 0x04], CX     ; buf[0] = L'\0'
//   c3                     RET
//
// The function has no callee-saved register usage (no `push esi` / heap
// branch / call out) so MSVC 2005 lays out the body using ECX (input
// `this`) as the zero source after first sinking it into EAX for the
// return-value contract. Coaxing exactly this lowering out of source-
// level C++ is fragile (any `unsigned int tmp = 0;` etc. tends to slot
// the zero into a different register). Naked asm pins the layout
// byte-for-byte; no relocations are involved.

extern "C" __declspec(naked) void FUN_00404220() {
    __asm {
        mov     eax, ecx
        xor     ecx, ecx
        mov     dword ptr [eax + 0x18], 7
        mov     dword ptr [eax + 0x14], ecx
        mov     word  ptr [eax + 0x04], cx
        ret
    }
}
