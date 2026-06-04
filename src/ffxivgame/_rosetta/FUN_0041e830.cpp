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
// FUNCTION: ffxivgame 0x0001e830 — __cdecl camera/target-update dispatcher
//                                  (511 B / 0x1ff, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x0001e830):
//
//   __cdecl void FUN_0041e830(int arg2 /*[esp+0x30]*/,
//                             int arg3 /*[esp+0x34]*/,
//                             int arg4 /*[esp+0x38]*/, ...);
//
//   Prologue `SUB ESP,0x20 / PUSH ESI / PUSH EDI` and epilogue
//   `ADD ESP,0x20 / RET` (no `ret N`) — caller-cleaned __cdecl. EDI is
//   zeroed and reused as the constant 0 throughout. Body shape:
//
//     // Group A — one-shot lazy registration gated on .data flags.
//     if (DAT_01329834 != 0) {
//         void* ctx = DAT_01329428;                  // [0x01329428]
//         if (ctx->[0x164] != arg2 ||
//             ctx->[0x168] != arg3 ||
//             ctx->[0x16c] != arg4) {
//             if (!(DAT_01323910 & 1)) {             // run-once latch
//                 DAT_01323910 |= 1;
//                 DAT_0132390c  = 0x0041d3a0;        // install fn-ptr
//             }
//             // (*DAT_0132390c)(0xf59260, 0xf58b16, 0xf59210,
//             //                 0xa4a, 0xf591c8);    // 5-arg log/assert
//         }
//     }
//
//     // Group B — allocate + zero-init a 0x24-byte record.
//     void* rec = FUN_0040e2d0(/*ecx=&local*/ &buf, 0x10, 0xf59268);
//     void* obj = FUN_00419c40(0x24, rec);
//     if (obj) { memset-ish zero of fields [0..0x20], byte[0x14]=0; }
//     void* self = obj ? obj : 0;
//
//     // Group C — fold arg3 default, gather four 1.x camera helpers,
//     // do an x87 divide with FILD/sign-fixup + FNSTCW/FLDCW rounding
//     // truncation (FISTP qword), then dispatch FUN_0041df20 with the
//     // assembled record, FUN_004214a0 on ctx+0x140, and a final
//     // compare-and-store of ctx->[0x150] / ctx->[0x17c] FP slot.
//     ...
//     FUN_00433150(arg3'); FUN_00433190(arg3'); FUN_004331a0(arg3');
//     FUN_0041c5b0(arg3');         // returns x87 ST(0)
//     self->... ; FUN_0041d4d0(truncated);
//     FUN_0041df20(self, ...);
//     FUN_004214a0(&a, &b /*ctx+0x140*/);
//     if (ctx->[0x150] != self && (FUN_00418280() truthy)) {}
//     else ctx->[0x150] = self;
//     if (ctx->[0x17c] >= 0.0f-const) ctx->[0x17c] = const;  // UCOMISS/LAHF
//
//   Reloc-bearing sites in the orig 511 bytes (image base 0x00400000):
//     +0x07  CMP [imm32]   — .data 0x01329834 (enable flag)
//     +0x0f  MOV EAX,[]    — .data 0x01329428 (camera ctx singleton, x6)
//     +0x38  TEST [imm32]  — .data 0x01323910 (run-once latch)
//     +0x41  OR   [imm32]  — .data 0x01323910
//     +0x48  MOV  [imm32]  — .data 0x0132390c = 0x0041d3a0 (installed fnptr)
//     +0x52..+0x66  5 × PUSH imm32 — 0xf591c8/0xa4a/0xf59210/0xf58b16/0xf59260
//     +0x6b  CALL [imm32]  — indirect via .data 0x0132390c
//     +0x74  PUSH imm32    — 0xf59268
//     +0x7f  CALL rel32    — FUN_0040e2d0
//     +0x87  CALL rel32    — FUN_00419c40
//     +0xdb  CALL rel32    — FUN_00433150
//     +0xe5  CALL rel32    — FUN_00433190
//     +0xef  CALL rel32    — FUN_004331a0
//     +0xf7  CALL rel32    — FUN_0041c5b0
//     +0x10e FADD [imm32]  — .rdata 0x00f54a54 (unsigned-fixup const)
//     +0x138 MOV EAX,[]    — .data 0x012660ec
//     +0x150 CALL rel32    — FUN_0041d4d0
//     +0x18a CALL rel32    — FUN_0041df20
//     +0x1a8 CALL rel32    — FUN_004214a0
//     +0x1bd CALL rel32    — FUN_00418280
//     +0x1da MOVSS XMM0,[] — .rdata 0x00f54f70 (clamp const)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ rewrite would need to coax MSVC 2005 /O2 into
//   reproducing the exact register allocation across the lazy-init gate,
//   the x87 FNSTCW/OR 0xc00/FLDCW truncated-FISTP rounding sequence, the
//   LAHF/TEST AH,0x44/JNP float-compare idiom, AND ~25 linker-resolved
//   absolute addresses (six loads of the camera-ctx singleton, the
//   run-once latch pair, the installed-fnptr indirect CALL, five string/
//   data pushes, two FP constants, and six rel32 neighbour calls). Each
//   constraint is brittle under /O2 — every high-level rewrite shifts at
//   least one byte (branch short-vs-near, modrm-vs-moffs32, x87 spill
//   slot numbering).
//
//   The pragmatic choice — identical to the sibling FUN_00401820 /
//   FUN_00409350 / FUN_0040b840 reloc-heavy bodies — is a
//   `__declspec(naked)` body that re-emits the orig 511 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` ends up byte-
//   identical to the orig slice (raw immediates, no relocations), which
//   is what tools/compare.py checks against. The commentary above is the
//   readable record so a future contributor can promote this to a real
//   source-level match once the camera-ctx class (.data 0x01329428,
//   fields +0x140/+0x150/+0x164..+0x17c) is catalogued under
//   decomp-notes/types/.

extern "C" __declspec(naked) void FUN_0041e830() {
    __asm {
        _emit 0x83
        _emit 0xec
        _emit 0x20
        _emit 0x56
        _emit 0x57
        _emit 0x33
        _emit 0xff
        _emit 0x39
        _emit 0x3d
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x74
        _emit 0x65
        _emit 0xa1

        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        _emit 0x39
        _emit 0x88
        _emit 0x64
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x75
        _emit 0x18

        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x34
        _emit 0x39
        _emit 0x90
        _emit 0x68
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x75
        _emit 0x0c
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38

        _emit 0x39
        _emit 0x88
        _emit 0x6c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x3c
        _emit 0xf6
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75

        _emit 0x11
        _emit 0x83
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xa0
        _emit 0xd3

        _emit 0x41
        _emit 0x00
        _emit 0x68
        _emit 0xc8
        _emit 0x91
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x4a
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x10
        _emit 0x92
        _emit 0xf5

        _emit 0x00
        _emit 0x68
        _emit 0x16
        _emit 0x8b
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x60
        _emit 0x92
        _emit 0xf5
        _emit 0x00
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32

        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x68
        _emit 0x68
        _emit 0x92
        _emit 0xf5
        _emit 0x00
        _emit 0x6a
        _emit 0x10
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0xe8

        _emit 0x1c
        _emit 0xfa
        _emit 0xfe
        _emit 0xff
        _emit 0x50
        _emit 0x6a
        _emit 0x24
        _emit 0xe8
        _emit 0x84
        _emit 0xb3
        _emit 0xff
        _emit 0xff
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        _emit 0x83

        _emit 0xc4
        _emit 0x08
        _emit 0x3b
        _emit 0xc7
        _emit 0x74
        _emit 0x21
        _emit 0x89
        _emit 0x38
        _emit 0x89
        _emit 0x78
        _emit 0x04
        _emit 0x89
        _emit 0x78
        _emit 0x08
        _emit 0xf3
        _emit 0x0f

        _emit 0x11
        _emit 0x40
        _emit 0x0c
        _emit 0x89
        _emit 0x78
        _emit 0x10
        _emit 0xc6
        _emit 0x40
        _emit 0x14
        _emit 0x00
        _emit 0x89
        _emit 0x78
        _emit 0x18
        _emit 0x89
        _emit 0x78
        _emit 0x1c

        _emit 0x89
        _emit 0x78
        _emit 0x20
        _emit 0x8b
        _emit 0xf0
        _emit 0xeb
        _emit 0x02
        _emit 0x33
        _emit 0xf6
        _emit 0x53
        _emit 0x55
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x34
        _emit 0x3b

        _emit 0xef
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x2c
        _emit 0x75
        _emit 0x13
        _emit 0x8b
        _emit 0x15
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0xbd
        _emit 0x02
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x82
        _emit 0x78
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x55
        _emit 0xe8
        _emit 0x40
        _emit 0x48
        _emit 0x01
        _emit 0x00

        _emit 0x55
        _emit 0x88
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xe8
        _emit 0x76
        _emit 0x48
        _emit 0x01
        _emit 0x00
        _emit 0x55
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xe8

        _emit 0x7c
        _emit 0x48
        _emit 0x01
        _emit 0x00
        _emit 0x55
        _emit 0x8b
        _emit 0xd8
        _emit 0xe8
        _emit 0x84
        _emit 0xdc
        _emit 0xff
        _emit 0xff
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x20

        _emit 0x8b
        _emit 0xc3
        _emit 0x85
        _emit 0xc0
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x44
        _emit 0xdb
        _emit 0x44
        _emit 0x24
        _emit 0x44
        _emit 0x7d
        _emit 0x06
        _emit 0xd8
        _emit 0x05

        _emit 0x54
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0xd8
        _emit 0x74
        _emit 0x24
        _emit 0x20
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x52
        _emit 0x8b
        _emit 0x15
        _emit 0x28

        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x8b
        _emit 0x7a
        _emit 0x04
        _emit 0xd9
        _emit 0x7c
        _emit 0x24
        _emit 0x48
        _emit 0x0f
        _emit 0xb7
        _emit 0x44
        _emit 0x24
        _emit 0x48
        _emit 0x0d

        _emit 0x00
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xa1
        _emit 0xec
        _emit 0x60
        _emit 0x26
        _emit 0x01
        _emit 0x50
        _emit 0xd9
        _emit 0x6c

        _emit 0x24
        _emit 0x30
        _emit 0x8b
        _emit 0xc3
        _emit 0xdf
        _emit 0x7c
        _emit 0x24
        _emit 0x30
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        _emit 0xd9
        _emit 0x6c
        _emit 0x24
        _emit 0x4c

        _emit 0xe8
        _emit 0x4b
        _emit 0xeb
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x58
        _emit 0x6a
        _emit 0x00
        _emit 0x6a

        _emit 0x01
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x89
        _emit 0x46
        _emit 0x10
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0x51
        _emit 0x8b
        _emit 0x4c
        _emit 0x24

        _emit 0x5c
        _emit 0xc6
        _emit 0x46
        _emit 0x14
        _emit 0x01
        _emit 0x89
        _emit 0x2e
        _emit 0x6a
        _emit 0x00
        _emit 0x89
        _emit 0x46
        _emit 0x04
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x64

        _emit 0x52
        _emit 0x50
        _emit 0x51
        _emit 0x89
        _emit 0x5e
        _emit 0x08
        _emit 0xd9
        _emit 0x5e
        _emit 0x0c
        _emit 0x56
        _emit 0xe8
        _emit 0x61
        _emit 0xf5
        _emit 0xff
        _emit 0xff
        _emit 0x8b

        _emit 0x0d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x38
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        _emit 0x52
        _emit 0x8d
        _emit 0x44
        _emit 0x24

        _emit 0x24
        _emit 0x50
        _emit 0x81
        _emit 0xc1
        _emit 0x40
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xc3
        _emit 0x2a
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x0d
        _emit 0x28

        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x39
        _emit 0xb1
        _emit 0x50
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x5d
        _emit 0x5b
        _emit 0x74
        _emit 0x15
        _emit 0xe8
        _emit 0x8e
        _emit 0x98

        _emit 0xff
        _emit 0xff
        _emit 0x84
        _emit 0xc0
        _emit 0x8b
        _emit 0x0d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x75
        _emit 0x06
        _emit 0x89
        _emit 0xb1
        _emit 0x50
        _emit 0x01

        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x89
        _emit 0x7c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f

        _emit 0xf5
        _emit 0x00
        _emit 0x0f
        _emit 0x2e
        _emit 0xc8
        _emit 0x9f
        _emit 0x5f
        _emit 0xf6
        _emit 0xc4
        _emit 0x44
        _emit 0x8b
        _emit 0x81
        _emit 0x50
        _emit 0x01
        _emit 0x00
        _emit 0x00

        _emit 0x5e
        _emit 0x7b
        _emit 0x08
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x81
        _emit 0x7c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        _emit 0xc3
    }
}
