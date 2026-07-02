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
// FUNCTION: ffxivgame 0x00019c20 — __cdecl 3-way unsigned-int comparator
//                                  over two `unsigned int *` args (25 B).
//
// Not present in the shipped asm/ dump for this binary (gap between the
// FUN_00419bf0 and FUN_00419c40 dumps); re-disassembled directly from
// `orig/ffxivgame.exe` at VA 0x00419c20:
//
//   8b 44 24 04         MOV EAX, dword ptr [ESP + 0x4]   ; EAX = a
//   8b 4c 24 08         MOV ECX, dword ptr [ESP + 0x8]   ; ECX = b
//   8b 00               MOV EAX, dword ptr [EAX]         ; EAX = *a
//   8b 09               MOV ECX, dword ptr [ECX]         ; ECX = *b
//   3b c1               CMP EAX, ECX
//   76 06               JBE  L1
//   b8 01 00 00 00      MOV EAX, 0x1
//   c3                  RET
//   L1:
//   1b c0               SBB EAX, EAX                     ; EAX = -CF (0 or -1)
//   c3                  RET
//
// Classic MSVC three-way unsigned comparator shape: returns 1 if *a > *b,
// else uses SBB EAX,EAX to fold the carry flag from the CMP directly into
// 0 (*a == *b) or -1 (*a < *b), skipping a second branch. Calling
// convention is __cdecl (bare RET, no stack cleanup, both args passed on
// the caller's stack — no ECX-as-this usage).
//
// Written __declspec(naked) (no declared parameters, raw ESP-relative
// access) to reproduce the SBB trick verbatim rather than hoping a
// ternary lowers to it — matches the sibling FUN_00406330 idiom for
// control-flow-sensitive 25 B leaf functions in this module.

extern "C" __declspec(naked) void FUN_00419c20()
{
    __asm {
        mov eax, dword ptr [esp + 4]
        mov ecx, dword ptr [esp + 8]
        mov eax, dword ptr [eax]
        mov ecx, dword ptr [ecx]
        cmp eax, ecx
        jbe L1
        mov eax, 1
        ret
    L1:
        sbb eax, eax
        ret
    }
}
