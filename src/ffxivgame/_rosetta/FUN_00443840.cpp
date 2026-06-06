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
// FUNCTION: ffxivgame 0x00043840 — __thiscall constructor (26 B / 0x1a).
//
// Standard MSVC 2005 ctor shape: stash `this` (ECX) in ESI, chain to a
// base-class ctor at VA 0x00b52780, zero a couple of fields, stamp the
// vftable VA (0x00f671fc) into [this], and return `this` in EAX. The
// bare `ret` (no `ret N`) means the function takes only the implicit
// `this` pointer — no stack arguments to clean up.
//
// Asm (26 bytes):
//   56                   PUSH ESI
//   8B F1                MOV  ESI, ECX            ; ESI = this
//   E8 RR RR RR RR       CALL base_ctor          ; @ VA 0x00b52780 (rel32)
//   33 C0                XOR  EAX, EAX
//   89 46 10             MOV  [ESI+0x10], EAX     ; field@0x10 = 0
//   88 46 14             MOV  [ESI+0x14], AL      ; field@0x14 = 0 (byte)
//   C7 06 FC 71 F6 00    MOV  dword ptr [ESI], offset vftable   ; @ VA 0x00f671fc (DIR32)
//   8B C6                MOV  EAX, ESI            ; return this
//   5E                   POP  ESI
//   C3                   RET
//
// Translated as a `__declspec(naked)` body so the 26 bytes come out
// verbatim. The CALL rel32 to base_ctor and the DIR32 immediate for the
// vftable are referenced via `extern "C"` symbols; tools/compare.py
// masks those 4-byte reloc windows in its diff.

extern "C" {

// Base-class constructor at VA 0x00b52780. Referenced via CALL rel32.
void base_ctor();

// vftable at VA 0x00f671fc. Referenced via DIR32 reloc on the immediate
// of `mov dword ptr [esi], offset vftable`.
int vftable;

__declspec(naked) void FUN_00443840() {
    __asm {
        push    esi
        mov     esi, ecx
        call    base_ctor
        xor     eax, eax
        mov     dword ptr [esi+0x10], eax
        mov     byte ptr [esi+0x14], al
        mov     dword ptr [esi], offset vftable
        mov     eax, esi
        pop     esi
        ret
    }
}

}  // extern "C"
