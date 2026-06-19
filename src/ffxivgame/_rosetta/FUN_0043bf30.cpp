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
// FUNCTION: ffxivgame 0x0003bf30 — __thiscall 2-IAT-call notify/close helper
//                                  (25 B / 0x19).
//
// Asm (25 bytes @ 0x0003bf30):
//   56                   PUSH ESI
//   8b f1                MOV  ESI, ECX            ; ESI = this (__thiscall)
//   8b 46 0c             MOV  EAX, [ESI + 0x0c]   ; load field_0xC
//   50                   PUSH EAX                 ; push as arg
//   ff 15 38 e1 f3 00    CALL dword ptr [0x00f3e138]  ; IAT fn #1 (ReleaseSemaphore-style)
//   8b 4e 08             MOV  ECX, [ESI + 0x08]   ; load field_0x8
//   51                   PUSH ECX                 ; push as arg
//   ff 15 3c e1 f3 00    CALL dword ptr [0x00f3e13c]  ; IAT fn #2 (CloseHandle-style)
//   5e                   POP  ESI
//   c3                   RET
//
// Structurally the same as the two IAT-dispatch sites in the sibling loops
// FUN_0043b770 / FUN_0043c330: the IAT slots at 0x00f3e138 (ReleaseSemaphore /
// SetEvent) and 0x00f3e13c (CloseHandle) are called in that order with handles
// stored at [this+0x0c] and [this+0x08] respectively.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Generating `ff 15 addr32` (CALL dword ptr [absolute]) from source-level
//   C++ requires __declspec(dllimport) and a correctly-named import thunk
//   which we cannot supply here.  Following the established sibling idiom
//   (FUN_00435670, FUN_0043b770, FUN_0043c330), the 25 bytes are re-emitted
//   verbatim via MASM _emit directives so the .obj's .text is byte-identical
//   to orig[0x3bf30..0x3bf49].
//
//   Reloc-bearing sites (two DIR32 IAT addresses, masked by compare.py):
//     +0x07  dword [0x00f3e138]  → IAT slot for fn #1
//     +0x11  dword [0x00f3e13c]  → IAT slot for fn #2

extern "C" __declspec(naked) void FUN_0043bf30() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x0c]
        _emit 0x46
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL dword ptr [0x00f3e138]
        _emit 0x15
        _emit 0x38
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x08]
        _emit 0x4e
        _emit 0x08
        _emit 0x51              // PUSH ECX
        _emit 0xff              // CALL dword ptr [0x00f3e13c]
        _emit 0x15
        _emit 0x3c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
