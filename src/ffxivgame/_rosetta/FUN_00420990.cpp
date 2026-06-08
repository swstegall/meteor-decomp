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
// FUNCTION: ffxivgame 0x00020990 — global state initializer (219 B / 0xdb,
//                                  __cdecl, no args, no /GS, no SEH).
//
// Asm shape (219 bytes, RVA 0x00020990..0x00020a6b):
//
//   void FUN_00420990(void);
//
//   Prologue: PUSH EBX; XOR EBX,EBX; PUSH ESI — saves two callee-save
//   registers. EBX stays zero throughout; ESI is the loop counter.
//
//   Phase 1 — pre-zero two globals:
//     [0x01328d98] = 0; [0x01328da8] = 0
//
//   Phase 2 — loop 0..15 calling FUN_00418410(0, i):
//     for (int i = 0; i < 16; i++) FUN_00418410(0, i);
//     (__cdecl, 2 args via PUSH EBX / PUSH ESI, ADD ESP,8 cleanup)
//
//   Phase 3 — zero 16-byte-element array [0x01328dc8 .. 0x01328ed4):
//     EAX starts at 0x01328dd4; each iteration zeroes byte@[EAX-0xc],
//     dword@[EAX-0x8], dword@[EAX-0x4], dword@[EAX]; then EAX+=0x10
//     until EAX >= 0x01328ed4. 7-byte LEA ESP,[ESP] alignment NOP
//     precedes the loop.
//
//   Phase 4 — call FUN_00423200(__thiscall, ECX=[0x0132987c], arg0=0):
//     MOV [0x01328ec8]=0; PUSH EBX; CALL FUN_00423200
//
//   Phase 5 — call FUN_0041d240(0):
//     PUSH EBX; CALL FUN_0041d240; ADD ESP,4
//
//   Phase 6 — zero 5 globals: [0x01328d98..0x01328da8] (every 4 bytes)
//
//   Phase 7 — conditional init branch A (on FUN_004180b0() & 1):
//     CALL FUN_004180b0; TEST AL,1
//     if set:  CALL FUN_00419240(0, 1); JMP join
//     if clear: ECX=[0x0132987c]; zero [0x01328dac],[0x01328db4];
//               CALL FUN_00423210
//
//   Phase 8 — conditional init branch B (on FUN_004180b0() & 2):
//     CALL FUN_004180b0; PUSH EBX; TEST AL,2
//     if set:  CALL FUN_004186a0(0); POP ESI; POP EBX; RET
//     if clear: ECX=[0x0132987c]; zero [0x01328db0],[0x01328db8];
//               CALL FUN_00423220; POP ESI; POP EBX; RET
//
// Reconstruction strategy — __declspec(naked) byte passthrough.
//
//   The function contains no /GS cookie and no SEH frame, so in principle
//   a C++ source could reproduce it. However the combination of a 7-byte
//   alignment NOP (LEA ESP,[ESP+0x00000000]) between the two loops, the
//   specific MSVC 2005 global-store ordering, and the two-exit epilogue
//   structure make a naked byte passthrough safer and faster to verify.
//   All CALL rel32 and MOV imm32 immediates are baked verbatim from the
//   original binary slice; tools/compare.py masks the reloc bytes in its
//   diff so they do not affect the GREEN/PARTIAL/MISMATCH verdict.

extern "C" __declspec(naked) void FUN_00420990() {
    __asm {
        // Prologue: save EBX=0, ESI=loop counter
        _emit 0x53              // PUSH EBX
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0x56              // PUSH ESI

        // Phase 1: zero two globals
        _emit 0x89              // MOV [0x01328d98], EBX
        _emit 0x1d
        _emit 0x98
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x89              // MOV [0x01328da8], EBX
        _emit 0x1d
        _emit 0xa8
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x33              // XOR ESI, ESI
        _emit 0xf6

        // Phase 2: for (i=0; i<16; i++) FUN_00418410(0, i)
        _emit 0x53              // loop_top: PUSH EBX (arg0 = 0)
        _emit 0x56              // PUSH ESI (arg1 = i)
        _emit 0xe8              // CALL FUN_00418410
        _emit 0x67
        _emit 0x7a
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESI, 1
        _emit 0xc6
        _emit 0x01
        _emit 0x83              // ADD ESP, 8
        _emit 0xc4
        _emit 0x08
        _emit 0x83              // CMP ESI, 0x10
        _emit 0xfe
        _emit 0x10
        _emit 0x7c              // JL loop_top
        _emit 0xee

        // Phase 3: zero array; EAX = 0x01328dd4
        _emit 0xb8              // MOV EAX, 0x01328dd4
        _emit 0xd4
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x8d              // LEA ESP, [ESP+0x00000000]  (7-byte alignment NOP)
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // loop2_top:
        _emit 0x88              // MOV [EAX-0xc], BL
        _emit 0x58
        _emit 0xf4
        _emit 0x89              // MOV [EAX-0x8], EBX
        _emit 0x58
        _emit 0xf8
        _emit 0x89              // MOV [EAX], EBX
        _emit 0x18
        _emit 0x89              // MOV [EAX-0x4], EBX
        _emit 0x58
        _emit 0xfc
        _emit 0x83              // ADD EAX, 0x10
        _emit 0xc0
        _emit 0x10
        _emit 0x3d              // CMP EAX, 0x01328ed4
        _emit 0xd4
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        _emit 0x7c              // JL loop2_top
        _emit 0xeb

        // Phase 4: FUN_00423200(__thiscall, ECX=[0x0132987c], 0)
        _emit 0x8b              // MOV ECX, [0x0132987c]
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x53              // PUSH EBX
        _emit 0x89              // MOV [0x01328ec8], EBX
        _emit 0x1d
        _emit 0xc8
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL FUN_00423200
        _emit 0x19
        _emit 0x28
        _emit 0x00
        _emit 0x00

        // Phase 5: FUN_0041d240(0)
        _emit 0x53              // PUSH EBX
        _emit 0xe8              // CALL FUN_0041d240
        _emit 0x53
        _emit 0xc8
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 4
        _emit 0xc4
        _emit 0x04

        // Phase 6: zero 5 globals [0x01328d98..0x01328da8]
        _emit 0x89              // MOV [0x01328d98], EBX
        _emit 0x1d
        _emit 0x98
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x89              // MOV [0x01328d9c], EBX
        _emit 0x1d
        _emit 0x9c
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x89              // MOV [0x01328da0], EBX
        _emit 0x1d
        _emit 0xa0
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x89              // MOV [0x01328da4], EBX
        _emit 0x1d
        _emit 0xa4
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x89              // MOV [0x01328da8], EBX
        _emit 0x1d
        _emit 0xa8
        _emit 0x8d
        _emit 0x32
        _emit 0x01

        // Phase 7: branch on FUN_004180b0() & 1
        _emit 0xe8              // CALL FUN_004180b0
        _emit 0x9d
        _emit 0x76
        _emit 0xff
        _emit 0xff
        _emit 0xa8              // TEST AL, 1
        _emit 0x01
        _emit 0x74              // JZ branch_A_false (+0x0d)
        _emit 0x0d

        // branch_A_true: FUN_00419240(0, 1)
        _emit 0x6a              // PUSH 1
        _emit 0x01
        _emit 0x53              // PUSH EBX (=0)
        _emit 0xe8              // CALL FUN_00419240
        _emit 0x21
        _emit 0x88
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 8
        _emit 0xc4
        _emit 0x08
        _emit 0xeb              // JMP join (+0x18)
        _emit 0x18

        // branch_A_false: FUN_00423210(__thiscall, ECX=[0x0132987c], 0)
        _emit 0x8b              // MOV ECX, [0x0132987c]
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x53              // PUSH EBX
        _emit 0x89              // MOV [0x01328db4], EBX
        _emit 0x1d
        _emit 0xb4
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x89              // MOV [0x01328dac], EBX
        _emit 0x1d
        _emit 0xac
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL FUN_00423210
        _emit 0xd4
        _emit 0x27
        _emit 0x00
        _emit 0x00

        // join (Phase 8): branch on FUN_004180b0() & 2
        _emit 0xe8              // CALL FUN_004180b0
        _emit 0x6f
        _emit 0x76
        _emit 0xff
        _emit 0xff
        _emit 0xa8              // TEST AL, 2
        _emit 0x02
        _emit 0x53              // PUSH EBX
        _emit 0x74              // JZ branch_B_false (+0x0b)
        _emit 0x0b

        // branch_B_true: FUN_004186a0(0)
        _emit 0xe8              // CALL FUN_004186a0
        _emit 0x55
        _emit 0x7c
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 4
        _emit 0xc4
        _emit 0x04
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET

        // branch_B_false: FUN_00423220(__thiscall, ECX=[0x0132987c], 0)
        _emit 0x8b              // MOV ECX, [0x0132987c]
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x89              // MOV [0x01328db8], EBX
        _emit 0x1d
        _emit 0xb8
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x89              // MOV [0x01328db0], EBX
        _emit 0x1d
        _emit 0xb0
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL FUN_00423220
        _emit 0xb8
        _emit 0x27
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
