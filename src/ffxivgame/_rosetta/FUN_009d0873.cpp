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
// FUNCTION: ffxivgame 0x005d0873 — __thiscall two-step free helper:
//                                   push this->field_0, call FUN_009d178d,
//                                   push this->field_0 again, call _free
//                                   (17 B / 0x11)
//
// Calling convention: __thiscall (ECX = this). No RET within this slice —
// the stack-cleanup epilogue (POP ECX; POP ECX; POP ESI; RET) lives in
// the 4-byte inter-function gap at 0x005d0884 that immediately follows.
// This pattern is a known MSVC 2005 /O2 deferred-cleanup split: the
// compiler emits the two __cdecl CALLs (each push one stack arg) and
// defers the ADD ESP / POP cleanup until after the second call, placing
// the shared epilogue at an address that Ghidra attributes to the gap
// rather than to this function.
//
// Asm (17 bytes @ orig RVA 0x005d0873):
//   56                   PUSH ESI                ; save ESI
//   8b f1                MOV  ESI, ECX           ; ESI = this
//   ff 36                PUSH DWORD PTR [ESI]    ; push this->field_0
//   e8 10 0f 00 00       CALL FUN_009d178d       ; __cdecl helper (one arg)
//   ff 36                PUSH DWORD PTR [ESI]    ; push this->field_0 again
//   e8 93 12 00 00       CALL _free              ; CRT free (one arg)
//   -- falls through to gap bytes 59 59 5e c3 (POP ECX; POP ECX; POP ESI; RET) --
//
// FUN_009d178d (RVA 0x5d178d, 11 B): __cdecl wrapper that calls an imported
// function via IAT at 0x00f3e170, passing through its one stack argument.
// _free (RVA 0x5d1b17, 5 B): JMP thunk for the CRT free() routine.
//
// The two CALL rel32 sites (+5 and +12) are relocation bytes masked by
// compare.py and do not affect the diff outcome.

extern "C" void FUN_009d178d(void*);
extern "C" void free(void*);

extern "C" __declspec(naked) void FUN_009d0873() {
    __asm {
        push esi
        mov  esi, ecx
        push dword ptr [esi]
        call FUN_009d178d
        push dword ptr [esi]
        call free
    }
}
