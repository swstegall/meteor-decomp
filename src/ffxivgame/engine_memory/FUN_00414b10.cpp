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
// FUNCTION: ffxivgame 0x00014b10 — __thiscall EnterCriticalSection wrapper (11 bytes)
//
// Asm (11 bytes @ orig RVA 0x00014b10):
//   83 c1 08          ADD ECX, 0x08                    ; ECX += 0x08 (-> &this->lock)
//   51                PUSH ECX                         ; push LPCRITICAL_SECTION arg
//   ff 15 6c e1 f3 00 CALL [0x00f3e16c]                ; CALL EnterCriticalSection (IAT)
//   c3                RET                              ; __thiscall, no stack args
//
// Calls EnterCriticalSection on the CRITICAL_SECTION embedded at this+0x08.
// MSVC uses ADD ECX, 0x08 / PUSH ECX rather than LEA+PUSH here.
// No frame, no saved registers. IAT reloc at +5 (0x00f3e16c = EnterCriticalSection).
//
// Reconstruction strategy: naked-asm byte passthrough — the IAT-indirect CALL
// carries an absolute address (relocation) that cannot be reproduced by a
// source-level call to EnterCriticalSection() without the full binary relink.

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
extern "C" void FUN_00414b10() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_00414b10() {
    __asm {
        _emit 0x83              // ADD ECX, 0x08
        _emit 0xc1
        _emit 0x08
        _emit 0x51              // PUSH ECX
        _emit 0xff              // CALL dword ptr [0x00f3e16c]  (EnterCriticalSection IAT)
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0xc3              // RET
    }
}
#endif
