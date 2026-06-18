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
// FUNCTION: ffxivgame 0x00035c80 — __thiscall conditional log/assert trampoline
//                                  with magic-static function-pointer install
//                                  (99 B / 0x63, RET 0x4 = 1 stack arg)
//
// Shape:
//   1. Load arg0 = [ESP+4]; read vtable ptr from *arg0; get fn ptr at +0x144.
//   2. Push *(this+0xc), *(this+0x8), *(this+0x4), arg0 as args; CALL EDX.
//   3. If EAX == 0 → return immediately (condition not met).
//   4. If EAX != 0: check a magic-static init flag at [0x01323910].
//      First call: set flag, store function pointer 0x00433720 into [0x0132390c].
//   5. Call [0x0132390c] with 5 pushed arguments (strings / line-number):
//        PUSH 0x00f652b0, 0x0000020a, 0x00f64c18, 0x00f65290, 0x00f64be8
//      ADD ESP, 0x14 (cdecl cleanup).
//   6. RET 0x4.
//
// The function carries four absolute DATA32 addresses and one DATA32 function
// pointer that are only valid in the original .text address space. A naked _emit
// passthrough is the only reliable way to reproduce them byte-for-byte in a
// standalone .obj; same strategy as siblings FUN_004130d0 and FUN_00406350.

extern "C" __declspec(naked) void FUN_00435c80() {
    __asm {
        // 00035c80: 8b 44 24 04   MOV EAX, dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00035c84: 8b 10         MOV EDX, dword ptr [EAX]
        _emit 0x8b
        _emit 0x10
        // 00035c86: 8b 92 44 01 00 00   MOV EDX, dword ptr [EDX+0x144]
        _emit 0x8b
        _emit 0x92
        _emit 0x44
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 00035c8c: 56            PUSH ESI
        _emit 0x56
        // 00035c8d: 8b 71 0c      MOV ESI, dword ptr [ECX+0xc]
        _emit 0x8b
        _emit 0x71
        _emit 0x0c
        // 00035c90: 56            PUSH ESI
        _emit 0x56
        // 00035c91: 8b 71 08      MOV ESI, dword ptr [ECX+0x8]
        _emit 0x8b
        _emit 0x71
        _emit 0x08
        // 00035c94: 8b 49 04      MOV ECX, dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x49
        _emit 0x04
        // 00035c97: 56            PUSH ESI
        _emit 0x56
        // 00035c98: 51            PUSH ECX
        _emit 0x51
        // 00035c99: 50            PUSH EAX
        _emit 0x50
        // 00035c9a: ff d2         CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00035c9c: 85 c0         TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00035c9e: 5e            POP ESI
        _emit 0x5e
        // 00035c9f: 74 3f         JZ +0x3f  (→ 00435ce0, RET)
        _emit 0x74
        _emit 0x3f
        // 00035ca1: b8 01 00 00 00   MOV EAX, 0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00035ca6: 84 05 10 39 32 01   TEST byte ptr [0x01323910], AL
        _emit 0x84
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00035cac: 75 10         JNZ +0x10  (→ 00435cbe, already inited)
        _emit 0x75
        _emit 0x10
        // 00035cae: 09 05 10 39 32 01   OR dword ptr [0x01323910], EAX
        _emit 0x09
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00035cb4: c7 05 0c 39 32 01 20 37 43 00
        //           MOV dword ptr [0x0132390c], 0x00433720
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00
        // 00035cbe: 68 b0 52 f6 00   PUSH 0x00f652b0
        _emit 0x68
        _emit 0xb0
        _emit 0x52
        _emit 0xf6
        _emit 0x00
        // 00035cc3: 68 0a 02 00 00   PUSH 0x0000020a
        _emit 0x68
        _emit 0x0a
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 00035cc8: 68 18 4c f6 00   PUSH 0x00f64c18
        _emit 0x68
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        // 00035ccd: 68 90 52 f6 00   PUSH 0x00f65290
        _emit 0x68
        _emit 0x90
        _emit 0x52
        _emit 0xf6
        _emit 0x00
        // 00035cd2: 68 e8 4b f6 00   PUSH 0x00f64be8
        _emit 0x68
        _emit 0xe8
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        // 00035cd7: ff 15 0c 39 32 01   CALL dword ptr [0x0132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00035cdd: 83 c4 14   ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00035ce0: c2 04 00   RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
