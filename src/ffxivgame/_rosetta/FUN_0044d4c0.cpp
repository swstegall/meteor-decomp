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
// FUNCTION: ffxivgame 0x0044d4c0 — `__cdecl` 2-arg trampoline that forwards
//                                   to the 3-arg helper FUN_0044d350,
//                                   wedging a constant 0 as the middle
//                                   argument (21 B / 0x15).
//
// Behaviour read from the disassembly at orig RVA 0x0004d4c0:
//
//   __cdecl <ret> FUN_0044d4c0(void* a, void* b) {
//       return FUN_0044d350(a, 0, b);
//   }
//
//   The two caller-pushed slots are loaded up front (EAX = b, ECX = a),
//   then re-pushed right-to-left around a literal `push 0` to build the
//   three-argument call frame. `add esp, 0xc` performs the `__cdecl`
//   caller cleanup of the three pushed 32-bit slots; the bare `ret`
//   (no `ret N`) marks FUN_0044d4c0 itself as `__cdecl`.
//
//   Asm shape (21 bytes total):
//
//     8b 44 24 08        mov  eax, [esp+8]      ; b
//     8b 4c 24 04        mov  ecx, [esp+4]      ; a
//     50                 push eax               ; arg3 = b
//     6a 00              push 0                 ; arg2 = 0
//     51                 push ecx               ; arg1 = a
//     e8 RR RR RR RR     call FUN_0044d350      ; e8 + REL32 reloc
//     83 c4 0c           add  esp, 0xc          ; cdecl cleanup (3 args)
//     c3                 ret                    ; cdecl epilogue
//
//   The only reloc-bearing site in the orig 21 bytes is the `e8` REL32 to
//   FUN_0044d350; tools/compare.py masks the 4-byte offset window during
//   the byte diff so the source-level CALL lines up with the orig PE's
//   resolved offset.

extern "C" void* __cdecl FUN_0044d350(void* a, int b, void* c);

extern "C" void* __cdecl FUN_0044d4c0(void* a, void* b) {
    return FUN_0044d350(a, 0, b);
}
