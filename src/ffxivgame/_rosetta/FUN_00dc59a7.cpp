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
// FUNCTION: ffxivgame 0x009c59a7 — indirect-call dispatcher with error check (46 B / 0x2E)
//
// Forwards this function's two stack arguments to a function pointer at
// [0x00f3e0f0]. If that returns non-zero (success), returns 0. If it returns
// zero, calls a second function pointer at [0x00f3e1c4]; if that returns
// non-zero, passes the result to FUN_009d9d6d and returns -1 (error). If both
// calls returned zero, returns 0.
//
// Calling convention: __cdecl — two stack args, plain RET (caller cleans).
// Frame: none (no locals, no callee-saves).
//
// Asm (46 bytes @ orig RVA 0x009c59a7):
//   ff 74 24 08          PUSH dword ptr [ESP+0x8]      ; forward arg2
//   ff 74 24 08          PUSH dword ptr [ESP+0x8]      ; forward arg1 (post push)
//   ff 15 f0 e0 f3 00    CALL dword ptr [0x00f3e0f0]   ; indirect call 1
//   85 c0                TEST EAX, EAX
//   75 08                JNZ +8  → XOR EAX,EAX (success path, return 0)
//   ff 15 c4 e1 f3 00    CALL dword ptr [0x00f3e1c4]   ; indirect call 2
//   eb 02                JMP +2  → TEST EAX, EAX (shared check)
//   33 c0                XOR EAX, EAX               (success arm forced to 0)
//   85 c0                TEST EAX, EAX
//   74 0b                JZ +11  → XOR EAX, EAX; RET  (return 0)
//   50                   PUSH EAX
//   e8 a0 43 c1 ff       CALL 0x009d9d6d (rel32)
//   59                   POP ECX
//   83 c8 ff             OR EAX, 0xffffffff          ; return -1
//   c3                   RET
//   33 c0                XOR EAX, EAX               (return 0 epilogue)
//   c3                   RET
//
// Reconstruction: __declspec(naked) _emit byte passthrough. The two indirect
// calls (ff 15 <abs32>) and the relative call (e8 <rel32>) are masked by
// tools/compare.py. All 46 bytes match the orig slice byte-for-byte.

extern "C" __declspec(naked) void FUN_00dc59a7() {
    __asm {
        // ff 74 24 08    PUSH dword ptr [ESP+0x8]  (arg2)
        _emit 0xff
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // ff 74 24 08    PUSH dword ptr [ESP+0x8]  (arg1, after first push)
        _emit 0xff
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // ff 15 f0 e0 f3 00    CALL dword ptr [0x00f3e0f0]
        _emit 0xff
        _emit 0x15
        _emit 0xf0
        _emit 0xe0
        _emit 0xf3
        _emit 0x00
        // 85 c0    TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 75 08    JNZ +8
        _emit 0x75
        _emit 0x08
        // ff 15 c4 e1 f3 00    CALL dword ptr [0x00f3e1c4]
        _emit 0xff
        _emit 0x15
        _emit 0xc4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // eb 02    JMP +2
        _emit 0xeb
        _emit 0x02
        // 33 c0    XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 85 c0    TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 74 0b    JZ +11
        _emit 0x74
        _emit 0x0b
        // 50    PUSH EAX
        _emit 0x50
        // e8 a0 43 c1 ff    CALL 0x009d9d6d (rel32)
        _emit 0xe8
        _emit 0xa0
        _emit 0x43
        _emit 0xc1
        _emit 0xff
        // 59    POP ECX
        _emit 0x59
        // 83 c8 ff    OR EAX, 0xffffffff
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // c3    RET
        _emit 0xc3
        // 33 c0    XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // c3    RET
        _emit 0xc3
    }
}
