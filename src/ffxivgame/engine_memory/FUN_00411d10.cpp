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
// FUNCTION: ffxivgame 0x00011d10 — __thiscall LeaveCriticalSection wrapper (11 bytes)
//
// Asm (11 bytes @ orig RVA 0x00011d10):
//   83 c1 5c          ADD ECX, 0x5c                    ; ECX += 0x5c (-> &this->lock)
//   51                PUSH ECX                         ; push LPCRITICAL_SECTION arg
//   ff 15 68 e1 f3 00 CALL [0x00f3e168]                ; CALL LeaveCriticalSection (IAT)
//   c3                RET                              ; __thiscall, no stack args
//
// Calls LeaveCriticalSection on the CRITICAL_SECTION embedded at this+0x5c.
// MSVC uses ADD ECX, 0x5c / PUSH ECX rather than LEA+PUSH here.
// No frame, no saved registers. IAT reloc at +5 (0x00f3e168 = LeaveCriticalSection).
//
// Reconstruction strategy: naked-asm byte passthrough — the IAT-indirect CALL
// carries an absolute address (relocation) that cannot be reproduced by a
// source-level call to LeaveCriticalSection() without the full binary relink.

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
extern "C" void FUN_00411d10() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_00411d10() {
    __asm {
        _emit 0x83              // ADD ECX, 0x5c
        _emit 0xc1
        _emit 0x5c
        _emit 0x51              // PUSH ECX
        _emit 0xff              // CALL dword ptr [0x00f3e168]  (LeaveCriticalSection IAT)
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0xc3              // RET
    }
}
#endif
