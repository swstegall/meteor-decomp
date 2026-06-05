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
// FUNCTION: ffxivgame 0x00043940 — `__stdcall` thin forwarder that re-passes
//                                   its (float, int) arguments to the helper
//                                   at 0x00b53f20 (21 B / 0x15).
//
// Behaviour read from the disassembly at orig RVA 0x00043940:
//
//     8b 44 24 08        mov  eax, [esp+8]     ; load int arg  b
//     d9 44 24 04        fld  dword [esp+4]    ; load float arg a -> st0
//     50                 push eax              ; push b   (2nd callee arg)
//     51                 push ecx              ; reserve 4-byte slot
//     d9 1c 24           fstp dword [esp]      ; spill a  (1st callee arg)
//     e8 RR RR RR RR     call 0x00b53f20       ; e8 + REL32 reloc
//     c2 08 00           ret  8                ; __stdcall epilogue (2 args)
//
//   The function evaluates both arguments into registers (b -> eax, a -> x87
//   st0) BEFORE adjusting esp, then performs the two right-to-left pushes:
//   b first, then the float a via the classic MSVC `push reg`-reserve +
//   `fstp [esp]` spill idiom. The wrapper does not clean its own pushed
//   slots — the callee at 0x00b53f20 is itself `__stdcall(float, int)` and
//   pops them, so the wrapper's `ret 8` only discards its own two args.
//
//   Calling convention: `__stdcall` (two 4-byte stack args, callee pops via
//   `ret 8`). No stack frame, no callee-saves, no security cookie.
//
//   The only reloc-bearing site in the orig 21 bytes is the `e8` REL32 to
//   FUN_00b53f20; `tools/compare.py` masks the 4-byte offset window during
//   the byte diff so the source-level CALL lines up.

extern "C" float __stdcall FUN_00b53f20(float a, int b);

extern "C" float __stdcall FUN_00443940(float a, int b) {
    return FUN_00b53f20(a, b);
}
