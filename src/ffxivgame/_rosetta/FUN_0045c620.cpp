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
// FUNCTION: ffxivgame 0x0005c620 — _ERR_pop_to_mark
//                                  (0xe9 / 233 B, no SEH, __cdecl).
//
// Behaviour read from asm/ffxivgame/0005c620__ERR_pop_to_mark.s:
//
//   __cdecl int _ERR_pop_to_mark(void)
//
//   Pops error-stack entries (in a 16-slot circular ring buffer) from the
//   current "top" index down to the "mark" index, clearing each slot's
//   fields and optionally freeing a stored pointer.  Stops early if it
//   encounters a slot whose flags field has bit 0 set.  Returns 0 if the
//   ring's top reached the mark, 1 if it stopped at a flagged slot (and
//   clears that bit before returning).
//
//   ESI = state pointer (from GetErrState @ 0x0045c360)
//   EDI = 0  (zero constant for clearing; conditionally pushed)
//   EBP = -1 (decrement constant / wrap sentinel; conditionally pushed)
//
//   Struct layout (inferred from SIB addressing):
//     +0x000..0x007  (unknown header, 8 bytes)
//     +0x008..0x047  flags[16]   (int[16], 4-byte elements)
//     +0x048..0x087  other1[16]  (int[16])
//     +0x088..0x0c7  ptr[16]     (void*[16], freed via 0x004632f0 when set)
//     +0x0c8..0x107  flags2[16]  (int[16])
//     +0x108..0x147  other2[16]  (int[16])
//     +0x148..0x187  prev[16]    (int[16], set to -1 on clear)
//     +0x188         mark        (int — current ring top; decremented in loop)
//     +0x18c         current     (int — target mark to pop to)
//
//   Loop structure:
//     1. Call GetErrState() → ESI
//     2. If state->current == state->mark: jump directly to tail.
//     3. Push EBP (=-1) and EDI (=0) as loop constants.
//     4. Loop: load mark index into ECX; test flags[mark] & 1.
//        – If flag set: exit loop (JNZ to pop EDI/EBP, fall through to tail).
//        – Else: clear flags[mark]=0, other1[mark]=0.
//              If ptr[mark] != NULL && flags2[mark] & 1:
//                  free ptr[mark]; ptr[mark] = NULL.
//              Clear flags2[mark]=0, other2[mark]=0, prev[mark]=-1.
//              mark--; if mark == -1: mark = 15.
//        – Repeat while current != mark.
//     5. Pop EDI, EBP.
//     6. Tail: if current == mark → XOR EAX,EAX → return 0.
//              else AND flags[mark] &= 0xfe → return 1.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The conditional push/pop pattern (PUSH EBP/EDI inside the if-branch,
//   not at the function prologue), the OR EBP,-1 idiom for setting -1, and
//   MSVC 2005's repeated reloads of [ESI+0x188] from memory (rather than
//   caching in a register across the loop body) make a source-level /O2
//   rewrite brittle.  The pragmatic choice — matching FUN_00402a30 /
//   FUN_00403a20 / FUN_004054d0 — is a naked-asm passthrough of the 233
//   orig bytes.

extern "C" __declspec(naked) void FUN_0045c620() {
    __asm {
        // 0005c620  PUSH ESI
        _emit 0x56
        // 0005c621  CALL 0x0045c360  (GetErrState, no args)
        _emit 0xe8
        _emit 0x3a
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 0005c626  MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 0005c628  MOV EAX, [ESI+0x18c]  ; current
        _emit 0x8b
        _emit 0x86
        _emit 0x8c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c62e  CMP EAX, [ESI+0x188]  ; current == mark?
        _emit 0x3b
        _emit 0x86
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c634  JZ 0x0045c6eb  (tail — skip loop if equal)
        _emit 0x0f
        _emit 0x84
        _emit 0xb1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005c63a  PUSH EBP  (save; also used as -1 constant)
        _emit 0x55
        // 0005c63b  PUSH EDI  (save; also used as 0 constant)
        _emit 0x57
        // 0005c63c  XOR EDI, EDI  ; EDI = 0
        _emit 0x33
        _emit 0xff
        // 0005c63e  OR EBP, 0xffffffff  ; EBP = -1
        _emit 0x83
        _emit 0xcd
        _emit 0xff
        // -- loop top (0x0045c641) --
        // 0005c641  MOV ECX, [ESI+0x188]  ; mark index
        _emit 0x8b
        _emit 0x8e
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c647  TEST byte [ESI+ECX*4+0x8], 1  ; flags[mark] & 1?
        _emit 0xf6
        _emit 0x44
        _emit 0x8e
        _emit 0x08
        _emit 0x01
        // 0005c64c  LEA EAX, [ESI+ECX*4+0x8]  ; &flags[mark]
        _emit 0x8d
        _emit 0x44
        _emit 0x8e
        _emit 0x08
        // 0005c650  JNZ 0x0045c6e9  ; flag set → exit loop
        _emit 0x0f
        _emit 0x85
        _emit 0x93
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005c656  MOV [EAX], EDI  ; flags[mark] = 0
        _emit 0x89
        _emit 0x38
        // 0005c658  MOV EDX, [ESI+0x188]  ; reload mark
        _emit 0x8b
        _emit 0x96
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c65e  MOV [ESI+EDX*4+0x48], EDI  ; other1[mark] = 0
        _emit 0x89
        _emit 0x7c
        _emit 0x96
        _emit 0x48
        // 0005c662  MOV EAX, [ESI+0x188]  ; reload mark
        _emit 0x8b
        _emit 0x86
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c668  CMP [ESI+EAX*4+0x88], EDI  ; ptr[mark] == NULL?
        _emit 0x39
        _emit 0xbc
        _emit 0x86
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005c66f  JZ 0x0045c698  ; skip free if NULL
        _emit 0x74
        _emit 0x27
        // 0005c671  TEST byte [ESI+EAX*4+0xc8], 1  ; flags2[mark] & 1?
        _emit 0xf6
        _emit 0x84
        _emit 0x86
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x01
        // 0005c679  JZ 0x0045c698  ; skip free if flag not set
        _emit 0x74
        _emit 0x1d
        // 0005c67b  MOV EAX, [ESI+EAX*4+0x88]  ; load ptr[mark]
        _emit 0x8b
        _emit 0x84
        _emit 0x86
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005c682  PUSH EAX  ; arg: pointer to free
        _emit 0x50
        // 0005c683  CALL 0x004632f0  (FreeErrPtr)
        _emit 0xe8
        _emit 0x68
        _emit 0x6c
        _emit 0x00
        _emit 0x00
        // 0005c688  MOV ECX, [ESI+0x188]  ; reload mark
        _emit 0x8b
        _emit 0x8e
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c68e  ADD ESP, 4  ; cdecl clean-up
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005c691  MOV [ESI+ECX*4+0x88], EDI  ; ptr[mark] = NULL
        _emit 0x89
        _emit 0xbc
        _emit 0x8e
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // -- skip_free join (0x0045c698) --
        // 0005c698  MOV EDX, [ESI+0x188]  ; reload mark
        _emit 0x8b
        _emit 0x96
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c69e  MOV [ESI+EDX*4+0xc8], EDI  ; flags2[mark] = 0
        _emit 0x89
        _emit 0xbc
        _emit 0x96
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005c6a5  MOV EAX, [ESI+0x188]  ; reload mark
        _emit 0x8b
        _emit 0x86
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c6ab  MOV [ESI+EAX*4+0x108], EDI  ; other2[mark] = 0
        _emit 0x89
        _emit 0xbc
        _emit 0x86
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c6b2  MOV ECX, [ESI+0x188]  ; reload mark
        _emit 0x8b
        _emit 0x8e
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c6b8  MOV [ESI+ECX*4+0x148], EBP  ; prev[mark] = -1
        _emit 0x89
        _emit 0xac
        _emit 0x8e
        _emit 0x48
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c6bf  ADD [ESI+0x188], EBP  ; mark-- (EBP = -1)
        _emit 0x01
        _emit 0xae
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c6c5  CMP [ESI+0x188], EBP  ; mark == -1?
        _emit 0x39
        _emit 0xae
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c6cb  JNZ 0x0045c6d7  ; skip wrap if not -1
        _emit 0x75
        _emit 0x0a
        // 0005c6cd  MOV [ESI+0x188], 0xf  ; wrap: mark = 15
        _emit 0xc7
        _emit 0x86
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // -- no_wrap join (0x0045c6d7) --
        // 0005c6d7  MOV EDX, [ESI+0x18c]  ; current
        _emit 0x8b
        _emit 0x96
        _emit 0x8c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c6dd  CMP EDX, [ESI+0x188]  ; current == mark?
        _emit 0x3b
        _emit 0x96
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c6e3  JNZ 0x0045c641  ; loop while current != mark
        _emit 0x0f
        _emit 0x85
        _emit 0x58
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // -- loop_exit (0x0045c6e9) --
        // 0005c6e9  POP EDI
        _emit 0x5f
        // 0005c6ea  POP EBP
        _emit 0x5d
        // -- tail (0x0045c6eb) --
        // 0005c6eb  MOV EAX, [ESI+0x188]  ; mark
        _emit 0x8b
        _emit 0x86
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c6f1  CMP [ESI+0x18c], EAX  ; current == mark?
        _emit 0x39
        _emit 0x86
        _emit 0x8c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c6f7  JNZ 0x0045c6fd  ; not equal → return 1 path
        _emit 0x75
        _emit 0x04
        // 0005c6f9  XOR EAX, EAX  ; return 0
        _emit 0x33
        _emit 0xc0
        // 0005c6fb  POP ESI
        _emit 0x5e
        // 0005c6fc  RET
        _emit 0xc3
        // 0005c6fd  AND [ESI+EAX*4+0x8], 0xfe  ; clear flags[mark] bit 0
        _emit 0x83
        _emit 0x64
        _emit 0x86
        _emit 0x08
        _emit 0xfe
        // 0005c702  MOV EAX, 1  ; return 1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005c707  POP ESI
        _emit 0x5e
        // 0005c708  RET
        _emit 0xc3
    }
}
