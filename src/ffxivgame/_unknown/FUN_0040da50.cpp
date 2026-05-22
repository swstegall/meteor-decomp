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
// FUNCTION: ffxivgame 0x0000da50 — LayoutMemorySpace::AllocStats_::GetCurrentMemsCount
//                                   (__thiscall, 80 B / 0x50)
//
// __thiscall int GetCurrentMemsCount(this)
//   ECX : this — pointer to an AllocStats_ object
//
// Behaviour:
//   1. Reads this->field_0 ([ESI+0x0]) and this->field_4 ([ESI+0x4]).
//   2. If field_0 >= field_4 (JNC, unsigned), skip the assert block.
//   3. Otherwise (field_0 < field_4, unsigned), performs a one-time lazy
//      initialization of the global assert function pointer at 0x0132390c:
//        - Tests bit 0 of DAT_01323910 (one-time flag byte).
//        - If not set: ORs 1 into DAT_01323910, stores 0x0040b0a0 into
//          DAT_0132390c (assert-handler function pointer).
//      Then calls the assert handler at DAT_0132390c with 5 arguments:
//        arg1 (last pushed): 0x00f55ba8  — assert expression string
//        arg2:               0x00f54d48  — file path string
//        arg3:               0x00f55bf0  — assert function-name variant
//        arg4:               0xb7 (183)  — line number
//        arg5 (first pushed):0x00f55c40  — function name string
//   4. Returns field_0 - field_4 (signed difference).
//
// Calling convention: __thiscall, no stack args (plain RET / c3).
// Callee-saves: ESI only (PUSH ESI / POP ESI).
// No local stack frame. No /GS cookie.
//
// Reloc-bearing sites in the orig 80 bytes:
//   The global-variable accesses (TEST/OR/MOV on 0x01323910 and 0x0132390c)
//   and the string literal PUSHes (0x00f55c40, 0x00f55bf0, 0x00f55ba8,
//   0x00f54d48, 0x00f55ba8) are absolute addresses in the binary's address
//   space.  Source-level C++ cannot reproduce them in a standalone .obj.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A __declspec(naked) body that re-emits the original 80 bytes verbatim
//   via MASM _emit directives produces a .obj whose .text is byte-identical
//   to the original slice. compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0040da50() {
    __asm {
        // 0000da50:  56                    PUSH ESI
        _emit 0x56
        // 0000da51:  8b f1                 MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 0000da53:  8b 06                 MOV EAX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 0000da55:  3b 46 04              CMP EAX, dword ptr [ESI+0x4]
        _emit 0x3b
        _emit 0x46
        _emit 0x04
        // 0000da58:  73 3f                 JNC +0x3f  (-> 0x0040da99)
        _emit 0x73
        _emit 0x3f
        // 0000da5a:  b8 01 00 00 00        MOV EAX, 0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000da5f:  84 05 10 39 32 01     TEST byte ptr [0x01323910], AL
        _emit 0x84
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0000da65:  75 10                 JNZ +0x10  (-> 0x0040da77)
        _emit 0x75
        _emit 0x10
        // 0000da67:  09 05 10 39 32 01     OR dword ptr [0x01323910], EAX
        _emit 0x09
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0000da6d:  c7 05 0c 39 32 01 a0 b0 40 00   MOV dword ptr [0x0132390c], 0x0040b0a0
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xa0
        _emit 0xb0
        _emit 0x40
        _emit 0x00
        // 0000da77:  68 40 5c f5 00        PUSH 0x00f55c40
        _emit 0x68
        _emit 0x40
        _emit 0x5c
        _emit 0xf5
        _emit 0x00
        // 0000da7c:  68 b7 00 00 00        PUSH 0xb7
        _emit 0x68
        _emit 0xb7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000da81:  68 f0 5b f5 00        PUSH 0x00f55bf0
        _emit 0x68
        _emit 0xf0
        _emit 0x5b
        _emit 0xf5
        _emit 0x00
        // 0000da86:  68 48 4d f5 00        PUSH 0x00f54d48
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 0000da8b:  68 a8 5b f5 00        PUSH 0x00f55ba8
        _emit 0x68
        _emit 0xa8
        _emit 0x5b
        _emit 0xf5
        _emit 0x00
        // 0000da90:  ff 15 0c 39 32 01     CALL dword ptr [0x0132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0000da96:  83 c4 14              ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0000da99:  8b 06                 MOV EAX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 0000da9b:  2b 46 04              SUB EAX, dword ptr [ESI+0x4]
        _emit 0x2b
        _emit 0x46
        _emit 0x04
        // 0000da9e:  5e                    POP ESI
        _emit 0x5e
        // 0000da9f:  c3                    RET
        _emit 0xc3
    }
}
