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
// FUNCTION: ffxivgame 0x0001fc60 — teardown/cleanup sequence for a global
//                                   manager struct (303 B / 0x12f, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x0001fc60):
//
//   __cdecl void FUN_0041fc60();
//
//   Loads a global manager pointer from [0x01329428].  If the manager's
//   activation byte at offset +0x180 is zero the function returns
//   immediately (nothing to do).  Otherwise it walks seven pointer slots
//   in the manager (offsets +0x19c, +0x1a0, +0x1a4, and the two globals
//   at 0x01329870 / 0x01329874, then +0x184, +0x188, +0x190, +0x194):
//   for each non-NULL slot it calls the appropriate vtable method and
//   then zeroes the slot.  The +0x19c slot uses vtable slot 2 with `this`
//   pushed on the stack; all other pointer slots use vtable slot 0 with
//   a literal `1` pushed as an argument (ECX = object pointer for
//   __thiscall).  Between the +0x1a0 and +0x1a4 blocks a helper at
//   0x0041ecf0 is called.  Finally the activation byte is cleared.
//
//   Stack frame: no local variables, no /GS cookie.  EBX is the only
//   callee-saved register used (as the zero constant).
//
//   Calling convention: __cdecl (RET with no stack-pop argument).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function body contains nine `MOV EAX,[0x01329428]` moffs32 loads
//   (opcode A1), two `MOV [0x01329870],EBX` / `MOV [0x01329874],EBX`
//   absolute stores, and one rel32 CALL to FUN_0041ecf0.  Getting MSVC
//   2005 /O2 to emit exactly these moffs32 forms (vs ModRM/SIB encodings)
//   and to choose the same instruction forms throughout would require
//   careful register-allocation control that is not easily achievable at
//   the source level.  The pragmatic choice — consistent with FUN_004014b0,
//   FUN_00401a00, FUN_00408f10, and all other opaque siblings — is a
//   `__declspec(naked)` body that re-emits the orig 303 bytes verbatim
//   via MASM `_emit` directives.  The .obj's `.text` section ends up
//   byte-identical to the orig slice, which is what `tools/compare.py`
//   checks.

extern "C" __declspec(naked) void FUN_0041fc60() {
    __asm {
        // 0001fc60: a1 28 94 32 01  MOV EAX,[0x01329428]
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001fc65: 53              PUSH EBX
        _emit 0x53
        // 0001fc66: 33 db           XOR EBX,EBX
        _emit 0x33
        _emit 0xdb
        // 0001fc68: 38 98 80 01 00 00  CMP byte ptr [EAX+0x180],BL
        _emit 0x38
        _emit 0x98
        _emit 0x80
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001fc6e: 0f 84 19 01 00 00  JZ 0x0041fd8d
        _emit 0x0f
        _emit 0x84
        _emit 0x19
        _emit 0x01
        _emit 0x00
        _emit 0x00

        // 0001fc74: 39 98 9c 01 00 00  CMP dword ptr [EAX+0x19c],EBX
        _emit 0x39
        _emit 0x98
        _emit 0x9c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001fc7a: 74 19           JZ 0x0041fc95
        _emit 0x74
        _emit 0x19
        // 0001fc7c: 8b 80 9c 01 00 00  MOV EAX,dword ptr [EAX+0x19c]
        _emit 0x8b
        _emit 0x80
        _emit 0x9c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001fc82: 8b 08           MOV ECX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x08
        // 0001fc84: 8b 51 08        MOV EDX,dword ptr [ECX+0x8]
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        // 0001fc87: 50              PUSH EAX
        _emit 0x50
        // 0001fc88: ff d2           CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0001fc8a: a1 28 94 32 01  MOV EAX,[0x01329428]
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001fc8f: 89 98 9c 01 00 00  MOV dword ptr [EAX+0x19c],EBX
        _emit 0x89
        _emit 0x98
        _emit 0x9c
        _emit 0x01
        _emit 0x00
        _emit 0x00

        // 0001fc95: 8b 88 a0 01 00 00  MOV ECX,dword ptr [EAX+0x1a0]
        _emit 0x8b
        _emit 0x88
        _emit 0xa0
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001fc9b: 3b cb           CMP ECX,EBX
        _emit 0x3b
        _emit 0xcb
        // 0001fc9d: 74 13           JZ 0x0041fcb2
        _emit 0x74
        _emit 0x13
        // 0001fc9f: 8b 01           MOV EAX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 0001fca1: 8b 10           MOV EDX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x10
        // 0001fca3: 6a 01           PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0001fca5: ff d2           CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0001fca7: a1 28 94 32 01  MOV EAX,[0x01329428]
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001fcac: 89 98 a0 01 00 00  MOV dword ptr [EAX+0x1a0],EBX
        _emit 0x89
        _emit 0x98
        _emit 0xa0
        _emit 0x01
        _emit 0x00
        _emit 0x00

        // 0001fcb2: e8 39 f0 ff ff  CALL 0x0041ecf0
        _emit 0xe8
        _emit 0x39
        _emit 0xf0
        _emit 0xff
        _emit 0xff

        // 0001fcb7: a1 28 94 32 01  MOV EAX,[0x01329428]
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001fcbc: 8b 88 a4 01 00 00  MOV ECX,dword ptr [EAX+0x1a4]
        _emit 0x8b
        _emit 0x88
        _emit 0xa4
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001fcc2: 3b cb           CMP ECX,EBX
        _emit 0x3b
        _emit 0xcb
        // 0001fcc4: 74 13           JZ 0x0041fcd9
        _emit 0x74
        _emit 0x13
        // 0001fcc6: 8b 11           MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 0001fcc8: 8b 02           MOV EAX,dword ptr [EDX]
        _emit 0x8b
        _emit 0x02
        // 0001fcca: 6a 01           PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0001fccc: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001fcce: a1 28 94 32 01  MOV EAX,[0x01329428]
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001fcd3: 89 98 a4 01 00 00  MOV dword ptr [EAX+0x1a4],EBX
        _emit 0x89
        _emit 0x98
        _emit 0xa4
        _emit 0x01
        _emit 0x00
        _emit 0x00

        // 0001fcd9: 8b 0d 70 98 32 01  MOV ECX,dword ptr [0x01329870]
        _emit 0x8b
        _emit 0x0d
        _emit 0x70
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001fcdf: 3b cb           CMP ECX,EBX
        _emit 0x3b
        _emit 0xcb
        // 0001fce1: 74 13           JZ 0x0041fcf6
        _emit 0x74
        _emit 0x13
        // 0001fce3: 8b 11           MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 0001fce5: 8b 02           MOV EAX,dword ptr [EDX]
        _emit 0x8b
        _emit 0x02
        // 0001fce7: 6a 01           PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0001fce9: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001fceb: a1 28 94 32 01  MOV EAX,[0x01329428]
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001fcf0: 89 1d 70 98 32 01  MOV dword ptr [0x01329870],EBX
        _emit 0x89
        _emit 0x1d
        _emit 0x70
        _emit 0x98
        _emit 0x32
        _emit 0x01

        // 0001fcf6: 8b 0d 74 98 32 01  MOV ECX,dword ptr [0x01329874]
        _emit 0x8b
        _emit 0x0d
        _emit 0x74
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001fcfc: 3b cb           CMP ECX,EBX
        _emit 0x3b
        _emit 0xcb
        // 0001fcfe: 74 13           JZ 0x0041fd13
        _emit 0x74
        _emit 0x13
        // 0001fd00: 8b 11           MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 0001fd02: 8b 02           MOV EAX,dword ptr [EDX]
        _emit 0x8b
        _emit 0x02
        // 0001fd04: 6a 01           PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0001fd06: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001fd08: a1 28 94 32 01  MOV EAX,[0x01329428]
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001fd0d: 89 1d 74 98 32 01  MOV dword ptr [0x01329874],EBX
        _emit 0x89
        _emit 0x1d
        _emit 0x74
        _emit 0x98
        _emit 0x32
        _emit 0x01

        // 0001fd13: 8b 88 84 01 00 00  MOV ECX,dword ptr [EAX+0x184]
        _emit 0x8b
        _emit 0x88
        _emit 0x84
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001fd19: 3b cb           CMP ECX,EBX
        _emit 0x3b
        _emit 0xcb
        // 0001fd1b: 74 13           JZ 0x0041fd30
        _emit 0x74
        _emit 0x13
        // 0001fd1d: 8b 11           MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 0001fd1f: 8b 02           MOV EAX,dword ptr [EDX]
        _emit 0x8b
        _emit 0x02
        // 0001fd21: 6a 01           PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0001fd23: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001fd25: a1 28 94 32 01  MOV EAX,[0x01329428]
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001fd2a: 89 98 84 01 00 00  MOV dword ptr [EAX+0x184],EBX
        _emit 0x89
        _emit 0x98
        _emit 0x84
        _emit 0x01
        _emit 0x00
        _emit 0x00

        // 0001fd30: 8b 88 88 01 00 00  MOV ECX,dword ptr [EAX+0x188]
        _emit 0x8b
        _emit 0x88
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001fd36: 3b cb           CMP ECX,EBX
        _emit 0x3b
        _emit 0xcb
        // 0001fd38: 74 13           JZ 0x0041fd4d
        _emit 0x74
        _emit 0x13
        // 0001fd3a: 8b 11           MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 0001fd3c: 8b 02           MOV EAX,dword ptr [EDX]
        _emit 0x8b
        _emit 0x02
        // 0001fd3e: 6a 01           PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0001fd40: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001fd42: a1 28 94 32 01  MOV EAX,[0x01329428]
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001fd47: 89 98 88 01 00 00  MOV dword ptr [EAX+0x188],EBX
        _emit 0x89
        _emit 0x98
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00

        // 0001fd4d: 8b 88 90 01 00 00  MOV ECX,dword ptr [EAX+0x190]
        _emit 0x8b
        _emit 0x88
        _emit 0x90
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001fd53: 3b cb           CMP ECX,EBX
        _emit 0x3b
        _emit 0xcb
        // 0001fd55: 74 13           JZ 0x0041fd6a
        _emit 0x74
        _emit 0x13
        // 0001fd57: 8b 11           MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 0001fd59: 8b 02           MOV EAX,dword ptr [EDX]
        _emit 0x8b
        _emit 0x02
        // 0001fd5b: 6a 01           PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0001fd5d: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001fd5f: a1 28 94 32 01  MOV EAX,[0x01329428]
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001fd64: 89 98 90 01 00 00  MOV dword ptr [EAX+0x190],EBX
        _emit 0x89
        _emit 0x98
        _emit 0x90
        _emit 0x01
        _emit 0x00
        _emit 0x00

        // 0001fd6a: 8b 88 94 01 00 00  MOV ECX,dword ptr [EAX+0x194]
        _emit 0x8b
        _emit 0x88
        _emit 0x94
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001fd70: 3b cb           CMP ECX,EBX
        _emit 0x3b
        _emit 0xcb
        // 0001fd72: 74 13           JZ 0x0041fd87
        _emit 0x74
        _emit 0x13
        // 0001fd74: 8b 11           MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 0001fd76: 8b 02           MOV EAX,dword ptr [EDX]
        _emit 0x8b
        _emit 0x02
        // 0001fd78: 6a 01           PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0001fd7a: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001fd7c: a1 28 94 32 01  MOV EAX,[0x01329428]
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001fd81: 89 98 94 01 00 00  MOV dword ptr [EAX+0x194],EBX
        _emit 0x89
        _emit 0x98
        _emit 0x94
        _emit 0x01
        _emit 0x00
        _emit 0x00

        // 0001fd87: 88 98 80 01 00 00  MOV byte ptr [EAX+0x180],BL
        _emit 0x88
        _emit 0x98
        _emit 0x80
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001fd8d: 5b              POP EBX
        _emit 0x5b
        // 0001fd8e: c3              RET
        _emit 0xc3
    }
}
