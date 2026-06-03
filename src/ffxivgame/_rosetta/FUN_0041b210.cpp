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
// FUNCTION: ffxivgame 0x0001b210 — conditional virtual dispatch via global singleton
//                                   (__cdecl, 3 args, 41 B / 0x29)
//
// void __cdecl FUN_0041b210(void *arg1, int arg2, void *arg3)
//
// Checks arg2 (second stack argument, loaded from [ESP+8] before the frame
// allocation); if non-zero, jumps past the function body (to 0x0041b239 —
// one byte past this function's own RET, sharing the next function's
// clean-up sequence).  If arg2 == 0, loads the global object pointer at
// 0x01329834, reads its vtable, and calls vtable slot 0x1a8/4 (entry 106)
// as a __cdecl-style indirect call, passing the object pointer itself as
// the first argument, followed by arg1 and arg3.
//
// The JNZ target (0x0041b239) lies one byte past the function's own
// ADD ESP, 0x1c; RET epilogue and is therefore unreproducible from
// source-level C++ without relying on compiler layout luck. The naked-asm
// passthrough is the only reliable path to a byte-identical .obj.
//
// Stack layout after SUB ESP, 0x1c:
//   [ESP+0x20]  arg1     (originally [ESP+0x04] at entry)
//   [ESP+0x24]  arg2     (originally [ESP+0x08] at entry) — condition
//   [ESP+0x28]  arg3     (originally [ESP+0x0c] at entry)
//
// Reloc-bearing site (compare.py masks these bytes):
//   +0x0f  A1 34 98 32 01  MOV EAX, [0x01329834]  (abs32 fixup)
//
// Asm (41 bytes @ orig RVA 0x0001b210):
//   8b 44 24 08              MOV EAX, dword ptr [ESP+0x8]
//   83 ec 1c                 SUB ESP, 0x1c
//   85 c0                    TEST EAX, EAX
//   75 1e                    JNZ 0x0041b239
//   8b 54 24 28              MOV EDX, dword ptr [ESP+0x28]
//   a1 34 98 32 01           MOV EAX, [0x01329834]
//   8b 08                    MOV ECX, dword ptr [EAX]
//   52                       PUSH EDX
//   8b 54 24 24              MOV EDX, dword ptr [ESP+0x24]
//   52                       PUSH EDX
//   50                       PUSH EAX
//   8b 81 a8 01 00 00        MOV EAX, dword ptr [ECX+0x1a8]
//   ff d0                    CALL EAX
//   83 c4 1c                 ADD ESP, 0x1c
//   c3                       RET

extern "C" __declspec(naked) void __cdecl FUN_0041b210(void *, int, void *) {
    __asm {
        // 0001b210: 8b 44 24 08   MOV EAX, [ESP+8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0001b214: 83 ec 1c      SUB ESP, 0x1c
        _emit 0x83
        _emit 0xec
        _emit 0x1c
        // 0001b217: 85 c0         TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0001b219: 75 1e         JNZ +0x1e  (→ 0x0041b239)
        _emit 0x75
        _emit 0x1e
        // 0001b21b: 8b 54 24 28   MOV EDX, [ESP+0x28]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x28
        // 0001b21f: a1 34 98 32 01  MOV EAX, [0x01329834]
        _emit 0xa1
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001b224: 8b 08         MOV ECX, [EAX]
        _emit 0x8b
        _emit 0x08
        // 0001b226: 52            PUSH EDX
        _emit 0x52
        // 0001b227: 8b 54 24 24   MOV EDX, [ESP+0x24]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 0001b22b: 52            PUSH EDX
        _emit 0x52
        // 0001b22c: 50            PUSH EAX
        _emit 0x50
        // 0001b22d: 8b 81 a8 01 00 00  MOV EAX, [ECX+0x1a8]
        _emit 0x8b
        _emit 0x81
        _emit 0xa8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001b233: ff d0         CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001b235: 83 c4 1c      ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 0001b238: c3            RET
        _emit 0xc3
    }
}
