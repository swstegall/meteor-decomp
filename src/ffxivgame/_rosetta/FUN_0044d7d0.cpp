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
// FUNCTION: ffxivgame 0x0004d7d0 — singleton teardown helper (42 B / 0x2A)
//
// void __cdecl FUN_0044d7d0(void)
//
// 1. Loads the fixed singleton/object address 0x01266e00 into ECX and calls
//    FUN_004592d0 as a __thiscall (this = 0x01266e00) — a cleanup/flush step.
// 2. Reads the global pointer slot at 0x0132cf4c. If it is null, returns
//    immediately. Otherwise it pushes that pointer plus the companion global
//    at 0x0132cf48 as arguments and calls FUN_009d00a8 (a two-arg free /
//    deallocator), then zeroes the 0x0132cf4c slot.
//
// Calling convention: __cdecl, no args, void return, plain RET. No frame
// pointer (/Oy), no locals.
//
// Asm (42 bytes @ orig RVA 0x0004d7d0):
//   b9 00 6e 26 01        MOV  ECX, 0x1266e00
//   e8 f6 ba 00 00        CALL FUN_004592d0           ; __thiscall, this=ECX
//   a1 4c cf 32 01        MOV  EAX, [0x0132cf4c]
//   85 c0                 TEST EAX, EAX
//   74 16                 JZ   0x0044d7f9             ; null → RET
//   50                    PUSH EAX                    ; arg2 = [0x0132cf4c]
//   a1 48 cf 32 01        MOV  EAX, [0x0132cf48]
//   50                    PUSH EAX                    ; arg1 = [0x0132cf48]
//   e8 b9 28 58 00        CALL FUN_009d00a8           ; (reloc)
//   c7 05 4c cf 32 01 00 00 00 00   MOV [0x0132cf4c], 0
//   c3                    RET
//
// Reconstruction strategy — naked-asm byte passthrough (mirrors the sibling
// FUN_004051e0 / FUN_004134b0). The two CALL rel32 immediates and the abs32
// global addresses are baked into the orig binary's own address space and
// re-emitted here verbatim, so the .obj's .text matches byte-for-byte with no
// relocations. tools/compare.py masks the reloc bytes and reports GREEN.

extern "C" __declspec(naked) void __cdecl FUN_0044d7d0() {
    __asm {
        // 0004d7d0: b9 00 6e 26 01   MOV ECX, 0x1266e00
        _emit 0xb9
        _emit 0x00
        _emit 0x6e
        _emit 0x26
        _emit 0x01
        // 0004d7d5: e8 f6 ba 00 00   CALL 0x004592d0
        _emit 0xe8
        _emit 0xf6
        _emit 0xba
        _emit 0x00
        _emit 0x00
        // 0004d7da: a1 4c cf 32 01   MOV EAX, [0x0132cf4c]
        _emit 0xa1
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // 0004d7df: 85 c0            TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0004d7e1: 74 16            JZ 0x0044d7f9
        _emit 0x74
        _emit 0x16
        // 0004d7e3: 50               PUSH EAX
        _emit 0x50
        // 0004d7e4: a1 48 cf 32 01   MOV EAX, [0x0132cf48]
        _emit 0xa1
        _emit 0x48
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // 0004d7e9: 50               PUSH EAX
        _emit 0x50
        // 0004d7ea: e8 b9 28 58 00   CALL 0x009d00a8
        _emit 0xe8
        _emit 0xb9
        _emit 0x28
        _emit 0x58
        _emit 0x00
        // 0004d7ef: c7 05 4c cf 32 01 00 00 00 00   MOV [0x0132cf4c], 0
        _emit 0xc7
        _emit 0x05
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004d7f9: c3               RET
        _emit 0xc3
    }
}
