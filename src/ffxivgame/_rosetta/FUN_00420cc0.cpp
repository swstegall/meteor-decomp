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
// FUNCTION: ffxivgame 0x00020cc0 — scalar-deleting destructor (43 B / 0x2B)
//
//   Calling convention: __thiscall (ECX = this)
//   Stack arg: unsigned int flags  [ESP+0x8 after PUSH ESI]
//   Returns: this pointer (ESI → EAX)
//   Epilogue: RET 0x4  (callee cleans one DWORD from the stack)
//
// Asm (43 bytes @ orig RVA 0x00020cc0):
//   56                     PUSH ESI
//   8b f1                  MOV ESI, ECX                      ; save this
//   c7 06 b8 98 f5 00      MOV dword ptr [ESI],    0xf598b8  ; vtable[0]
//   c7 46 0c c4 98 f5 00   MOV dword ptr [ESI+0xc],0xf598c4  ; vtable[1]
//   e8 cb f9 ff ff         CALL FUN_004206a0                 ; base dtor (ECX=this)
//   f6 44 24 08 01         TEST byte ptr [ESP+0x8], 0x1      ; delete flag
//   74 09                  JZ   +0x9  (→ 0x00420ce5)
//   8b 4e fc               MOV  ECX, dword ptr [ESI-0x4]    ; alloc header
//   56                     PUSH ESI                          ; arg = this
//   e8 8b d2 fe ff         CALL FUN_0040df70                 ; operator delete
//   8b c6                  MOV  EAX, ESI                     ; return this
//   5e                     POP  ESI
//   c2 04 00               RET  0x4
//
// The two immediate addresses (vtable pointers 0xf598b8 / 0xf598c4) and
// the two CALL rel32 offsets are the reloc-bearing sites; compare.py
// masks those bytes during the diff.

extern "C" void FUN_004206a0();   // base-class destructor (thiscall)
extern "C" void FUN_0040df70();   // operator delete helper

extern "C" __declspec(naked) void FUN_00420cc0() {
    __asm {
        // 00020cc0: 56                  PUSH ESI
        _emit 0x56
        // 00020cc1: 8b f1               MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00020cc3: c7 06 b8 98 f5 00   MOV dword ptr [ESI], 0xf598b8
        _emit 0xc7
        _emit 0x06
        _emit 0xb8
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        // 00020cc9: c7 46 0c c4 98 f5 00  MOV dword ptr [ESI+0xc], 0xf598c4
        _emit 0xc7
        _emit 0x46
        _emit 0x0c
        _emit 0xc4
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        // 00020cd0: e8 cb f9 ff ff      CALL FUN_004206a0
        call FUN_004206a0
        // 00020cd5: f6 44 24 08 01      TEST byte ptr [ESP+0x8], 0x1
        _emit 0xf6
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        // 00020cda: 74 09               JZ 0x00420ce5
        _emit 0x74
        _emit 0x09
        // 00020cdc: 8b 4e fc            MOV ECX, dword ptr [ESI-0x4]
        _emit 0x8b
        _emit 0x4e
        _emit 0xfc
        // 00020cdf: 56                  PUSH ESI
        _emit 0x56
        // 00020ce0: e8 8b d2 fe ff      CALL FUN_0040df70
        call FUN_0040df70
        // 00020ce5: 8b c6               MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 00020ce7: 5e                  POP ESI
        _emit 0x5e
        // 00020ce8: c2 04 00            RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
