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
// FUNCTION: ffxivgame 0x000139b0 — __thiscall LeaveCriticalSection wrapper (11 bytes)
//
// Asm (11 bytes @ orig RVA 0x000139b0):
//   83 c1 3c          ADD ECX, 0x3c                    ; ECX += 0x3c (-> &this->lock)
//   51                PUSH ECX                         ; push LPCRITICAL_SECTION arg
//   ff 15 68 e1 f3 00 CALL [0x00f3e168]                ; CALL LeaveCriticalSection (IAT)
//   c3                RET                              ; __thiscall, no stack args
//
// Calls LeaveCriticalSection on the CRITICAL_SECTION embedded at this+0x3c.
// MSVC uses ADD ECX, 0x3c / PUSH ECX rather than LEA+PUSH here.
// No frame, no saved registers. IAT reloc at +5 (0x00f3e168 = LeaveCriticalSection).
//
// The +0x3c lock offset matches the `CRITICAL_SECTION lock` field in
// SQEX::CDev::Engine::Memory::Alternative::DetachableHeapSpace (see
// decomp-notes/types/ffxivgame/0x00013850.md). Paired with FUN_004139a0
// (EnterCriticalSection wrapper, same lock offset, IAT 0x00f3e16c).
//
// Reconstruction strategy: naked-asm byte passthrough — the IAT-indirect CALL
// carries an absolute address (relocation) that cannot be reproduced by a
// source-level call to LeaveCriticalSection() without the full binary relink.

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
extern "C" void FUN_004139b0() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_004139b0() {
    __asm {
        _emit 0x83              // ADD ECX, 0x3c
        _emit 0xc1
        _emit 0x3c
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
