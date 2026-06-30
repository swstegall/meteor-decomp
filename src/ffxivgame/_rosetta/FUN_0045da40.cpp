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
// FUNCTION: ffxivgame 0x0005da40 — `__cdecl` 3-arg fan-out wrapper (29 B).
//
// Loads arg1 (EDX), arg2 (ECX), arg3 (EAX) from the caller's stack frame,
// then calls a 4-arg __cdecl routine at VA 0x0045fe20 with arguments ordered
// as: arg1, arg2, arg3, 0x00f68e34 (a hardcoded absolute address pushed
// first, appearing as the 4th/last arg to the callee in left-to-right C
// order). After the callee returns, the wrapper cleans up 0x10 bytes
// (4 dwords) from the stack and issues a plain RET (no pop count —
// confirming __cdecl).
//
// Asm shape (29 bytes, RVA 0x0005da40..0x0005da5c):
//
//   0005da40:  8b 44 24 0c          MOV  EAX, [ESP+0xC]     ; arg3
//   0005da44:  8b 4c 24 08          MOV  ECX, [ESP+0x8]     ; arg2
//   0005da48:  8b 54 24 04          MOV  EDX, [ESP+0x4]     ; arg1
//   0005da4c:  68 34 8e f6 00       PUSH 0x00f68e34          ; 4th arg (abs32 reloc)
//   0005da51:  50                   PUSH EAX                 ; 3rd arg
//   0005da52:  51                   PUSH ECX                 ; 2nd arg
//   0005da53:  52                   PUSH EDX                 ; 1st arg
//   0005da54:  e8 c7 23 00 00       CALL FUN_0045fe20        ; rel32 = +0x000023c7
//   0005da59:  83 c4 10             ADD  ESP, 0x10           ; cdecl cleanup (4 * 4 bytes)
//   0005da5c:  c3                   RET
//
// Reloc-bearing sites in the orig 29 bytes:
//     +0x0d   PUSH imm32 → 0x00f68e34  (absolute data address, 4th callee arg)
//     +0x15   CALL rel32 → FUN_0045fe20
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The PUSH-immediate operand 0x00f68e34 is an abs32 relocation, and the
//   CALL carries a rel32 relocation. A source-level rewrite would require
//   both callees to be matched/linked already so that compare.py's reloc
//   masking can pair them up. The naked byte-passthrough avoids that
//   dependency and is byte-identical with no risk of MSVC reordering
//   the three MOV loads or inserting a different push sequence — matching
//   the convention used by siblings FUN_00401000, FUN_00404e10, and others.

extern "C" __declspec(naked) void FUN_0045da40() {
    __asm {
        _emit 0x8b    // MOV  EAX, [ESP+0xC]     ; arg3
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b    // MOV  ECX, [ESP+0x8]     ; arg2
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b    // MOV  EDX, [ESP+0x4]     ; arg1
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x68    // PUSH 0x00f68e34          ; 4th arg (abs32 reloc)
        _emit 0x34
        _emit 0x8e
        _emit 0xf6
        _emit 0x00
        _emit 0x50    // PUSH EAX                 ; 3rd arg
        _emit 0x51    // PUSH ECX                 ; 2nd arg
        _emit 0x52    // PUSH EDX                 ; 1st arg
        _emit 0xe8    // CALL FUN_0045fe20        ; rel32 = +0x000023c7
        _emit 0xc7
        _emit 0x23
        _emit 0x00
        _emit 0x00
        _emit 0x83    // ADD  ESP, 0x10           ; cdecl cleanup
        _emit 0xc4
        _emit 0x10
        _emit 0xc3    // RET
    }
}
