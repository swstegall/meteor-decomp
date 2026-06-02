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
// FUNCTION: ffxivgame 0x005daab2 — FUN_009daab2 (naked EBP-relative conditional
//                                  call thunk, 24 B)
//
// Naked helper that runs inside a caller's EBP frame (no prologue/epilogue).
// Checks a local variable at [EBP-0x20]: if it is non-zero, returns
// immediately. If zero, forwards four EBP-relative values as arguments to
// FUN_009d1bee (__stdcall, 4 args) and then returns.
//
// Asm (24 bytes @ RVA 0x005daab2 / VA 0x009daab2):
//   83 7d e0 00    CMP  dword ptr [EBP - 0x20], 0x0
//   75 11          JNZ  +0x11  → done (RET)
//   ff 75 1c       PUSH dword ptr [EBP + 0x1c]
//   ff 75 e4       PUSH dword ptr [EBP - 0x1c]
//   ff 75 10       PUSH dword ptr [EBP + 0x10]
//   ff 75 08       PUSH dword ptr [EBP + 0x08]
//   e8 25 71 ff ff CALL FUN_009d1bee          ; rel32 reloc
//   c3             RET
//
// FUN_009d1bee is __stdcall (4 × int args = 16 bytes; callee cleans stack),
// so no ADD ESP, 16 is needed in the caller.

extern "C" void __stdcall FUN_009d1bee(int, int, int, int);

extern "C" __declspec(naked) void FUN_009daab2()
{
    __asm {
        cmp  dword ptr [ebp - 0x20], 0
        jnz  done
        push dword ptr [ebp + 0x1c]
        push dword ptr [ebp - 0x1c]
        push dword ptr [ebp + 0x10]
        push dword ptr [ebp + 0x08]
        call FUN_009d1bee
    done:
        ret
    }
}
