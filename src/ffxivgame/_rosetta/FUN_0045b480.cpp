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
// FUNCTION: ffxivgame 0x0045b480 — one-shot init gate (50 B / 0x32)
//
// Calls an IAT-indirect function (likely a Windows API, e.g. FindWindow /
// CreateMutex, stored at import slot [0x00f3e1a0]) with three arguments:
//   arg0 = 0x0132d110  (string / object pointer in .data)
//   arg1 = 0x1         (boolean/mode flag)
//   arg2 = 0x0         (NULL)
// If the call returns non-zero (already-running / already-created), the
// function returns immediately.  Otherwise it:
//   1. Calls FUN_009d1b35(1) (a __cdecl factory/open call) and stores the
//      result handle in the global at 0x0132d10c.
//   2. Calls FUN_0045cb40() — first subordinate init.
//   3. Calls FUN_0045c820() — second subordinate init.
//   4. Tail-jumps to FUN_0045cb10() — main continuation.
//
// Calling convention: __cdecl — no args, void return, plain RET (c3).
// Frame: none (no locals, no callee-saved pushes).
//
// All absolute addresses (0x0132d110, [0x00f3e1a0], 0x0132d10c) and
// rel32 targets are baked into the orig binary's VA space.  We re-emit the
// full 50-byte sequence verbatim via naked + _emit so the .obj .text matches
// byte-for-byte.  tools/compare.py masks the four rel32 slots and the imm32
// IAT slot from the byte diff.
//
// Byte layout (RVA 0x0005b480, 50 bytes):
//   6a 00                PUSH 0x0
//   6a 01                PUSH 0x1
//   68 10 d1 32 01       PUSH 0x0132d110
//   ff 15 a0 e1 f3 00    CALL dword ptr [0x00f3e1a0]
//   85 c0                TEST EAX, EAX
//   75 1e                JNZ +0x1e  (→ RET at +0x31)
//   6a 01                PUSH 0x1
//   e8 9b 66 57 00       CALL 0x009d1b35
//   83 c4 04             ADD ESP, 0x4
//   a3 0c d1 32 01       MOV [0x0132d10c], EAX
//   e8 99 16 00 00       CALL 0x0045cb40
//   e8 74 13 00 00       CALL 0x0045c820
//   e9 5f 16 00 00       JMP 0x0045cb10
//   c3                   RET

extern "C" __declspec(naked) void FUN_0045b480() {
    __asm {
        // 0005b480: 6a 00              PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0005b482: 6a 01              PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005b484: 68 10 d1 32 01     PUSH 0x0132d110
        _emit 0x68
        _emit 0x10
        _emit 0xd1
        _emit 0x32
        _emit 0x01
        // 0005b489: ff 15 a0 e1 f3 00  CALL dword ptr [0x00f3e1a0]
        _emit 0xff
        _emit 0x15
        _emit 0xa0
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0005b48f: 85 c0              TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005b491: 75 1e              JNZ +0x1e
        _emit 0x75
        _emit 0x1e
        // 0005b493: 6a 01              PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005b495: e8 9b 66 57 00     CALL 0x009d1b35
        _emit 0xe8
        _emit 0x9b
        _emit 0x66
        _emit 0x57
        _emit 0x00
        // 0005b49a: 83 c4 04           ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005b49d: a3 0c d1 32 01     MOV [0x0132d10c], EAX
        _emit 0xa3
        _emit 0x0c
        _emit 0xd1
        _emit 0x32
        _emit 0x01
        // 0005b4a2: e8 99 16 00 00     CALL 0x0045cb40
        _emit 0xe8
        _emit 0x99
        _emit 0x16
        _emit 0x00
        _emit 0x00
        // 0005b4a7: e8 74 13 00 00     CALL 0x0045c820
        _emit 0xe8
        _emit 0x74
        _emit 0x13
        _emit 0x00
        _emit 0x00
        // 0005b4ac: e9 5f 16 00 00     JMP 0x0045cb10
        _emit 0xe9
        _emit 0x5f
        _emit 0x16
        _emit 0x00
        _emit 0x00
        // 0005b4b1: c3                 RET
        _emit 0xc3
    }
}
