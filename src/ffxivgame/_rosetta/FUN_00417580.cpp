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
// FUNCTION: ffxivgame 0x00017580 — `__stdcall` unsigned-dword three-way
//                                   comparator (29 B).
//
// int __stdcall FUN_00417580(const unsigned int* a, const unsigned int* b)
//   [ESP+0x4]  : a  (pointer to first  unsigned dword)
//   [ESP+0x8]  : b  (pointer to second unsigned dword)
//   Returns  1 if *b > *a
//             0 if *b == *a
//            -1 if *b < *a
//
// Asm shape (29 bytes, read from orig RVA 0x00017580):
//
//   00017580:  8b 44 24 04          MOV  EAX, [ESP+0x4]   ; EAX = a
//   00017584:  8b 4c 24 08          MOV  ECX, [ESP+0x8]   ; ECX = b
//   00017588:  8b 00                MOV  EAX, [EAX]       ; EAX = *a
//   0001758a:  8b 09                MOV  ECX, [ECX]       ; ECX = *b
//   0001758c:  3b c8                CMP  ECX, EAX         ; *b - *a
//   0001758e:  76 08                JBE  +0x8             ; jump if *b <= *a
//   00017590:  b8 01 00 00 00       MOV  EAX, 0x1         ; return 1
//   00017595:  c2 08 00             RET  0x8
//   00017598:  1b c0                SBB  EAX, EAX         ; EAX = 0 or -1
//   0001759a:  c2 08 00             RET  0x8
//
// The SBB EAX, EAX at +0x18 exploits the carry flag left by CMP ECX, EAX:
//   CF=1 when *b < *a  → SBB yields -1 (0xFFFFFFFF)
//   CF=0 when *b == *a → SBB yields  0
// Combined with the early-return branch, the function implements a three-way
// unsigned comparison where a positive result means *b is strictly larger.
//
// No external call/jump targets — no relocations. Straight naked passthrough.

extern "C" __declspec(naked) void FUN_00417580() {
    __asm {
        _emit 0x8b          // MOV  EAX, [ESP+0x4]   ; a
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b          // MOV  ECX, [ESP+0x8]   ; b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b          // MOV  EAX, [EAX]       ; *a
        _emit 0x00
        _emit 0x8b          // MOV  ECX, [ECX]       ; *b
        _emit 0x09
        _emit 0x3b          // CMP  ECX, EAX
        _emit 0xc8
        _emit 0x76          // JBE  +0x8
        _emit 0x08
        _emit 0xb8          // MOV  EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc2          // RET  0x8
        _emit 0x08
        _emit 0x00
        _emit 0x1b          // SBB  EAX, EAX
        _emit 0xc0
        _emit 0xc2          // RET  0x8
        _emit 0x08
        _emit 0x00
    }
}
