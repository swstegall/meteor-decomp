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
// FUNCTION: ffxivgame 0x0003f810 — __thiscall 1-arg wrapper forwarding to
//                                  FUN_0043f4b0 with (arg, 0, -1) and
//                                  returning `this` (23 B / 0x17).
//
// Asm shape (23 bytes @ orig RVA 0x0003f810):
//
//   8b 44 24 04          MOV  EAX, dword ptr [ESP+0x4]   ; arg
//   56                   PUSH ESI
//   6a ff                PUSH -1
//   6a 00                PUSH 0
//   50                   PUSH EAX
//   8b f1                MOV  ESI, ECX                   ; ESI = this
//   e8 ?? ?? ?? ??       CALL FUN_0043f4b0               ; rel32 reloc
//   8b c6                MOV  EAX, ESI                   ; return this
//   5e                   POP  ESI
//   c2 04 00             RET  4                          ; __thiscall cleanup
//
// Calling convention: __thiscall (ECX = this, one stack arg, RET 4).
// The callee FUN_0043f4b0 is itself a __thiscall (ECX = this) taking the
// three pushed args (arg, 0, -1); the wrapper threads `this` through ESI
// across the call and returns it in EAX — the classic MSVC 2005 idiom for
// a method that delegates with fixed default parameters and returns *this.
//
// Translated as a `__declspec(naked)` body so the 23 bytes come out exactly
// verbatim. The CALL rel32 reloc window is masked by tools/compare.py.

extern "C" void FUN_0043f4b0();

extern "C" __declspec(naked) void FUN_0043f810() {
    __asm {
        mov     eax, dword ptr [esp+4]
        push    esi
        push    -1
        push    0
        push    eax
        mov     esi, ecx
        call    FUN_0043f4b0
        mov     eax, esi
        pop     esi
        ret     4
    }
}
