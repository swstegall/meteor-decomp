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
// FUNCTION: ffxivgame 0x00403cf0 — single-arg __stdcall trampoline that
// forwards (count, 0) to the operator-new[] wrapper at FUN_00403bd0
// (`operator new[]` for arrays of 28-byte elements). The second
// argument is the canonical MSVC "extra slot" that the 2-arg `operator
// new[](size_t, int)` placement-style overload takes; FUN_00403bd0's
// body ignores it, but the calling convention still requires the push
// so the cdecl stack layout lines up.
//
// Asm (18 bytes):
//   8b 44 24 04         mov  eax, [esp+4]      ; load `count`
//   6a 00               push 0                 ; placement int = 0
//   50                  push eax               ; count
//   e8 d4 fe ff ff      call FUN_00403bd0      ; rel32 reloc, cdecl callee
//   83 c4 08            add  esp, 8            ; caller cleanup (cdecl)
//   c2 04 00            ret  4                 ; __stdcall epilogue
//
// Calling convention: __stdcall (single 4-byte arg, callee cleans).
// The single reloc-bearing position (the call rel32 at off 0x07) is
// masked out of the byte diff by tools/compare.py.

extern "C" void operator_new_array_28(); // FUN_00403bd0

extern "C" __declspec(naked) void FUN_00403cf0() {
    __asm {
        mov  eax, dword ptr [esp + 4]
        push 0
        push eax
        call operator_new_array_28
        add  esp, 8
        ret  4
    }
}
