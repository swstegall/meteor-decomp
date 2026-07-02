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
// FUNCTION: ffxivgame 0x0043e1d0 — __thiscall constructor: stamps its own
//                                  vftable then delegates field init to
//                                  FUN_0043c6c0 (24 B / 0x18).
//
// Asm (24 bytes @ 0x0043e1d0):
//   56                    PUSH ESI
//   8b f1                 MOV  ESI, ECX             ; ESI = this (ECX preserved)
//   c7 06 RR RR RR RR     MOV  dword ptr [ESI],
//                              offset vftable        ; this->vftable = ...  (DIR32 reloc)
//   a1 RR RR RR RR        MOV  EAX, [g_0132ca1c]     ; load global arg      (moffs32 reloc)
//   50                    PUSH EAX
//   e8 RR RR RR RR        CALL FUN_0043c6c0          ; this(ECX)=this, arg=EAX; rel32 reloc
//   8b c6                 MOV  EAX, ESI              ; return this
//   5e                    POP  ESI
//   c3                    RET
//
// FUN_0043c6c0 is the __thiscall "member initialiser" documented in its
// own file (src/ffxivgame/_rosetta/FUN_0043c6c0.cpp) — it writes only to
// [this + 0x4] and never touches [this + 0x0], which is why the vftable
// stamp here precedes the call: this class has no polymorphic base, so
// MSVC emits the vftable assignment as the first statement of the
// constructor body, ahead of the subsequent init-helper call.
//
// Translated as a `__declspec(naked)` body (mirrors the sibling ctor
// idiom in FUN_00401030.cpp) so the vftable DIR32 reloc, the global
// moffs32 reloc, and the CALL rel32 reloc all land at their exact
// original offsets; tools/compare.py masks all three reloc windows in
// its diff, so the .obj's .text is byte-identical to the original slice.

extern "C" {

// This class's own vftable (DIR32 reloc target — value itself is masked
// by compare.py, only the reloc's presence/position matters).
int FUN_0043e1d0_vftable;

// Global argument forwarded to the field-init helper (moffs32 reloc
// target — same masking rule).
int g_0132ca1c;

// __thiscall member initialiser: FUN_0043c6c0(this=ECX, arg on stack);
// callee-cleans 1 dword (RET 4). See FUN_0043c6c0.cpp for its body.
void FUN_0043c6c0();

__declspec(naked) void FUN_0043e1d0() {
    __asm {
        push    esi
        mov     esi, ecx
        mov     dword ptr [esi], offset FUN_0043e1d0_vftable
        mov     eax, dword ptr [g_0132ca1c]
        push    eax
        call    FUN_0043c6c0
        mov     eax, esi
        pop     esi
        ret
    }
}

}  // extern "C"
