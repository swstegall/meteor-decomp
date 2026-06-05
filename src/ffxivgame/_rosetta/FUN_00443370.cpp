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
// FUNCTION: ffxivgame 0x00043370 — __thiscall single-arg ctor that forwards
//                                  its argument to a base ctor, stamps an
//                                  object vftable, and returns *this (25 B).
//
// Structurally a sibling of std::bad_alloc::bad_alloc @ 0x00401030: a
// __thiscall constructor that loads its one stack arg, passes it to a base
// constructor (here @ VA 0x00442aa0), writes the derived vftable VA
// (0x00f67124) into [this], and returns `this` in EAX. The only shape
// difference from the bad_alloc ctor is the arg is loaded by value
// (`MOV EAX,[ESP+4]`) BEFORE the `PUSH ESI`, rather than as `LEA &arg`
// after it.
//
// Asm (25 bytes):
//   8b 44 24 04          MOV EAX, [ESP+0x4]        ; load arg
//   56                   PUSH ESI
//   50                   PUSH EAX                  ; pass arg to base ctor
//   8b f1                MOV ESI, ECX              ; ESI = this
//   e8 RR RR RR RR       CALL base_ctor            ; @ VA 0x00442aa0 (rel32)
//   c7 06 24 71 f6 00    MOV dword ptr [ESI],      ; vftable @ VA 0x00f67124
//                              offset vftable        (DIR32 immediate)
//   8b c6                MOV EAX, ESI              ; return this
//   5e                   POP ESI
//   c2 04 00             RET 0x4                   ; __thiscall cleanup
//
// Translated as a `__declspec(naked)` body so the 25 bytes come out exactly.
// The two reloc-bearing operands (CALL rel32 to the base ctor, DIR32
// immediate for the vftable) are referenced via `extern "C"` symbols;
// tools/compare.py masks those 4-byte windows in its diff.

extern "C" {

// Base constructor at VA 0x00442aa0. Referenced via CALL rel32 reloc.
void base_ctor();

// Derived object vftable at VA 0x00f67124. Referenced via DIR32 reloc on
// the immediate of `mov dword ptr [esi], offset vftable`.
int vftable;

__declspec(naked) void FUN_00443370() {
    __asm {
        mov     eax, dword ptr [esp+4]
        push    esi
        push    eax
        mov     esi, ecx
        call    base_ctor
        mov     dword ptr [esi], offset vftable
        mov     eax, esi
        pop     esi
        ret     4
    }
}

}  // extern "C"
