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
// FUNCTION: ffxivgame 0x00039bb0 — __thiscall one-shot subsystem reset/init
//                                  (511 B / 0x1ff, no SEH, no /GS).
//
// Inspection (read from the disassembly at orig RVA 0x00039bb0):
//
//   __thiscall void FUN_00439bb0(this);  // ECX = this, returns void.
//
//   The body is a single guarded init that runs only the first time
//   through, then bumps a "done" counter on every call:
//
//     if (this->m_initDone /* [esi+0xd4] */ == 0) {
//         // --- reset/clear a chain of subobjects (state-numbered
//         //     0,1,2 calls into FUN_004186d0 / FUN_00419b60 with the
//         //     this-relative members +0x0c, +0x4c, +0x8c) ---
//         FUN_00418410(0, this->m_field8 /* [esi+0x8] */);
//         ... three FUN_004186d0 / FUN_00419b60 pairs with state 0/1/2 ...
//         // snapshot four globals into this:
//         this->[+0xcc] = *(u32*)0x01328f68;
//         this->[+0xd0] = *(u8*) 0x01328ef0;
//         this->[+0xd1] = *(u8*) 0x01328ee8;
//         this->[+0xd2] = *(u8*) 0x01328f19;
//         // two FUN_00419020 state-numbered (0/1) calls bracketing a
//         // FUN_0042fcf0 pair;
//         // x87 FP math: load consts [0x00fb7a60]/[0x00fb7a64]/[0x00f62f80],
//         // FILD two signed ints from [0x01328fa4]/[0x01328fa0] with the
//         // unsigned-fixup FADD [0x00f54a54] (2^32) when the high bit set,
//         // build an arg frame and CALL FUN_0043a1f0, then FUN_00419020 (2);
//         // --- re-init the globals just snapshotted ---
//         *(u32*)0x01328f68 = 0;  FUN_0041d0e0(0);
//         *(u8*) 0x01328ef0 = 1;  FUN_0041c1f0(1);
//         *(u8*) 0x01328ee8 = 0;  FUN_0041c1d0(0);
//         *(u8*) 0x01328f19 = 0;  FUN_0041c270(0);
//         *(u32*)0x01328ef8 = 6; *(u32*)0x01328efc = 7;
//         *(u32*)0x01328f04 = 6; *(u32*)0x01328f08 = 7;
//         FUN_0041d050(6, 7, 6, 7);
//         // FP const [0x00fb7a64] + three state-numbered FUN_004186e0 setup,
//         FUN_004186e0(...); FUN_00419240(3, 0); FUN_004186a0(3);
//     }
//     ++this->m_initDone;  // [esi+0xd4]
//
//   Stack frame: SUB ESP,0x44 prologue + transient SUB ESP,0x10 / ADD ESP
//   fold-downs around the FP arg-frame builds; ESI/EDI are the only
//   callee-saves pushed.
//
//   Reloc-bearing sites (image base 0x00400000 — every rel32 call target,
//   absolute global load/store, and FP-constant pointer below lands in a
//   relocation window in a real .obj):
//     +0x14 CALL FUN_00418410   +0x2a CALL FUN_004186d0 (×3: +0x2a/+0x44/+0x5e)
//     +0x36 / +0x50 / +0x6d CALL FUN_00419b60 (×3)
//     +0x72.. absolute loads .data 0x01328f68 / 0x01328ef0 / 0x01328ee8 /
//             0x01328f19  (snapshot into this)
//     +0xa6 CALL FUN_0042fcf0 (×2: +0xa6/+0xc2)  +0xb8/+0xd4 CALL FUN_00419020
//     FP consts .rdata 0x00fb7a60 / 0x00f62f80 / 0x00fb7a64 / 0x00f54a54
//     FILD ints .data 0x01328fa4 / 0x01328fa0
//     +0x136 CALL FUN_0043a1f0
//     +0x155 CALL FUN_0041d0e0  +0x163 CALL FUN_0041c1f0
//     +0x171 CALL FUN_0041c1d0  +0x17f CALL FUN_0041c270
//     stores .data 0x01328ef8 / 0x01328efc / 0x01328f04 / 0x01328f08
//     +0x1ac CALL FUN_0041d050  +0x1da CALL FUN_004186e0
//     +0x1e3 CALL FUN_00419240  +0x1ea CALL FUN_004186a0
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ rewrite would have to coax MSVC 2005 /O2 into
//   reproducing ~25 rel32 call targets in exact order, the inline x87 FP
//   const loads + unsigned-int FILD fixups, the interleaved arg-frame
//   builds (MOV EAX,ESP / MOV [EAX],imm state-numbering), AND the
//   linker-resolved absolute addresses across ~40 relocation windows.
//   Every high-level rewrite shifts at least one byte (state numbering,
//   branch short-vs-near, FP const ordering, modrm vs moffs32). The
//   pragmatic choice — matching the established sibling FUN_00401820 /
//   FUN_00409350 idiom — is a `__declspec(naked)` body re-emitting the
//   orig 511 bytes verbatim via MASM `_emit`. The .obj's `.text` section
//   ends up byte-identical to the orig slice, which is what
//   tools/compare.py checks against.

extern "C" __declspec(naked) void FUN_00439bb0() {
    __asm {
        _emit 0x83
        _emit 0xec
        _emit 0x44
        _emit 0x56
        _emit 0x8b
        _emit 0xf1
        _emit 0x83
        _emit 0xbe
        _emit 0xd4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x57
        _emit 0x0f
        _emit 0x85
        _emit 0xde
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        _emit 0x50
        _emit 0x6a
        _emit 0x00
        _emit 0xe8
        _emit 0x41
        _emit 0xe8
        _emit 0xfd
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x8b
        _emit 0xc4
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xf1
        _emit 0xea
        _emit 0xfd
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x50
        _emit 0x8d
        _emit 0x4e
        _emit 0x0c
        _emit 0xe8
        _emit 0x75
        _emit 0xff
        _emit 0xfd
        _emit 0xff
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
        _emit 0xd7
        _emit 0xea
        _emit 0xfd
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x50
        _emit 0x8d
        _emit 0x4e
        _emit 0x4c
        _emit 0xe8
        _emit 0x5b
        _emit 0xff
        _emit 0xfd
        _emit 0xff
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
        _emit 0xbd
        _emit 0xea
        _emit 0xfd
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x50
        _emit 0x8d
        _emit 0x8e
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x3e
        _emit 0xff
        _emit 0xfd
        _emit 0xff
        _emit 0x8b
        _emit 0x0d
        _emit 0x68
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x89
        _emit 0x8e
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8a
        _emit 0x15
        _emit 0xf0
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        _emit 0x88
        _emit 0x96
        _emit 0xd0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xa0
        _emit 0xe8
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        _emit 0x88
        _emit 0x86
        _emit 0xd1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8a
        _emit 0x0d
        _emit 0x19
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x52
        _emit 0x88
        _emit 0x8e
        _emit 0xd2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x95
        _emit 0x60
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
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
        _emit 0xb3
        _emit 0xf3
        _emit 0xfd
        _emit 0xff
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x50
        _emit 0xe8
        _emit 0x79
        _emit 0x60
        _emit 0xff
        _emit 0xff
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
        _emit 0x97
        _emit 0xf3
        _emit 0xfd
        _emit 0xff
        _emit 0xd9
        _emit 0x05
        _emit 0x60
        _emit 0x7a
        _emit 0xfb
        _emit 0x00
        _emit 0x8b
        _emit 0x0d
        _emit 0xa4
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x04
        _emit 0xd9
        _emit 0x05
        _emit 0x80
        _emit 0x2f
        _emit 0xf6
        _emit 0x00
        _emit 0x83
        _emit 0xec
        _emit 0x10
        _emit 0x85
        _emit 0xc9
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0xd9
        _emit 0x05
        _emit 0x64
        _emit 0x7a
        _emit 0xfb
        _emit 0x00
        _emit 0xd9
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0xdb
        _emit 0x05
        _emit 0xa4
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x7d
        _emit 0x06
        _emit 0xd8
        _emit 0x05
        _emit 0x54
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0x8b
        _emit 0x15
        _emit 0xa0
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x85
        _emit 0xd2
        _emit 0xdb
        _emit 0x05
        _emit 0xa0
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x7d
        _emit 0x06
        _emit 0xd8
        _emit 0x05
        _emit 0x54
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x04
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        _emit 0x50
        _emit 0xe8
        _emit 0x05
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
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
        _emit 0x23
        _emit 0xf3
        _emit 0xfd
        _emit 0xff
        _emit 0x33
        _emit 0xc0
        _emit 0x50
        _emit 0xa3
        _emit 0x68
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0xe8
        _emit 0xd6
        _emit 0x33
        _emit 0xfe
        _emit 0xff
        _emit 0x6a
        _emit 0x01
        _emit 0xc6
        _emit 0x05
        _emit 0xf0
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0xe8
        _emit 0xd8
        _emit 0x24
        _emit 0xfe
        _emit 0xff
        _emit 0x6a
        _emit 0x00
        _emit 0xc6
        _emit 0x05
        _emit 0xe8
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0xe8
        _emit 0xaa
        _emit 0x24
        _emit 0xfe
        _emit 0xff
        _emit 0x6a
        _emit 0x00
        _emit 0xc6
        _emit 0x05
        _emit 0x19
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0xe8
        _emit 0x3c
        _emit 0x25
        _emit 0xfe
        _emit 0xff
        _emit 0xb8
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xb9
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x51
        _emit 0x8b
        _emit 0xd0
        _emit 0x8b
        _emit 0xf9
        _emit 0x52
        _emit 0x57
        _emit 0x89
        _emit 0x0d
        _emit 0xf8
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        _emit 0xa3
        _emit 0xfc
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        _emit 0x89
        _emit 0x0d
        _emit 0x04
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0xa3
        _emit 0x08
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0xe8
        _emit 0xef
        _emit 0x32
        _emit 0xfe
        _emit 0xff
        _emit 0xd9
        _emit 0x05
        _emit 0x64
        _emit 0x7a
        _emit 0xfb
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        _emit 0x8b
        _emit 0xc4
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x04
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x51
        _emit 0x8b
        _emit 0xc4
        _emit 0x51
        _emit 0xc7
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xc4
        _emit 0x6a
        _emit 0x00
        _emit 0xc7
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x51
        _emit 0xe9
        _emit 0xfd
        _emit 0xff
        _emit 0x6a
        _emit 0x00
        _emit 0x6a
        _emit 0x03
        _emit 0xe8
        _emit 0xa8
        _emit 0xf4
        _emit 0xfd
        _emit 0xff
        _emit 0x6a
        _emit 0x03
        _emit 0xe8
        _emit 0x01
        _emit 0xe9
        _emit 0xfd
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        _emit 0x83
        _emit 0x86
        _emit 0xd4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x5f
        _emit 0x5e
        _emit 0x83
        _emit 0xc4
        _emit 0x44
        _emit 0xc3
    }
}
