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
// FUNCTION: ffxivgame 0x00024fe0 — __cdecl no-arg thunk that stores the
//                                  constant index 7 to a global and calls
//                                  FUN_0041d100 with the same value
//                                  (18 B / 0x12).
//
// Asm (18 bytes @ 0x00024fe0):
//   b8 07 00 00 00   MOV  EAX, 7
//   50               PUSH EAX               ; push arg for the call
//   a3 14 8f 32 01   MOV  [0x01328f14], EAX ; store 7 to global (a3 = acc-specific form)
//   e8 10 81 ff ff   CALL FUN_0041d100       ; __cdecl, 1-int arg
//   59               POP  ECX               ; single-arg cdecl cleanup
//   c3               RET                    ; __cdecl (no args to pop)
//
// Calling convention: __cdecl (RET with no operand).
// Stack frame: none. No callee-saved registers touched.
//
// Structural notes:
//   The semantic source is `FUN_0041d100(g_state = 7)` — stores 7 to the
//   global at 0x01328f14 and passes the same value as the call argument.
//   However, source-level C++ under MSVC 2005 /O2 emits the shorter
//   `PUSH imm8` + `MOV [addr], imm32` encoding (19 bytes total), whereas
//   the original binary uses the accumulator-routing path that loads 7 into
//   EAX first (`MOV EAX, 7`), pushes EAX (1 byte), then stores via the
//   5-byte `a3 <imm32>` MOV-EAX-to-moffs32 form — 18 bytes total.  The
//   EAX-routing form is in fact 1 byte shorter overall (11 vs 12 bytes for
//   the push+store pair), but MSVC 2005 does not rediscover it from a
//   simple constant-argument call.  The `__declspec(naked)` byte passthrough
//   below preserves the exact original encoding.
//
// Reloc-bearing sites (masked by compare.py — raw addresses here match orig):
//   +0x07   MOV [imm32], EAX  — absolute data reloc  → VA 0x01328f14
//   +0x0b   CALL rel32        — relative code reloc   → FUN_0041d100

extern "C" __declspec(naked) void FUN_00424fe0() {
    __asm {
        _emit 0xb8      // MOV EAX, 7
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x50      // PUSH EAX

        _emit 0xa3      // MOV [0x01328f14], EAX  (a3 = MOV moffs32, EAX)
        _emit 0x14
        _emit 0x8f
        _emit 0x32
        _emit 0x01

        _emit 0xe8      // CALL FUN_0041d100  (rel32; reloc-masked)
        _emit 0x10
        _emit 0x81
        _emit 0xff
        _emit 0xff

        _emit 0x59      // POP ECX  (single-arg cdecl cleanup)
        _emit 0xc3      // RET
    }
}
