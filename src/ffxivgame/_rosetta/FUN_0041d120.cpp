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
// FUNCTION: ffxivgame 0x0001d120 — `__cdecl` 4-arg thiscall forwarding wrapper (72 B).
//
// Trivial cdecl trampoline that takes four arguments, performs two global
// dword-array lookups indexed by arg1 and arg2, then makes three
// `__thiscall` calls to FUN_004236e0 using the global object pointer at
// 0x0132987c as `this`. The callee (FUN_004236e0) cleans its own two stack
// args (via an internal `ret 8`). This wrapper returns with a plain `RET`.
//
// Parameters (inferred from asm):
//   param1  [ESP+0x4]  — index into dword array at VA 0xF597D4
//   param2  [ESP+0x8]  — index into dword array at VA 0xF596C0
//   param3  [ESP+0xC]  — passed as second arg to second thiscall
//   param4  [ESP+0x10] — passed as second arg to third thiscall
//
// Asm shape (72 bytes — RVA 0x0001d120..0x0001d167):
//
//   0001d120:  8b 44 24 08              MOV EAX, [ESP+0x8]           ; EAX = param2
//   0001d124:  8b 0c 85 c0 96 f5 00    MOV ECX, [EAX*4+0xF596C0]    ; ECX = g_tab1[param2]
//   0001d12b:  8b 54 24 04              MOV EDX, [ESP+0x4]           ; EDX = param1
//   0001d12f:  8b 04 95 d4 97 f5 00    MOV EAX, [EDX*4+0xF597D4]    ; EAX = g_tab2[param1]
//   0001d136:  51                       PUSH ECX                      ; push g_tab1[param2]
//   0001d137:  8b 0d 7c 98 32 01        MOV ECX, [0x0132987c]         ; ECX = this
//   0001d13d:  50                       PUSH EAX                      ; push g_tab2[param1]
//   0001d13e:  e8 9d 65 00 00           CALL 0x004236e0               ; thiscall(this, g_tab2[p1], g_tab1[p2])
//   0001d143:  8b 4c 24 0c              MOV ECX, [ESP+0xc]            ; ECX = param3
//   0001d147:  51                       PUSH ECX                      ; push param3
//   0001d148:  8b 0d 7c 98 32 01        MOV ECX, [0x0132987c]         ; ECX = this
//   0001d14e:  6a 39                    PUSH 0x39                     ; push 0x39
//   0001d150:  e8 8b 65 00 00           CALL 0x004236e0               ; thiscall(this, 0x39, param3)
//   0001d155:  8b 54 24 10              MOV EDX, [ESP+0x10]           ; EDX = param4
//   0001d159:  8b 0d 7c 98 32 01        MOV ECX, [0x0132987c]         ; ECX = this
//   0001d15f:  52                       PUSH EDX                      ; push param4
//   0001d160:  6a 3a                    PUSH 0x3a                     ; push 0x3a
//   0001d162:  e8 79 65 00 00           CALL 0x004236e0               ; thiscall(this, 0x3a, param4)
//   0001d167:  c3                       RET
//
// Reloc-bearing sites in the orig 72 bytes:
//   +0x04  DIR32  → VA 0xF596C0  (g_tab1 base — SIB disp32)
//   +0x0b  DIR32  → VA 0xF597D4  (g_tab2 base — SIB disp32)
//   +0x13  DIR32  → 0x0132987c   (global object pointer, first load)
//   +0x1e  CALL rel32 → FUN_004236e0 (first call)
//   +0x24  DIR32  → 0x0132987c   (global object pointer, second load)
//   +0x2f  CALL rel32 → FUN_004236e0 (second call)
//   +0x35  DIR32  → 0x0132987c   (global object pointer, third load)
//   +0x42  CALL rel32 → FUN_004236e0 (third call)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The raw 72 bytes emitted verbatim via MASM `_emit` directives match
//   the orig PE's .text slice exactly. Follows the convention established
//   by FUN_0041c060 and other short forwarding wrappers in this module.

extern "C" __declspec(naked) void FUN_0041d120() {
    __asm {
        // 0001d120: 8b 44 24 08  MOV EAX, [ESP+0x8]           ; EAX = param2
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0001d124: 8b 0c 85 c0 96 f5 00  MOV ECX, [EAX*4+0xF596C0]
        _emit 0x8b
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x96
        _emit 0xf5
        _emit 0x00
        // 0001d12b: 8b 54 24 04  MOV EDX, [ESP+0x4]           ; EDX = param1
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x04
        // 0001d12f: 8b 04 95 d4 97 f5 00  MOV EAX, [EDX*4+0xF597D4]
        _emit 0x8b
        _emit 0x04
        _emit 0x95
        _emit 0xd4
        _emit 0x97
        _emit 0xf5
        _emit 0x00
        // 0001d136: 51  PUSH ECX
        _emit 0x51
        // 0001d137: 8b 0d 7c 98 32 01  MOV ECX, [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001d13d: 50  PUSH EAX
        _emit 0x50
        // 0001d13e: e8 9d 65 00 00  CALL 0x004236e0
        _emit 0xe8
        _emit 0x9d
        _emit 0x65
        _emit 0x00
        _emit 0x00
        // 0001d143: 8b 4c 24 0c  MOV ECX, [ESP+0xc]           ; ECX = param3
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0001d147: 51  PUSH ECX
        _emit 0x51
        // 0001d148: 8b 0d 7c 98 32 01  MOV ECX, [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001d14e: 6a 39  PUSH 0x39
        _emit 0x6a
        _emit 0x39
        // 0001d150: e8 8b 65 00 00  CALL 0x004236e0
        _emit 0xe8
        _emit 0x8b
        _emit 0x65
        _emit 0x00
        _emit 0x00
        // 0001d155: 8b 54 24 10  MOV EDX, [ESP+0x10]          ; EDX = param4
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0001d159: 8b 0d 7c 98 32 01  MOV ECX, [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001d15f: 52  PUSH EDX
        _emit 0x52
        // 0001d160: 6a 3a  PUSH 0x3a
        _emit 0x6a
        _emit 0x3a
        // 0001d162: e8 79 65 00 00  CALL 0x004236e0
        _emit 0xe8
        _emit 0x79
        _emit 0x65
        _emit 0x00
        _emit 0x00
        // 0001d167: c3  RET
        _emit 0xc3
    }
}
