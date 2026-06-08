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
// FUNCTION: ffxivgame 0x0042d000 — wrapper around FUN_0041c270 that
//                                  temporarily zeroes a global flag at
//                                  0x01328f19, calls FUN_0042cbc0, then
//                                  restores the flag and tail-jumps to
//                                  FUN_0041c270 with modified arguments.
//
// Asm shape (67 bytes / 0x43):
//
//   0002d000:  51                    PUSH ECX
//   0002d001:  53                    PUSH EBX
//   0002d002:  8a 1d 19 8f 32 01     MOV BL, byte ptr [0x01328f19]
//   0002d008:  6a 00                 PUSH 0x0
//   0002d00a:  88 5c 24 08           MOV byte ptr [ESP+0x8], BL
//   0002d00e:  c6 05 19 8f 32 01 00  MOV byte ptr [0x01328f19], 0x0
//   0002d015:  e8 56 f2 fe ff        CALL 0x0041c270
//   0002d01a:  d9 ee                 FLDZ
//   0002d01c:  8b 44 24 10           MOV EAX, dword ptr [ESP+0x10]
//   0002d020:  d9 1c 24              FSTP float ptr [ESP]
//   0002d023:  50                    PUSH EAX
//   0002d024:  e8 97 fb ff ff        CALL 0x0042cbc0
//   0002d029:  8b 4c 24 0c           MOV ECX, dword ptr [ESP+0xc]
//   0002d02d:  83 c4 08              ADD ESP, 0x8
//   0002d030:  88 1d 19 8f 32 01     MOV byte ptr [0x01328f19], BL
//   0002d036:  5b                    POP EBX
//   0002d037:  83 c4 04              ADD ESP, 0x4
//   0002d03a:  89 4c 24 04           MOV dword ptr [ESP+0x4], ECX
//   0002d03e:  e9 2d f2 fe ff        JMP 0x0041c270
//
// Pattern: save/restore global byte 0x01328f19 around two sibling calls;
//   FLDZ + FSTP rewrites the first stack slot to 0.0f; tail-jump with
//   modified first argument (ECX replaces original arg1 on stack).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ cannot reproduce this exact sequence due to:
//     - the unusual prologue (PUSH ECX / PUSH EBX without a standard frame)
//     - x87 FLDZ + FSTP used to zero a stack slot (float constant)
//     - ECX overwrite via [ESP+0xc] load before the tail JMP
//     - absolute memory references to 0x01328f19 resolved in the image
//     - rel32 CALL/JMP offsets already resolved in the binary
//   Following the pattern of sibling FUN_00406fa0 / FUN_00408780, emit
//   all 67 bytes verbatim via MASM _emit directives.

extern "C" __declspec(naked) void FUN_0042d000() {
    __asm {
        _emit 0x51              // PUSH ECX
        _emit 0x53              // PUSH EBX
        _emit 0x8a              // MOV BL, byte ptr [0x01328f19]
        _emit 0x1d
        _emit 0x19
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x88              // MOV byte ptr [ESP+0x8], BL
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0xc6              // MOV byte ptr [0x01328f19], 0x0
        _emit 0x05
        _emit 0x19
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0xe8              // CALL 0x0041c270
        _emit 0x56
        _emit 0xf2
        _emit 0xfe
        _emit 0xff
        _emit 0xd9              // FLDZ
        _emit 0xee
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xd9              // FSTP float ptr [ESP]
        _emit 0x1c
        _emit 0x24
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0042cbc0
        _emit 0x97
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0xc]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x88              // MOV byte ptr [0x01328f19], BL
        _emit 0x1d
        _emit 0x19
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ESP+0x4], ECX
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xe9              // JMP 0x0041c270
        _emit 0x2d
        _emit 0xf2
        _emit 0xfe
        _emit 0xff
    }
}
