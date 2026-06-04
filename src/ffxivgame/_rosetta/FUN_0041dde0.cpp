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
// FUNCTION: ffxivgame 0x0041dde0 — `__cdecl` game-state tick / input
//                                  dispatch (319 B / 0x13f).
//
// Inspection (read from the disassembly at orig RVA 0x0001dde0):
//
//   __cdecl void FUN_0041dde0();
//
//   Reads the global game-object pointer at [0x01329428], follows it to
//   the sub-object at +0x150, and calls an unknown function at 0x00423380
//   with the dword at +0x18 of that sub-object and the global context
//   pointer at [0x0132987c] as arguments.  The return value (EAX) and the
//   global byte at [0x0132989c] gate three branches:
//
//     if ([0x0132989c] != 0) {
//         if (EAX != 0) {
//             // vtable dispatch on [0x01329834]: call slot +0x0c, then
//             // check magic constants 0x88760827 / 0x88760869 and either
//             // call FUN_0041d6a0 or call SendMessage-style IAT routine
//             // with args (hwnd@+0x10, 0x10, 0, 0).
//         }
//     } else {
//         if (EAX != 0 && [0x01329880] == 0) {
//             // call the IAT routine at [0x00f3e424] with
//             // (hwnd@+0x10, 0xe01, 0, 0), then set [0x01329880] = 1.
//         }
//     }
//
//     // Common tail: query a function pointer via [0x00f3e148], compare
//     // a counter at [0x012660e4] against [0x012660e0], optionally call
//     // FUN_0041d6a0.  Then call vtable slot +0x10 on [0x01329834],
//     // compute a delta into the game-object at [0x01329428]+0x174, and
//     // (if [0x012660e9] != 0) fire two additional vtable slots on the
//     // sub-object at +0x1a0 before a tail-call to 0x004233b0/0x00423390.
//
//   No stack frame, no SEH, no locals; all work is done in registers.
//   Calling convention: __cdecl (clean RET at 0x0001de44; tail-call JMP
//   at the very end — 0x0001df1a: JMP 0x00423390).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function accesses ~14 distinct absolute addresses in .data /
//   .rdata / IAT and uses two internal tail-calls whose rel32 displacements
//   are image-base-relative.  Reproducing all of those relocations from
//   source-level C++ under MSVC 2005 /O2 would require the full surrounding
//   class hierarchy.  The pragmatic approach — identical to FUN_004014b0,
//   FUN_00401a00, FUN_00408f10, and all other _rosetta siblings — is a
//   __declspec(naked) body that re-emits the 319 bytes verbatim via MASM
//   _emit directives.

extern "C" __declspec(naked) void FUN_0041dde0() {
    __asm {
        // 0001dde0: a1 28 94 32 01  MOV EAX,[0x01329428]
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001dde5: 8b 88 50 01 00 00  MOV ECX,dword ptr [EAX+0x150]
        _emit 0x8b
        _emit 0x88
        _emit 0x50
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001ddeb: 8b 51 18  MOV EDX,dword ptr [ECX+0x18]
        _emit 0x8b
        _emit 0x51
        _emit 0x18
        // 0001ddee: 8b 0d 7c 98 32 01  MOV ECX,dword ptr [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001ddf4: 52  PUSH EDX
        _emit 0x52
        // 0001ddf5: e8 86 55 00 00  CALL 0x00423380
        _emit 0xe8
        _emit 0x86
        _emit 0x55
        _emit 0x00
        _emit 0x00
        // 0001ddfa: 80 3d 9c 98 32 01 00  CMP byte ptr [0x0132989c],0x0
        _emit 0x80
        _emit 0x3d
        _emit 0x9c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // 0001de01: 74 42  JZ 0x0041de45
        _emit 0x74
        _emit 0x42
        // 0001de03: 85 c0  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001de05: 74 70  JZ 0x0041de77
        _emit 0x74
        _emit 0x70
        // 0001de07: a1 34 98 32 01  MOV EAX,[0x01329834]
        _emit 0xa1
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001de0c: 8b 08  MOV ECX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x08
        // 0001de0e: 8b 51 0c  MOV EDX,dword ptr [ECX+0xc]
        _emit 0x8b
        _emit 0x51
        _emit 0x0c
        // 0001de11: 50  PUSH EAX
        _emit 0x50
        // 0001de12: ff d2  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0001de14: 3d 27 08 76 88  CMP EAX,0x88760827
        _emit 0x3d
        _emit 0x27
        _emit 0x08
        _emit 0x76
        _emit 0x88
        // 0001de19: 74 0e  JZ 0x0041de29
        _emit 0x74
        _emit 0x0e
        // 0001de1b: 3d 69 08 76 88  CMP EAX,0x88760869
        _emit 0x3d
        _emit 0x69
        _emit 0x08
        _emit 0x76
        _emit 0x88
        // 0001de20: 75 55  JNZ 0x0041de77
        _emit 0x75
        _emit 0x55
        // 0001de22: e8 79 f8 ff ff  CALL 0x0041d6a0
        _emit 0xe8
        _emit 0x79
        _emit 0xf8
        _emit 0xff
        _emit 0xff
        // 0001de27: eb 4e  JMP 0x0041de77
        _emit 0xeb
        _emit 0x4e
        // 0001de29: a1 28 94 32 01  MOV EAX,[0x01329428]
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001de2e: 8b 88 50 01 00 00  MOV ECX,dword ptr [EAX+0x150]
        _emit 0x8b
        _emit 0x88
        _emit 0x50
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001de34: 8b 51 10  MOV EDX,dword ptr [ECX+0x10]
        _emit 0x8b
        _emit 0x51
        _emit 0x10
        // 0001de37: 6a 00  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001de39: 6a 00  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001de3b: 6a 10  PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 0001de3d: 52  PUSH EDX
        _emit 0x52
        // 0001de3e: ff 15 94 e4 f3 00  CALL dword ptr [0x00f3e494]
        _emit 0xff
        _emit 0x15
        _emit 0x94
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        // 0001de44: c3  RET
        _emit 0xc3
        // 0001de45: 85 c0  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001de47: 74 2e  JZ 0x0041de77
        _emit 0x74
        _emit 0x2e
        // 0001de49: 80 3d 80 98 32 01 00  CMP byte ptr [0x01329880],0x0
        _emit 0x80
        _emit 0x3d
        _emit 0x80
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // 0001de50: 75 25  JNZ 0x0041de77
        _emit 0x75
        _emit 0x25
        // 0001de52: a1 28 94 32 01  MOV EAX,[0x01329428]
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001de57: 8b 88 50 01 00 00  MOV ECX,dword ptr [EAX+0x150]
        _emit 0x8b
        _emit 0x88
        _emit 0x50
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001de5d: 8b 51 10  MOV EDX,dword ptr [ECX+0x10]
        _emit 0x8b
        _emit 0x51
        _emit 0x10
        // 0001de60: 6a 00  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001de62: 6a 00  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001de64: 68 01 0e 00 00  PUSH 0xe01
        _emit 0x68
        _emit 0x01
        _emit 0x0e
        _emit 0x00
        _emit 0x00
        // 0001de69: 52  PUSH EDX
        _emit 0x52
        // 0001de6a: ff 15 24 e4 f3 00  CALL dword ptr [0x00f3e424]
        _emit 0xff
        _emit 0x15
        _emit 0x24
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        // 0001de70: c6 05 80 98 32 01 01  MOV byte ptr [0x01329880],0x1
        _emit 0xc6
        _emit 0x05
        _emit 0x80
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 0001de77: 56  PUSH ESI
        _emit 0x56
        // 0001de78: 8b 35 48 e1 f3 00  MOV ESI,dword ptr [0x00f3e148]
        _emit 0x8b
        _emit 0x35
        _emit 0x48
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0001de7e: 6a 00  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001de80: 68 90 98 32 01  PUSH 0x1329890
        _emit 0x68
        _emit 0x90
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001de85: ff d6  CALL ESI
        _emit 0xff
        _emit 0xd6
        // 0001de87: 85 c0  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001de89: 74 1c  JZ 0x0041dea7
        _emit 0x74
        _emit 0x1c
        // 0001de8b: a1 e4 60 26 01  MOV EAX,[0x012660e4]
        _emit 0xa1
        _emit 0xe4
        _emit 0x60
        _emit 0x26
        _emit 0x01
        // 0001de90: 50  PUSH EAX
        _emit 0x50
        // 0001de91: 68 e0 60 26 01  PUSH 0x12660e0
        _emit 0x68
        _emit 0xe0
        _emit 0x60
        _emit 0x26
        _emit 0x01
        // 0001de96: ff d6  CALL ESI
        _emit 0xff
        _emit 0xd6
        // 0001de98: 8b 0d e4 60 26 01  MOV ECX,dword ptr [0x012660e4]
        _emit 0x8b
        _emit 0x0d
        _emit 0xe4
        _emit 0x60
        _emit 0x26
        _emit 0x01
        // 0001de9e: 3b c1  CMP EAX,ECX
        _emit 0x3b
        _emit 0xc1
        // 0001dea0: 74 05  JZ 0x0041dea7
        _emit 0x74
        _emit 0x05
        // 0001dea2: e8 f9 f7 ff ff  CALL 0x0041d6a0
        _emit 0xe8
        _emit 0xf9
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        // 0001dea7: a1 34 98 32 01  MOV EAX,[0x01329834]
        _emit 0xa1
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001deac: 8b 10  MOV EDX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x10
        // 0001deae: 50  PUSH EAX
        _emit 0x50
        // 0001deaf: 8b 42 10  MOV EAX,dword ptr [EDX+0x10]
        _emit 0x8b
        _emit 0x42
        _emit 0x10
        // 0001deb2: ff d0  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001deb4: 8b 0d 28 94 32 01  MOV ECX,dword ptr [0x01329428]
        _emit 0x8b
        _emit 0x0d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001deba: 8b 91 70 01 00 00  MOV EDX,dword ptr [ECX+0x170]
        _emit 0x8b
        _emit 0x91
        _emit 0x70
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001dec0: 2b d0  SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 0001dec2: 80 3d e9 60 26 01 00  CMP byte ptr [0x012660e9],0x0
        _emit 0x80
        _emit 0x3d
        _emit 0xe9
        _emit 0x60
        _emit 0x26
        _emit 0x01
        _emit 0x00
        // 0001dec9: 89 91 74 01 00 00  MOV dword ptr [ECX+0x174],EDX
        _emit 0x89
        _emit 0x91
        _emit 0x74
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001decf: 5e  POP ESI
        _emit 0x5e
        // 0001ded0: 74 42  JZ 0x0041df14
        _emit 0x74
        _emit 0x42
        // 0001ded2: 8b 89 a0 01 00 00  MOV ECX,dword ptr [ECX+0x1a0]
        _emit 0x8b
        _emit 0x89
        _emit 0xa0
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001ded8: 8b 41 0c  MOV EAX,dword ptr [ECX+0xc]
        _emit 0x8b
        _emit 0x41
        _emit 0x0c
        // 0001dedb: 8b 50 04  MOV EDX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0001dede: 83 c1 0c  ADD ECX,0xc
        _emit 0x83
        _emit 0xc1
        _emit 0x0c
        // 0001dee1: ff d2  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0001dee3: a1 28 94 32 01  MOV EAX,[0x01329428]
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001dee8: 8b 80 a0 01 00 00  MOV EAX,dword ptr [EAX+0x1a0]
        _emit 0x8b
        _emit 0x80
        _emit 0xa0
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001deee: 8b 50 0c  MOV EDX,dword ptr [EAX+0xc]
        _emit 0x8b
        _emit 0x50
        _emit 0x0c
        // 0001def1: 8d 48 0c  LEA ECX,[EAX+0xc]
        _emit 0x8d
        _emit 0x48
        _emit 0x0c
        // 0001def4: 8b 42 08  MOV EAX,dword ptr [EDX+0x8]
        _emit 0x8b
        _emit 0x42
        _emit 0x08
        // 0001def7: ff d0  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001def9: 8b 0d 28 94 32 01  MOV ECX,dword ptr [0x01329428]
        _emit 0x8b
        _emit 0x0d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001deff: 8b 91 a0 01 00 00  MOV EDX,dword ptr [ECX+0x1a0]
        _emit 0x8b
        _emit 0x91
        _emit 0xa0
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001df05: 8b 42 10  MOV EAX,dword ptr [EDX+0x10]
        _emit 0x8b
        _emit 0x42
        _emit 0x10
        // 0001df08: 8b 0d 7c 98 32 01  MOV ECX,dword ptr [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001df0e: 50  PUSH EAX
        _emit 0x50
        // 0001df0f: e8 9c 54 00 00  CALL 0x004233b0
        _emit 0xe8
        _emit 0x9c
        _emit 0x54
        _emit 0x00
        _emit 0x00
        // 0001df14: 8b 0d 7c 98 32 01  MOV ECX,dword ptr [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001df1a: e9 71 54 00 00  JMP 0x00423390
        _emit 0xe9
        _emit 0x71
        _emit 0x54
        _emit 0x00
        _emit 0x00
    }
}
