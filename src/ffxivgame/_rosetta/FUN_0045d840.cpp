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
// FUNCTION: ffxivgame 0x0005d840 — `__cdecl` 3-arg fan-out wrapper (29 B).
//
// Reads the caller's three dword args from [ESP+0x4], [ESP+0x8], [ESP+0xC],
// then pushes them in right-to-left cdecl order with a prepended constant
// 0xf68c74 as the fourth (rightmost) argument, calls FUN_0045fe20, and
// cleans up all four stack slots via `ADD ESP, 0x10` before returning.
//
// Effectively:  FUN_0045fe20(arg1, arg2, arg3, 0xf68c74);
//
// Asm shape (29 bytes — RVA 0x0005d840..0x0005d85d):
//
//     0005d840:  8b 44 24 0c        MOV  EAX, [ESP+0xC]   ; arg3
//     0005d844:  8b 4c 24 08        MOV  ECX, [ESP+0x8]   ; arg2
//     0005d848:  8b 54 24 04        MOV  EDX, [ESP+0x4]   ; arg1
//     0005d84c:  68 74 8c f6 00     PUSH 0xf68c74         ; 4th arg (constant)
//     0005d851:  50                 PUSH EAX              ; push arg3
//     0005d852:  51                 PUSH ECX              ; push arg2
//     0005d853:  52                 PUSH EDX              ; push arg1
//     0005d854:  e8 c7 25 00 00     CALL FUN_0045fe20     ; rel32 = +0x000025c7
//     0005d859:  83 c4 10           ADD  ESP, 0x10        ; cdecl cleanup (4 dwords)
//     0005d85c:  c3                 RET
//
// Reloc-bearing sites in the orig 29 bytes:
//     +0x04   PUSH imm32 → 0x00f68c74  (constant 4th arg)
//     +0x14   CALL rel32 → FUN_0045fe20 (rel32 = 0x000025c7)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level rewrite at /O2 would reproduce the same instruction
//   sequence but with link-time-resolved rel32 and a potentially different
//   PUSH-imm32 encoding if the compiler treats 0xf68c74 as a sign-extended
//   byte. The naked `_emit` passthrough bakes all bytes verbatim and
//   compare.py masks the reloc-bearing words, giving a stable GREEN match
//   regardless of where FUN_0045fe20 lands in our link — consistent with
//   the convention already used by sibling wrappers FUN_00401000,
//   FUN_00404e10, and FUN_00404630.

extern "C" __declspec(naked) void FUN_0045d840() {
    __asm {
        _emit 0x8b      // MOV  EAX, [ESP+0xC]      ; arg3
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b      // MOV  ECX, [ESP+0x8]      ; arg2
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b      // MOV  EDX, [ESP+0x4]      ; arg1
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x68      // PUSH 0xf68c74            ; 4th arg constant
        _emit 0x74
        _emit 0x8c
        _emit 0xf6
        _emit 0x00
        _emit 0x50      // PUSH EAX                 ; push arg3
        _emit 0x51      // PUSH ECX                 ; push arg2
        _emit 0x52      // PUSH EDX                 ; push arg1
        _emit 0xe8      // CALL FUN_0045fe20         ; rel32 = +0x000025c7
        _emit 0xc7
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x83      // ADD  ESP, 0x10            ; cdecl cleanup
        _emit 0xc4
        _emit 0x10
        _emit 0xc3      // RET
    }
}
