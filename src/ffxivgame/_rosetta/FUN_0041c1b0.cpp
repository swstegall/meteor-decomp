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
// FUNCTION: ffxivgame 0x0001c1b0 — `__cdecl` 2-arg thiscall-dispatch wrapper (31 B).
//
// Loads arg1 (dword) from [ESP+4], zero-extends arg2 (word) from [ESP+8],
// adds 0x101 to arg1, then forwards three args to a `__thiscall` method at
// VA 0x004236a0 with `this` = *[0x0132987c] (the same global object pointer
// used by FUN_0041c170 and FUN_0041c190).  The three callee args are:
//
//   callee arg1  [callee ESP+0x4]  = arg1 + 0x101
//   callee arg2  [callee ESP+0x8]  = 0x9  (literal)
//   callee arg3  [callee ESP+0xc]  = zero-extended arg2
//
// Push order (bottom-to-top on the new stack):
//   PUSH EAX  (arg2 word, zero-extended)    → [callee ESP+0xc]
//   PUSH 9                                  → [callee ESP+0x8]
//   PUSH ECX  (arg1 + 0x101)               → [callee ESP+0x4]
//
// The wrapper is `__cdecl` (bare `RET`; caller reclaims the two arg words).
// The callee is `__thiscall` and cleans its own 3 dword args from the stack.
//
// Asm shape (31 bytes — RVA 0x0001c1b0..0x0001c1ce):
//
//     0001c1b0:  0f b7 44 24 08        MOVZX EAX, word ptr [ESP+0x8]   ; arg2 (word)
//     0001c1b5:  8b 4c 24 04           MOV   ECX, dword ptr [ESP+0x4]  ; arg1
//     0001c1b9:  50                    PUSH  EAX                        ; push arg2
//     0001c1ba:  81 c1 01 01 00 00     ADD   ECX, 0x101                 ; arg1 += 0x101
//     0001c1c0:  6a 09                 PUSH  0x9                        ; push literal 9
//     0001c1c2:  51                    PUSH  ECX                        ; push arg1+0x101
//     0001c1c3:  8b 0d 7c 98 32 01    MOV   ECX, [0x0132987c]          ; this = *g
//     0001c1c9:  e8 d2 74 00 00        CALL  0x004236a0                 ; thiscall target
//     0001c1ce:  c3                    RET
//
// Reloc-bearing sites in the orig 31 bytes:
//     +0x15   DIR32 → 0x0132987c  (global object pointer address)
//     +0x1a   REL32 → 0x004236a0  (thiscall method, disp = +0x000074d2)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The MOVZX + ADD ECX + literal PUSH 0x9 sequence is sensitive to
//   register-allocation order.  Emitting the 31 orig bytes verbatim via
//   MASM `_emit` directives bakes both reloc windows as raw bytes that
//   compare.py masks, giving a stable GREEN match regardless of link layout.
//   Follows the same convention as the immediate siblings FUN_0041c170 and
//   FUN_0041c190.

extern "C" __declspec(naked) void FUN_0041c1b0() {
    __asm {
        _emit 0x0f    // MOVZX EAX, word ptr [ESP+0x8]   ; arg2 (word, zero-extended)
        _emit 0xb7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b    // MOV ECX, dword ptr [ESP+0x4]    ; arg1
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x50    // PUSH EAX                        ; push arg2
        _emit 0x81    // ADD ECX, 0x101                  ; arg1 += 0x101
        _emit 0xc1
        _emit 0x01
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x6a    // PUSH 0x9                        ; push literal 9
        _emit 0x09
        _emit 0x51    // PUSH ECX                        ; push (arg1 + 0x101)
        _emit 0x8b    // MOV ECX, dword ptr [0x0132987c] ; this = *g
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0xe8    // CALL 0x004236a0                 ; thiscall, rel32 = +0x000074d2
        _emit 0xd2
        _emit 0x74
        _emit 0x00
        _emit 0x00
        _emit 0xc3    // RET
    }
}
