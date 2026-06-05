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
// FUNCTION: ffxivgame 0x000599f0 — stack-argument adjustor thunk: decrements
//                                   [esp+4] by 4 then tail-jumps to
//                                   FUN_00458cd0 (10 B / 0xa).
//
// Calling convention: no frame (naked); single stdcall arg on stack.
//
// Behaviour:
//   SUB dword ptr [esp+4], 4   ; decrease the pointer argument in-place
//   JMP FUN_00458cd0            ; tail-call the real worker
//
// FUN_00458cd0 is __stdcall (ret 4): it increments a 32-bit field at +0x8
// of the (now-adjusted) pointer and returns the new value.  The adjustment
// here shifts the pointer back by one 4-byte step so the callee addresses a
// different struct base — classic MSVC multi-inheritance adjustor, but on
// the stack rather than ECX.

extern "C" void FUN_00458cd0();

extern "C" __declspec(naked) void FUN_004599f0() {
    __asm {
        sub dword ptr [esp + 4], 4
        jmp FUN_00458cd0
    }
}
