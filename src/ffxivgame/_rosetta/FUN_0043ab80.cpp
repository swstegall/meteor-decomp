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
// FUNCTION: ffxivgame 0x0003ab80 — __thiscall constructor that initialises an
//                                   array of 13 sub-objects (28 B each) via
//                                   `eh_vector_constructor_iterator', then
//                                   zeroes a trailing sentinel field (37 B / 0x25).
//
// Calling convention : __thiscall (ECX = this); no stack args; plain RET;
//                      returns this in EAX.
// Frame             : none (/Oy — only callee-save ESI used).
//
// Layout (offsets from `this'):
//   [+0x000 .. +0x16b]  13 x 28-byte elements (constructed by FUN_0043a6b0 /
//                        destructed by FUN_0043aa50)
//   [+0x16c]            sentinel DWORD zeroed after array construction
//                        (0x1c * 0xd = 28 * 13 = 364 = 0x16c ✓)
//
// The call to `eh_vector_constructor_iterator' (RVA 0x5d61d6, VA 0x009d61d6)
// cannot be referenced by its backtick name in C++; the 5-byte CALL rel32 is
// emitted verbatim via _emit and compare.py masks the 4-byte displacement.
//
// MSVC /O2 defers `MOV ESI, ECX' until after the first three argument pushes
// because those pushes do not reference ESI; the save is interleaved in the
// middle of the argument setup rather than placed at function entry.
//
// Context: FUN_0043b440 uses FUN_0043ab80 as the element-constructor pointer
// and FUN_0043abb0 as the element-destructor pointer when it itself calls
// `eh_vector_constructor_iterator' for a 6-element, 0x170-byte array.
//
// Asm (37 bytes @ orig RVA 0x0003ab80):
//   56                              PUSH ESI
//   68 50 aa 43 00                  PUSH &FUN_0043aa50    ; arg5 dtor (masked)
//   68 b0 a6 43 00                  PUSH &FUN_0043a6b0    ; arg4 ctor (masked)
//   6a 0d                           PUSH 0xd              ; arg3 count = 13
//   8b f1                           MOV  ESI, ECX         ; save this (deferred)
//   6a 1c                           PUSH 0x1c             ; arg2 elem size = 28
//   56                              PUSH ESI              ; arg1 pMem = this
//   e8 3f b6 59 00                  CALL eh_vector_constructor_iterator (masked)
//   c7 86 6c 01 00 00 00 00 00 00   MOV  dword ptr [ESI+0x16c], 0
//   8b c6                           MOV  EAX, ESI
//   5e                              POP  ESI
//   c3                              RET

extern "C" void FUN_0043a6b0();   // element constructor (thiscall, no stack args)
extern "C" void FUN_0043aa50();   // element destructor  (thiscall, no stack args)

extern "C" __declspec(naked) void FUN_0043ab80() {
    __asm {
        // 0003ab80:  56
        push esi
        // 0003ab81:  68 50 aa 43 00  — &FUN_0043aa50 (element dtor; reloc-masked)
        push OFFSET FUN_0043aa50
        // 0003ab86:  68 b0 a6 43 00  — &FUN_0043a6b0 (element ctor; reloc-masked)
        push OFFSET FUN_0043a6b0
        // 0003ab8b:  6a 0d           — count = 13
        push 0xd
        // 0003ab8d:  8b f1           — MOV ESI, ECX  (deferred /O2 save)
        mov  esi, ecx
        // 0003ab8f:  6a 1c           — element size = 28
        push 0x1c
        // 0003ab91:  56              — pMem = this
        push esi
        // 0003ab92:  e8 3f b6 59 00  — CALL `eh_vector_constructor_iterator' (reloc-masked)
        _emit 0xe8
        _emit 0x3f
        _emit 0xb6
        _emit 0x59
        _emit 0x00
        // 0003ab97:  c7 86 6c 01 00 00 00 00 00 00
        mov  dword ptr [esi + 0x16c], 0
        // 0003aba1:  8b c6
        mov  eax, esi
        // 0003aba3:  5e
        pop  esi
        // 0003aba4:  c3
        ret
    }
}
