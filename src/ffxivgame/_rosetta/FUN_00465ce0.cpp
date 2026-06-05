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
// FUNCTION: ffxivgame 0x00065ce0 — `__cdecl` 8-byte struct copy (19 B / 0x13).
//
// Behaviour read from the disassembly at orig RVA 0x00065ce0:
//
//   __cdecl void FUN_00465ce0(Pair* dst, Pair* src) {
//       *dst = *src;            // copy two consecutive dwords
//   }
//
//   Two pointer arguments: dst = [ESP+4], src = [ESP+8]. The body loads
//   src first, reads its first dword (EDX), then loads dst and stores;
//   reads src's second dword (EAX) and stores it at dst+4. No frame, no
//   callee-saves, no security cookie. Plain `RET` (caller cleans args)
//   confirms `__cdecl`.
//
//   Asm shape (19 bytes total):
//
//     8b 44 24 08        mov  eax, [esp+8]    ; src
//     8b 10              mov  edx, [eax]      ; src->a
//     8b 4c 24 04        mov  ecx, [esp+4]    ; dst
//     89 11              mov  [ecx], edx      ; dst->a = src->a
//     8b 40 04           mov  eax, [eax+4]    ; src->b
//     89 41 04           mov  [ecx+4], eax    ; dst->b = src->b
//     c3                 ret

struct Pair {
    int a;
    int b;
};

extern "C" void __cdecl FUN_00465ce0(Pair* dst, Pair* src) {
    *dst = *src;
}
