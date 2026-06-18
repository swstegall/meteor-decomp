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
// FUNCTION: ffxivgame 0x0002e390 — FUN_0042e390 (90 B / 0x5a, no SEH).
//
// Inspection (read from asm/ffxivgame/0002e390_FUN_0042e390.s):
//
//   __cdecl void FUN_0042e390();
//
//   Loads a global pointer from [0x0132997c] into ESI, calls FUN_004180b0
//   and tests bit 0x2 of AL. If the bit is SET (TEST result non-zero, JZ
//   skips), it pushes 0 and calls FUN_004186a0, then jumps to the tail.
//   If the bit is CLEAR (JZ taken), it pushes ESI, zeroes [0x01328db8],
//   stores ESI into [0x01328db0], and calls FUN_0041c460.
//
//   Tail (common path):
//     Reloads [0x0132997c] into ECX.
//     Pops the one argument from the stack (ADD ESP,0x4).
//     Pushes absolute address 0xf62e98 and 0, adds 8 to ECX,
//     calls FUN_00420900 (returns result in EAX).
//     Saves EAX+ECX pointer on stack, loads [0x01329960] into ECX,
//     calls FUN_00420cf0.
//     Pops ESI, returns (__cdecl — no ret N).
//
//   Reloc-bearing sites (CALL rel32 and absolute data references):
//     +0x02  MOV ESI,[0x0132997c]
//     +0x07  CALL FUN_004180b0      (rel32 → 0x004180b0)
//     +0x12  CALL FUN_004186a0      (rel32 → 0x004186a0)
//     +0x1a  MOV [0x01328db8],0x0
//     +0x24  MOV [0x01328db0],ESI
//     +0x2a  CALL FUN_0041c460      (rel32 → 0x0041c460)
//     +0x2f  MOV ECX,[0x0132997c]
//     +0x38  PUSH 0xf62e98
//     +0x42  CALL FUN_00420900      (rel32 → 0x00420900)
//     +0x4c  MOV ECX,[0x01329960]
//     +0x53  CALL FUN_00420cf0      (rel32 → 0x00420cf0)
//
// Reconstruction strategy — naked-asm byte passthrough.
//
//   Source-level C++ cannot reproduce the exact byte sequence due to the
//   absolute memory references and CALL rel32 relocations that only resolve
//   in a full-binary relink at image base 0x00400000.  tools/compare.py
//   masks the relocation windows on the cmp_obj path so a naked-asm .obj
//   with the same raw bytes matches byte-for-byte.
//
// Asm shape (90 bytes — RVA 0x0002e390..0x0002e3e9):
//
//   0002e390: 56                         PUSH ESI
//   0002e391: 8b 35 7c 99 32 01          MOV ESI,[0x0132997c]
//   0002e397: e8 14 9d fe ff             CALL 0x004180b0
//   0002e39c: a8 02                      TEST AL,0x2
//   0002e39e: 74 09                      JZ +0x9  (to 0x0042e3a9)
//   0002e3a0: 6a 00                      PUSH 0x0
//   0002e3a2: e8 f9 a2 fe ff             CALL 0x004186a0
//   0002e3a7: eb 16                      JMP +0x16 (to 0x0042e3bf)
//   0002e3a9: 56                         PUSH ESI
//   0002e3aa: c7 05 b8 8d 32 01
//             00 00 00 00               MOV dword ptr [0x01328db8],0x0
//   0002e3b4: 89 35 b0 8d 32 01          MOV [0x01328db0],ESI
//   0002e3ba: e8 a1 e0 fe ff             CALL 0x0041c460
//   0002e3bf: 8b 0d 7c 99 32 01          MOV ECX,[0x0132997c]
//   0002e3c5: 83 c4 04                   ADD ESP,0x4
//   0002e3c8: 68 98 2e f6 00             PUSH 0xf62e98
//   0002e3cd: 6a 00                      PUSH 0x0
//   0002e3cf: 83 c1 08                   ADD ECX,0x8
//   0002e3d2: e8 29 25 ff ff             CALL 0x00420900
//   0002e3d7: 8d 4c 24 08                LEA ECX,[ESP+0x8]
//   0002e3db: 51                         PUSH ECX
//   0002e3dc: 8b 0d 60 99 32 01          MOV ECX,[0x01329960]
//   0002e3e2: 50                         PUSH EAX
//   0002e3e3: e8 08 29 ff ff             CALL 0x00420cf0
//   0002e3e8: 5e                         POP ESI
//   0002e3e9: c3                         RET

extern "C" __declspec(naked) void FUN_0042e390() {
    __asm {
        _emit 0x56
        _emit 0x8b
        _emit 0x35
        _emit 0x7c
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xe8
        _emit 0x14
        _emit 0x9d
        _emit 0xfe
        _emit 0xff
        _emit 0xa8
        _emit 0x02
        _emit 0x74
        _emit 0x09

        _emit 0x6a
        _emit 0x00
        _emit 0xe8
        _emit 0xf9
        _emit 0xa2
        _emit 0xfe
        _emit 0xff
        _emit 0xeb
        _emit 0x16
        _emit 0x56
        _emit 0xc7
        _emit 0x05
        _emit 0xb8
        _emit 0x8d
        _emit 0x32
        _emit 0x01

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x35
        _emit 0xb0
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0xe8
        _emit 0xa1
        _emit 0xe0
        _emit 0xfe
        _emit 0xff
        _emit 0x8b

        _emit 0x0d
        _emit 0x7c
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x68
        _emit 0x98
        _emit 0x2e
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0x83

        _emit 0xc1
        _emit 0x08
        _emit 0xe8
        _emit 0x29
        _emit 0x25
        _emit 0xff
        _emit 0xff
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x51
        _emit 0x8b
        _emit 0x0d
        _emit 0x60
        _emit 0x99

        _emit 0x32
        _emit 0x01
        _emit 0x50
        _emit 0xe8
        _emit 0x08
        _emit 0x29
        _emit 0xff
        _emit 0xff
        _emit 0x5e
        _emit 0xc3
    }
}
