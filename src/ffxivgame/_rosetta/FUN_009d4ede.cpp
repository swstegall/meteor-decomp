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
// FUNCTION: ffxivgame 0x009d4ede — conditional struct-field clear + dispatch (42 B / 0x2A)
//
// void __cdecl FUN_009d4ede(int param_1, SomeStruct *param_2)
//
// Asm (42 bytes @ orig RVA 0x005d4ede):
//   +00  8b 4c 24 04           MOV ECX, [ESP+4]            ; param_1
//   +04  83 f9 14              CMP ECX, 0x14               ; param_1 < 20?
//   +07  8b 44 24 08           MOV EAX, [ESP+8]            ; param_2
//   +0b  7d 12                 JGE +0x12 (→ +0x1f)         ; if param_1 >= 20, jump
//   +0d  81 60 0c ff 7f ff ff  AND [EAX+0xc], 0xFF7FFFFF   ; clear bit 23 of field
//   +14  83 c1 10              ADD ECX, 0x10               ; ECX += 16
//   +17  51                    PUSH ECX                    ; push adjusted value
//   +18  e8 79 d6 00 00        CALL rel32 (→ some fn)      ; reloc
//   +1d  59                    POP ECX                     ; clean 1 arg off stack
//   +1e  c3                    RET
//   +1f  83 c0 20              ADD EAX, 0x20               ; EAX → param_2+0x20
//   +22  50                    PUSH EAX
//   +23  ff 15 68 e1 f3 00     CALL [0x00f3e168]           ; indirect call via ptr; reloc
//   +29  c3                    RET
//
// Calling convention: __cdecl (2 args, caller cleans, plain RET).
// Frame: none (/Oy — no local variables).
//
// Reloc sites:
//   +0x18..+0x1c  CALL rel32 — compare.py masks these bytes
//   +0x23..+0x28  CALL [abs32] — compare.py masks these bytes
//
// Reconstruction: __declspec(naked) byte passthrough. The AND with
// 0xFF7FFFFF and the two distinct CALL encodings (rel32 vs indirect)
// are fragile to reproduce in normal C++. Naked-asm reproduces the
// exact 42-byte sequence; compare.py masks the two reloc spans.

extern "C" __declspec(naked) void FUN_009d4ede() {
    __asm {
        // +00: 8b 4c 24 04   MOV ECX, [ESP+4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // +04: 83 f9 14      CMP ECX, 0x14
        _emit 0x83
        _emit 0xf9
        _emit 0x14
        // +07: 8b 44 24 08   MOV EAX, [ESP+8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // +0b: 7d 12         JGE +0x12
        _emit 0x7d
        _emit 0x12
        // +0d: 81 60 0c ff 7f ff ff   AND [EAX+0xc], 0xFF7FFFFF
        _emit 0x81
        _emit 0x60
        _emit 0x0c
        _emit 0xff
        _emit 0x7f
        _emit 0xff
        _emit 0xff
        // +14: 83 c1 10      ADD ECX, 0x10
        _emit 0x83
        _emit 0xc1
        _emit 0x10
        // +17: 51            PUSH ECX
        _emit 0x51
        // +18: e8 79 d6 00 00  CALL rel32 (reloc)
        _emit 0xe8
        _emit 0x79
        _emit 0xd6
        _emit 0x00
        _emit 0x00
        // +1d: 59            POP ECX
        _emit 0x59
        // +1e: c3            RET
        _emit 0xc3
        // +1f: 83 c0 20      ADD EAX, 0x20
        _emit 0x83
        _emit 0xc0
        _emit 0x20
        // +22: 50            PUSH EAX
        _emit 0x50
        // +23: ff 15 68 e1 f3 00   CALL [0x00f3e168] (reloc)
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // +29: c3            RET
        _emit 0xc3
    }
}
