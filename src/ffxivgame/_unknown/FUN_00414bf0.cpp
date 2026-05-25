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
// FUNCTION: ffxivgame 0x00014bf0 — __thiscall three-field init (20 B)
//
// Initialises three adjacent fields on the `this` object:
//   this[+0x00] = 0   (dword)
//   this[+0x04] = 0   (dword)
//   this[+0x08] = 1   (byte)
//
// No arguments other than the implicit `this` (passed in ECX). No
// frame (/Oy), no callee-saved spills — leaf function.
//
// Asm (20 bytes @ orig RVA 0x00014bf0):
//   8b c1                  MOV EAX, ECX                       ; eax = this
//   c7 00 00 00 00 00      MOV DWORD PTR [EAX], 0
//   c7 40 04 00 00 00 00   MOV DWORD PTR [EAX + 0x04], 0
//   c6 40 08 01            MOV BYTE  PTR [EAX + 0x08], 1
//   c3                     RET
//
// Calling convention: __thiscall (ECX = this, no stack args, plain RET).
// The leading `MOV EAX, ECX` is the canonical MSVC 2005 /O2 thiscall
// prologue — even when the function doesn't return `this`, MSVC re-uses
// EAX as the base register for the subsequent struct stores so the
// instruction stream uses the shorter [EAX+disp8] form consistently.
//
// Encoded as `__declspec(naked)` inline asm so the .obj's `.text` is
// exactly 20 bytes matching orig — a high-level C++ ctor would emit
// the equivalent stores via ECX directly (`c7 01 / c7 41 04 / c6 41
// 08`), which is the same byte count but a different instruction
// encoding, breaking the byte match.
//
// Sibling pattern: FUN_00404220 (19 B, same `MOV EAX, ECX` prologue +
// constant stores via [EAX+disp]) — same family of small thiscall
// initialisers naked-encoded for byte fidelity.

extern "C" __declspec(naked) void FUN_00414bf0() {
    __asm {
        mov eax, ecx
        mov dword ptr [eax], 0
        mov dword ptr [eax + 4], 0
        mov byte ptr [eax + 8], 1
        ret
    }
}
