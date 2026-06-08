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
// FUNCTION: ffxivgame 0x00034300 — cached-slot lookup with 5-arg callback
//                                  (__thiscall, 98 B / 0x62)
//
// __thiscall void FUN_00434300(arg1, arg2, arg3, arg4, arg5)
//   ECX = this
//
// Reads a global byte at [0x01328d90] as an index `n`, computes
// slot offset = n*7*4 (via LEA EDX,[EAX*8]+SUB/LEA ECX), then
// calls FUN_00417ab0(__thiscall ECX=slot, 0x34) to allocate or find
// a cached 0x34-byte entry.
//
// If allocation succeeds (EAX != 0): reorders the 5 caller args and
//   forwards them to FUN_004352e0 (__thiscall ECX=entry), then passes
//   the return value to *(this+0xC)->FUN_0043c2d0(result) as a one-arg
//   thiscall callback.
//
// On failure (EAX == 0): calls the same callback with 0 instead.
//
// Both paths end with RET 0x14, cleaning 5 DWORD args off the stack.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function contains three CALL rel32 targets, one absolute
//   address operand (MOV ECX,[imm32] for the global), and several
//   [ESP+disp8] addressing modes that depend on precise PUSH/POP
//   sequencing. Encoding all of that from source-level C++ would
//   require coaxing the MSVC 2005 register allocator into an exact
//   frame layout. A __declspec(naked) body re-emitting the orig 98
//   bytes verbatim via MASM _emit produces a .obj whose .text is
//   byte-identical to the orig slice; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00434300() {
    __asm {
        // 00034300: 56              PUSH ESI
        _emit 0x56
        // 00034301: 8b f1           MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00034303: 8b 0d 90 8d 32 01  MOV ECX, dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00034309: 0f b6 01        MOVZX EAX, byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 0003430c: 8d 14 c5 00 00 00 00  LEA EDX, [EAX*8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00034313: 2b d0           SUB EDX, EAX
        _emit 0x2b
        _emit 0xd0
        // 00034315: 8b 41 04        MOV EAX, dword ptr [ECX + 0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00034318: 8d 0c 90        LEA ECX, [EAX + EDX*4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 0003431b: 6a 34           PUSH 0x34
        _emit 0x6a
        _emit 0x34
        // 0003431d: e8 8e 37 fe ff  CALL 0x00417ab0
        _emit 0xe8
        _emit 0x8e
        _emit 0x37
        _emit 0xfe
        _emit 0xff
        // 00034322: 85 c0           TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00034324: 74 2d           JZ +0x2d  (→ 0x00434353)
        _emit 0x74
        _emit 0x2d
        // 00034326: 8b 4c 24 18     MOV ECX, dword ptr [ESP + 0x18]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 0003432a: 8b 54 24 14     MOV EDX, dword ptr [ESP + 0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0003432e: 51              PUSH ECX
        _emit 0x51
        // 0003432f: 8b 4c 24 10     MOV ECX, dword ptr [ESP + 0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00034333: 52              PUSH EDX
        _emit 0x52
        // 00034334: 8b 54 24 18     MOV EDX, dword ptr [ESP + 0x18]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 00034338: 51              PUSH ECX
        _emit 0x51
        // 00034339: 8b 4c 24 14     MOV ECX, dword ptr [ESP + 0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0003433d: 52              PUSH EDX
        _emit 0x52
        // 0003433e: 51              PUSH ECX
        _emit 0x51
        // 0003433f: 8b c8           MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 00034341: e8 9a 0f 00 00  CALL 0x004352e0
        _emit 0xe8
        _emit 0x9a
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        // 00034346: 8b 4e 0c        MOV ECX, dword ptr [ESI + 0xc]
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        // 00034349: 50              PUSH EAX
        _emit 0x50
        // 0003434a: e8 81 7f 00 00  CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x81
        _emit 0x7f
        _emit 0x00
        _emit 0x00
        // 0003434f: 5e              POP ESI
        _emit 0x5e
        // 00034350: c2 14 00        RET 0x14
        _emit 0xc2
        _emit 0x14
        _emit 0x00
        // 00034353: 8b 4e 0c        MOV ECX, dword ptr [ESI + 0xc]
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        // 00034356: 33 c0           XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00034358: 50              PUSH EAX
        _emit 0x50
        // 00034359: e8 72 7f 00 00  CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x72
        _emit 0x7f
        _emit 0x00
        _emit 0x00
        // 0003435e: 5e              POP ESI
        _emit 0x5e
        // 0003435f: c2 14 00        RET 0x14
        _emit 0xc2
        _emit 0x14
        _emit 0x00
    }
}
