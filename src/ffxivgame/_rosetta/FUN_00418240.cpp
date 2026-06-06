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
// FUNCTION: ffxivgame 0x00418240 — two-void-call + guarded-pointer-call + tail-jmp (33 B)
//
// No stack frame, no saved registers, no RET — ends with a tail-call JMP.
// Pattern: call two zero-arg void fns, then call a getter; if the getter
// returned non-null, forward the pointer to a single-arg thunk; then
// unconditionally tail-call a continuation.
//
// Disassembly (verbatim, 33 bytes):
//   00418240:  e8 4b c7 00 00   call FUN_00424990              ; void no-op
//   00418245:  e8 86 c3 00 00   call FUN_004245d0              ; void init
//   0041824a:  e8 a1 36 00 00   call FUN_0041b8f0              ; → EAX = ptr
//   0041824f:  85 c0            test eax, eax
//   00418251:  74 09            je   +9 (→ 0041825c)
//   00418253:  50               push eax
//   00418254:  e8 57 6d 00 00   call thunk_FUN_0041ec20        ; 1-arg __cdecl
//   00418259:  83 c4 04         add  esp, 4
//   0041825c:  e9 1f 7d 00 00   jmp  FUN_0041ff80              ; tail call
//
// Calling convention: __cdecl, zero args, void return.
// Frame: none (no push ebp / mov ebp,esp; no callee-saved regs).
// The tail JMP is MSVC /O2 optimising "return FUN_0041ff80();" as a
// jump-instead-of-call+ret since no cleanup is needed.

extern "C" void FUN_00424990();
extern "C" void FUN_004245d0();
extern "C" int  FUN_0041b8f0();
extern "C" void thunk_FUN_0041ec20(int);
extern "C" void FUN_0041ff80();

extern "C" __declspec(naked) void FUN_00418240()
{
    __asm {
        call FUN_00424990
        call FUN_004245d0
        call FUN_0041b8f0
        test eax, eax
        je   skip
        push eax
        call thunk_FUN_0041ec20
        add  esp, 4
    skip:
        jmp  FUN_0041ff80
    }
}
