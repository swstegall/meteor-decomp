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
// FUNCTION: ffxivgame 0x00019bf0 — unsigned 64-bit less-than compare (37 B)
//
// Returns 1 if *a < *b (unsigned 64-bit), 0 otherwise.
// Compares high 32-bit word first (at +4), then low word (at +0) if equal.
//
// Calling convention: __stdcall (RET 8, 2 pointer args).
// Frame: none (/Oy — no locals, no callee-saves).
//
// Register layout (expected):
//   EAX = a (pointer to a; reused as a's low word after hi comparison)
//   EDX = a's high 32-bit word (scratch)
//   ECX = b (pointer to b; b's words accessed via memory operands)
//
// Asm (37 bytes @ orig RVA 0x00019bf0):
//   8b 44 24 04   MOV EAX, [ESP+4]     ; a
//   8b 50 04      MOV EDX, [EAX+4]     ; a->hi (high 32 bits at +4)
//   8b 4c 24 08   MOV ECX, [ESP+8]     ; b
//   3b 51 04      CMP EDX, [ECX+4]     ; a->hi vs b->hi
//   77 10         JA  → return_0       ; a->hi > b->hi
//   72 06         JC  → return_1       ; a->hi < b->hi
//   8b 00         MOV EAX, [EAX]       ; a->lo (low 32 bits at +0)
//   3b 01         CMP EAX, [ECX]       ; a->lo vs b->lo
//   73 08         JNC → return_0       ; a->lo >= b->lo
//   b8 01000000   MOV EAX, 1           ; return_1
//   c2 08 00      RET 8
//   33 c0         XOR EAX, EAX         ; return_0
//   c2 08 00      RET 8

int __stdcall FUN_00419bf0(const unsigned __int64* a, const unsigned __int64* b)
{
    if (*a < *b) return 1;
    return 0;
}
