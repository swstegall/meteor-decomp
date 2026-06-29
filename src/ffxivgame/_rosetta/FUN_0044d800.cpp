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
// FUNCTION: ffxivgame 0x0004d800 — guarded virtual dispatch, returns bool (38 B / 0x26)
//
// No arguments. Returns bool via AL.
// Calling convention: __cdecl, plain RET.
// Frame: none (/Oy — no locals, no callee-saves).
//
// Logic:
//   1. Load int from [0x01266dfc] into EAX.
//   2. If negative (JL), return false.
//   3. Load object pointer from [0x0132cf40] into ECX.
//   4. If NULL (JZ), return false.
//   5. Virtual dispatch through vtable slot [ECX][0x48/4]=18:
//        PUSH EAX (the int)
//        PUSH 0x0132cf60 (address of a global buffer, first arg)
//        CALL EAX ([vtable+0x48]); ECX remains the this pointer (__thiscall callee).
//   6. Return true (AL = 1).
//
// CALL EAX is a register-indirect call (ff d0) — no linker relocation.
// All global addresses are absolute 32-bit immediates embedded directly
// by _emit; compare.py sees them as literal bytes, matching the original.
//
// Asm (38 bytes @ orig RVA 0x0004d800):
//   a1 fc 6d 26 01          MOV EAX, [0x01266dfc]
//   85 c0                   TEST EAX, EAX
//   7c 1a                   JL  +0x1a  →  XOR AL,AL / RET
//   8b 0d 40 cf 32 01       MOV ECX, [0x0132cf40]
//   85 c9                   TEST ECX, ECX
//   74 10                   JZ  +0x10  →  XOR AL,AL / RET
//   8b 11                   MOV EDX, [ECX]
//   50                      PUSH EAX
//   8b 42 48                MOV EAX, [EDX+0x48]
//   68 60 cf 32 01          PUSH 0x0132cf60
//   ff d0                   CALL EAX
//   b0 01                   MOV AL, 0x1
//   c3                      RET
//   32 c0                   XOR AL, AL
//   c3                      RET

extern "C" __declspec(naked) bool __cdecl FUN_0044d800() {
    __asm {
        // 0004d800: a1 fc 6d 26 01   MOV EAX, [0x01266dfc]
        _emit 0xa1
        _emit 0xfc
        _emit 0x6d
        _emit 0x26
        _emit 0x01
        // 0004d805: 85 c0            TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0004d807: 7c 1a            JL +0x1a  (→ XOR AL,AL)
        _emit 0x7c
        _emit 0x1a
        // 0004d809: 8b 0d 40 cf 32 01  MOV ECX, [0x0132cf40]
        _emit 0x8b
        _emit 0x0d
        _emit 0x40
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // 0004d80f: 85 c9            TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 0004d811: 74 10            JZ +0x10  (→ XOR AL,AL)
        _emit 0x74
        _emit 0x10
        // 0004d813: 8b 11            MOV EDX, [ECX]
        _emit 0x8b
        _emit 0x11
        // 0004d815: 50               PUSH EAX
        _emit 0x50
        // 0004d816: 8b 42 48         MOV EAX, [EDX+0x48]
        _emit 0x8b
        _emit 0x42
        _emit 0x48
        // 0004d819: 68 60 cf 32 01   PUSH 0x0132cf60
        _emit 0x68
        _emit 0x60
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // 0004d81e: ff d0            CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0004d820: b0 01            MOV AL, 0x1
        _emit 0xb0
        _emit 0x01
        // 0004d822: c3               RET
        _emit 0xc3
        // 0004d823: 32 c0            XOR AL, AL
        _emit 0x32
        _emit 0xc0
        // 0004d825: c3               RET
        _emit 0xc3
    }
}
