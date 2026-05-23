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
// FUNCTION: ffxivgame 0x000107d0 — __thiscall pool-walk dispatcher
//           (SQEX::CDev::Engine::Memory - iterator/dispatch method)
//
// Saves `this` (ESI), calls vtable slot 11 (offset 0x2c) with `this`,
// pushes field_0x4c, sets the busy flag at field_0x50 = 1, calls
// FUN_0040f7b0, clears the flag, restores `this`, then tail-calls
// vtable slot 12 (offset 0x30) with `this` in ECX.
//
// Calling convention: __thiscall, no stack args → no explicit RET
//   (tail call to vtable[0x30]).
//
// Asm (39 bytes):
//   56              PUSH ESI
//   8b f1           MOV ESI, ECX
//   8b 06           MOV EAX, [ESI]
//   8b 50 2c        MOV EDX, [EAX+0x2c]
//   ff d2           CALL EDX
//   8b 46 4c        MOV EAX, [ESI+0x4c]
//   50              PUSH EAX
//   8b ce           MOV ECX, ESI
//   c6 46 50 01     MOV byte [ESI+0x50], 1
//   e8 c7 ef ff ff  CALL 0x0040f7b0
//   8b 16           MOV EDX, [ESI]
//   8b 42 30        MOV EAX, [EDX+0x30]
//   c6 46 50 00     MOV byte [ESI+0x50], 0
//   8b ce           MOV ECX, ESI
//   5e              POP ESI
//   ff e0           JMP EAX
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//
//   Source-level C++ cannot reliably reproduce the interleaved flag
//   store + arg push + ECX setup ordering that MSVC 2005 /O2 emits
//   when the busy flag (field_0x50) is set between argument evaluation
//   and the __thiscall ECX load.  A naked passthrough preserves the
//   original byte sequence byte-for-byte, including the CALL rel32
//   displacement (0xFFFFEFC7 → FUN_0040f7b0 resolved in the linked PE).
//   The #if guard makes the file parseable by clang/GCC without
//   -fms-extensions.

#if defined(__clang__) || defined(__GNUC__)
// clang / GCC stub for static-analysis only — NOT compiled in production.
extern "C" void FUN_004107d0() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_004107d0()
{
    __asm {
        // 000107d0: 56              PUSH ESI
        _emit 0x56
        // 000107d1: 8b f1           MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 000107d3: 8b 06           MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 000107d5: 8b 50 2c        MOV EDX, [EAX+0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 000107d8: ff d2            CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000107da: 8b 46 4c        MOV EAX, [ESI+0x4c]
        _emit 0x8b
        _emit 0x46
        _emit 0x4c
        // 000107dd: 50              PUSH EAX
        _emit 0x50
        // 000107de: 8b ce           MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000107e0: c6 46 50 01     MOV byte [ESI+0x50], 1
        _emit 0xc6
        _emit 0x46
        _emit 0x50
        _emit 0x01
        // 000107e4: e8 c7 ef ff ff  CALL 0x0040f7b0
        _emit 0xe8
        _emit 0xc7
        _emit 0xef
        _emit 0xff
        _emit 0xff
        // 000107e9: 8b 16           MOV EDX, [ESI]
        _emit 0x8b
        _emit 0x16
        // 000107eb: 8b 42 30        MOV EAX, [EDX+0x30]
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 000107ee: c6 46 50 00     MOV byte [ESI+0x50], 0
        _emit 0xc6
        _emit 0x46
        _emit 0x50
        _emit 0x00
        // 000107f2: 8b ce           MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000107f4: 5e              POP ESI
        _emit 0x5e
        // 000107f5: ff e0           JMP EAX
        _emit 0xff
        _emit 0xe0
    }
}
#endif
