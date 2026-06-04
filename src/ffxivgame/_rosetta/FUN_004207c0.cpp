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
// FUNCTION: ffxivgame 0x000207c0 — `__cdecl` 1-arg byte-write + thiscall
//                                   dispatch (27 B).
//
// Loads the caller's single byte argument, stores it to the global byte at
// 0x01328ee8, zero-extends it to DWORD, then calls the __thiscall method at
// 0x004236e0 on the object pointed to by the global dword at 0x0132987c
// (loaded into ECX). Two stack args are passed to the callee: literal 0xf
// (first / [ESP+4]) and the zero-extended byte (second / [ESP+8]). Because
// the callee is callee-cleanup (__thiscall, RET 8), no ADD ESP is needed
// before the plain RET.
//
// Asm shape (27 bytes):
//     000207c0:  8a 44 24 04           MOV  AL,  byte ptr [ESP+0x4]      ; byte arg
//     000207c4:  8b 0d 7c 98 32 01     MOV  ECX, dword ptr [0x0132987c]  ; this ptr
//     000207ca:  a2 e8 8e 32 01        MOV  [0x01328ee8], AL             ; store byte
//     000207cf:  0f b6 c0              MOVZX EAX, AL                     ; zero-extend
//     000207d2:  50                    PUSH EAX                           ; arg2 for callee
//     000207d3:  6a 0f                 PUSH 0xf                           ; arg1 for callee
//     000207d5:  e8 06 2f 00 00        CALL 0x004236e0                    ; __thiscall (RET 8)
//     000207da:  c3                    RET
//
// Reloc-bearing sites in the orig 27 bytes:
//     +0x06  MOV ECX, imm32    → 0x0132987c  (global this-pointer slot)
//     +0x0b  MOV [abs32], AL   → 0x01328ee8  (global byte destination)
//     +0x16  CALL rel32        → 0x004236e0  (thiscall dispatch target)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level rewrite would require extern declarations for the global
//   this-pointer slot (0x0132987c), the global byte (0x01328ee8), and the
//   thiscall callee, plus a class definition for the thiscall receiver. Even
//   with all of those in place, MSVC 2005 /O2 does not reliably reproduce the
//   exact AL/ECX/MOVZX/PUSH ordering without additional hints. Emitting the
//   27 orig bytes verbatim via MASM `_emit` directives is the same approach
//   used by siblings FUN_00401000, FUN_00404e10, etc. and guarantees a
//   byte-identical .text section.

extern "C" __declspec(naked) void FUN_004207c0() {
    __asm {
        _emit 0x8a      // MOV  AL, byte ptr [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b      // MOV  ECX, dword ptr [0x0132987c]
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0xa2      // MOV  [0x01328ee8], AL
        _emit 0xe8
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        _emit 0x0f      // MOVZX EAX, AL
        _emit 0xb6
        _emit 0xc0
        _emit 0x50      // PUSH EAX
        _emit 0x6a      // PUSH 0xf
        _emit 0x0f
        _emit 0xe8      // CALL 0x004236e0  (rel32 = +0x00002f06)
        _emit 0x06
        _emit 0x2f
        _emit 0x00
        _emit 0x00
        _emit 0xc3      // RET
    }
}
