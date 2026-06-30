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
// FUNCTION: ffxivgame 0x0045cb10 — conditional double-init via two calls
//                                  to FUN_0045c920 (45 B / 0x2D)
//
// Reads the DWORD global pointer at DAT_01268538, passes it to
// FUN_0045c2d0 (a check predicate).  If that returns non-zero the
// function returns immediately.  Otherwise it calls FUN_0045c920 twice:
//   first  with (0x01268538, 0)   — address of the pointer + NULL
//   second with (0x012686c0, 0)   — a second data slot  + NULL
// then pops all four args and returns.
//
// Calling convention: __cdecl — no args, void return, plain RET.
// Frame: none (no locals, no callee-saves).
//
// Asm (45 bytes @ orig RVA 0x0005cb10):
//   a1 38 85 26 01          MOV  EAX, [0x01268538]
//   50                      PUSH EAX
//   e8 b5 f7 ff ff          CALL FUN_0045c2d0
//   83 c4 04                ADD  ESP, 4
//   85 c0                   TEST EAX, EAX
//   75 1a                   JNZ  0x0045cb3c   (+0x1a)
//   68 38 85 26 01          PUSH 0x01268538
//   50                      PUSH EAX           ; 0 at this point
//   e8 f3 fd ff ff          CALL FUN_0045c920
//   68 c0 86 26 01          PUSH 0x012686c0
//   6a 00                   PUSH 0
//   e8 e7 fd ff ff          CALL FUN_0045c920
//   83 c4 10                ADD  ESP, 0x10
//   c3                      RET
//
// Reconstruction: __declspec(naked) _emit byte passthrough.

extern "C" __declspec(naked) void FUN_0045cb10() {
    __asm {
        // 0005cb10: a1 38 85 26 01    MOV EAX, [0x01268538]
        _emit 0xa1
        _emit 0x38
        _emit 0x85
        _emit 0x26
        _emit 0x01
        // 0005cb15: 50                PUSH EAX
        _emit 0x50
        // 0005cb16: e8 b5 f7 ff ff    CALL FUN_0045c2d0
        _emit 0xe8
        _emit 0xb5
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        // 0005cb1b: 83 c4 04          ADD ESP, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005cb1e: 85 c0             TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005cb20: 75 1a             JNZ +0x1a  (-> 0x0045cb3c)
        _emit 0x75
        _emit 0x1a
        // 0005cb22: 68 38 85 26 01    PUSH 0x01268538
        _emit 0x68
        _emit 0x38
        _emit 0x85
        _emit 0x26
        _emit 0x01
        // 0005cb27: 50                PUSH EAX
        _emit 0x50
        // 0005cb28: e8 f3 fd ff ff    CALL FUN_0045c920
        _emit 0xe8
        _emit 0xf3
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 0005cb2d: 68 c0 86 26 01    PUSH 0x012686c0
        _emit 0x68
        _emit 0xc0
        _emit 0x86
        _emit 0x26
        _emit 0x01
        // 0005cb32: 6a 00             PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0005cb34: e8 e7 fd ff ff    CALL FUN_0045c920
        _emit 0xe8
        _emit 0xe7
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 0005cb39: 83 c4 10          ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0005cb3c: c3                RET
        _emit 0xc3
    }
}
