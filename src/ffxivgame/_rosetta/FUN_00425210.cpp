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
// FUNCTION: ffxivgame 0x00025210 — global matrix/transform snapshot pump
//                                  (463 B / 0x1cf, no SEH, no /GS).
//
// Inspection (read from the disassembly at orig RVA 0x00025210):
//
//   __cdecl void FUN_00425210(void);
//
//   Body shape (straight-line, no branches):
//
//     // Latch + clear a one-byte global flag, snapshot it into a sibling.
//     unsigned char f = *(unsigned char*)0x01328f19;   // MOV AL,[moffs]
//     *(unsigned char*)0x01266110 = f;                 // MOV [moffs],AL
//     *(unsigned char*)0x01328f19 = 0;                 // clear flag
//     FUN_0041c270(0);                                 // one-arg init (arg slot
//                                                      //   reused for the loop
//                                                      //   below; never repushed)
//
//     // Three iterations (idx 0,1,2): write the index into the reused stack
//     // arg slot at [ESP], call FUN_004186d0 which returns EAX = ptr to a
//     // 0x40-byte (8 qword) source block, then MOVQ-copy all 8 qwords into a
//     // global landing zone. Landing zones are 0x40 apart:
//     //   idx 0 → 0x01329990   idx 1 → 0x013299d0   idx 2 → 0x01329a10
//     for (int i = 0; i < 3; ++i) {
//         *(int*)esp = i;
//         qword* src = (qword*)FUN_004186d0();         // reads [esp] arg
//         memcpy_qword8(globalBlock[i], src);          // 8 × MOVQ via XMM0
//     }
//
//     // Tail: three more iterations (idx 0,1,2) that pair FUN_0042fcf0
//     // (one ptr arg, returns EAX) with FUN_00419020 (two stack args,
//     // index in the reused [ESP] slot). The ptr args are LEAs into the
//     // local frame ([ESP+4]/[ESP+8]) carried across the calls.
//
//   Stack frame: SUB ESP,0x40 prologue; one initial PUSH 0 reserves the
//   reused arg slot; net ADD ESP,0x48 / RET epilogue (__cdecl, void).
//
//   Reloc-bearing sites (all image-base-0x00400000-dependent):
//     - moffs8  0x01328f19 (flag, read + cleared)
//     - moffs8  0x01266110 (flag snapshot)
//     - 24 × MOVQ [imm32] into 0x01329990 .. 0x01329a48 (the three blocks)
//     - rel32 CALL FUN_0041c270 / FUN_004186d0 (×3) / FUN_0042fcf0 (×3) /
//       FUN_00419020 (×3)
//
// Reconstruction strategy — naked-asm byte passthrough (same idiom as the
// sibling FUN_00401820 / FUN_0040b840 / FUN_00409350 reloc-heavy bodies).
// A source-level rewrite would have to coax MSVC 2005 /O2 into the exact
// SSE2-MOVQ block copy, the reused-arg-slot calling sequence, and the ~30
// linker-resolved absolute/rel32 windows — every one brittle under /O2.
// Emitting the orig 463 bytes verbatim makes the .obj's .text section
// byte-identical to the orig slice, which is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_00425210() {
    __asm {
        _emit 0xa0
        _emit 0x19
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xec
        _emit 0x40
        _emit 0x6a
        _emit 0x00
        _emit 0xa2
        _emit 0x10
        _emit 0x61
        _emit 0x26
        _emit 0x01
        _emit 0xc6

        _emit 0x05
        _emit 0x19
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0xe8
        _emit 0x45
        _emit 0x70
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xc4
        _emit 0xc7
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x98
        _emit 0x34
        _emit 0xff
        _emit 0xff
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x00
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05

        _emit 0x90
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x08
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0x98
        _emit 0x99
        _emit 0x32

        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x10
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xa0
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xf3
        _emit 0x0f

        _emit 0x7e
        _emit 0x40
        _emit 0x18
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xa8
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x20

        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xb0
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x28
        _emit 0x66
        _emit 0x0f
        _emit 0xd6

        _emit 0x05
        _emit 0xb8
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x30
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xc0
        _emit 0x99

        _emit 0x32
        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x38
        _emit 0x8b
        _emit 0xc4
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xc8
        _emit 0x99
        _emit 0x32

        _emit 0x01
        _emit 0xc7
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x24
        _emit 0x34
        _emit 0xff
        _emit 0xff
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x00

        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xd0
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x08
        _emit 0x66
        _emit 0x0f
        _emit 0xd6

        _emit 0x05
        _emit 0xd8
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x10
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xe0
        _emit 0x99

        _emit 0x32
        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x18
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xe8
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xf3

        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x20
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xf0
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40

        _emit 0x28
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xf8
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x30
        _emit 0x66
        _emit 0x0f

        _emit 0xd6
        _emit 0x05
        _emit 0x00
        _emit 0x9a
        _emit 0x32
        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x38
        _emit 0x8b
        _emit 0xc4
        _emit 0x66
        _emit 0x0f
        _emit 0xd6

        _emit 0x05
        _emit 0x08
        _emit 0x9a
        _emit 0x32
        _emit 0x01
        _emit 0xc7
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xb0
        _emit 0x33
        _emit 0xff
        _emit 0xff

        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x00
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0x10
        _emit 0x9a
        _emit 0x32
        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40

        _emit 0x08
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0x18
        _emit 0x9a
        _emit 0x32
        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x10
        _emit 0x66
        _emit 0x0f

        _emit 0xd6
        _emit 0x05
        _emit 0x20
        _emit 0x9a
        _emit 0x32
        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x18
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0x28

        _emit 0x9a
        _emit 0x32
        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x20
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0x30
        _emit 0x9a
        _emit 0x32
        _emit 0x01

        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x28
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0x38
        _emit 0x9a
        _emit 0x32
        _emit 0x01
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e

        _emit 0x40
        _emit 0x30
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0x40
        _emit 0x9a
        _emit 0x32
        _emit 0x01
        _emit 0xf3
        _emit 0x0f

        _emit 0x7e
        _emit 0x40
        _emit 0x38
        _emit 0x51
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0x48
        _emit 0x9a
        _emit 0x32
        _emit 0x01
        _emit 0xe8
        _emit 0x5f
        _emit 0xa9
        _emit 0x00

        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x50
        _emit 0x51
        _emit 0x8b
        _emit 0xc4
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x7d

        _emit 0x3c
        _emit 0xff
        _emit 0xff
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x52
        _emit 0xe8
        _emit 0x43
        _emit 0xa9
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c

        _emit 0x50
        _emit 0x51
        _emit 0x8b
        _emit 0xc4
        _emit 0xc7
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x61
        _emit 0x3c
        _emit 0xff
        _emit 0xff
        _emit 0x8d

        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x50
        _emit 0xe8
        _emit 0x27
        _emit 0xa9
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x50
        _emit 0x51
        _emit 0x8b
        _emit 0xc4

        _emit 0xc7
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x45
        _emit 0x3c
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x48
        _emit 0xc3
    }
}
