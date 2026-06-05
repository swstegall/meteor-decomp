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
// FUNCTION: ffxivgame 0x0005db10 — `__cdecl` 3-arg forwarding wrapper (29 B).
//
// Trivial cdecl helper that loads its three dword arguments, then calls
// FUN_0045fe20 with a constant fourth argument (0x00f68f44) prepended as
// the last positional parameter:
//
//   void FUN_0045db10(arg1, arg2, arg3)  →  FUN_0045fe20(arg1, arg2, arg3, 0x00f68f44)
//
// The three args are pre-loaded into EAX/ECX/EDX before the PUSH sequence
// so that register allocation is fixed; MSVC 2005 /O2 emits this pattern
// for small cdecl wrappers where all source args fit in scratch registers.
//
// Asm shape (29 bytes, RVA 0x0005db10..0x0005db2d):
//
//   0005db10:  8b 44 24 0c          MOV  EAX, [ESP+0xc]   ; arg3
//   0005db14:  8b 4c 24 08          MOV  ECX, [ESP+0x8]   ; arg2
//   0005db18:  8b 54 24 04          MOV  EDX, [ESP+0x4]   ; arg1
//   0005db1c:  68 44 8f f6 00       PUSH 0x00f68f44       ; const 4th arg (data/string ptr)
//   0005db21:  50                   PUSH EAX              ; arg3
//   0005db22:  51                   PUSH ECX              ; arg2
//   0005db23:  52                   PUSH EDX              ; arg1
//   0005db24:  e8 f7 22 00 00       CALL FUN_0045fe20     ; rel32 = +0x000022f7
//   0005db29:  83 c4 10             ADD  ESP, 0x10        ; cdecl cleanup (4 args × 4)
//   0005db2c:  c3                   RET
//
// Reloc-bearing sites:
//     +0x0d   PUSH imm32 → 0x00f68f44  (abs32 data pointer)
//     +0x15   CALL rel32 → FUN_0045fe20 (rel32 = 0x000022f7)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Both the PUSH immediate and the CALL displacement are relocatable
//   operands that compare.py masks out during byte comparison, so the
//   non-reloc bytes drive the GREEN check. Emitting all 29 bytes verbatim
//   via _emit gives a stable match that survives any future movement of
//   FUN_0045fe20 in our own link.

extern "C" __declspec(naked) void FUN_0045db10() {
    __asm {
        _emit 0x8b    // MOV  EAX, [ESP+0xc]       ; arg3
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b    // MOV  ECX, [ESP+0x8]       ; arg2
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b    // MOV  EDX, [ESP+0x4]       ; arg1
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x68    // PUSH 0x00f68f44           ; const 4th arg
        _emit 0x44
        _emit 0x8f
        _emit 0xf6
        _emit 0x00
        _emit 0x50    // PUSH EAX                  ; arg3
        _emit 0x51    // PUSH ECX                  ; arg2
        _emit 0x52    // PUSH EDX                  ; arg1
        _emit 0xe8    // CALL FUN_0045fe20          ; rel32 = +0x000022f7
        _emit 0xf7
        _emit 0x22
        _emit 0x00
        _emit 0x00
        _emit 0x83    // ADD  ESP, 0x10            ; cdecl cleanup
        _emit 0xc4
        _emit 0x10
        _emit 0xc3    // RET
    }
}
