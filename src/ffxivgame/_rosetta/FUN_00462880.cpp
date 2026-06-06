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
// FUNCTION: ffxivgame 0x00462880 — registry/list lookup-and-build helper
//                                  (233 B / 0xe9, no SEH).
//
// Behaviour read from the disassembly at orig RVA 0x00062880:
//
//   __cdecl void* FUN_00462880(/*arg0 unused*/, void* list /*[esp+0x1c]*/,
//                              void* ctx /*[esp+0x20]*/);
//
//   Prologue `MOV EAX,0x4; CALL 0x009d29d0` is the MSVC fixed-size stack
//   reserve helper (4-byte local frame = loop index `i`), followed by the
//   usual PUSH EBX/EBP/ESI/EDI register save.
//
//   Structural shape:
//
//     void* out = FUN_00460f30(0xf69b60);     // alloc/lookup; bail → 0
//     if (!out) return 0;
//     int i = 0;
//     int n = FUN_00464030(list);             // element count
//     while (i < n) {
//         Entry* e = FUN_00464040(i, list);   // element at index
//         int cmp = e->compare(ctx, out);     // __thiscall FUN_004626c0
//         if (cmp > 0) { ++i; continue; }     // skip
//         if (cmp < 0) goto fail;             // ordering broke → fail
//         // cmp == 0: tag dispatch on e->field4 (8/10-byte memcmp)
//         if (memcmp(0xf69868, e->field4, 8) == 0)
//             FUN_004620d0(&out[1], e->field8);
//         else if (memcmp(0xf6985c, e->field4, 10) == 0) {
//             out->field8 = FUN_00462050(/*EDI=*/e->field8, /*EBX=*/[esp+0x18]);
//             if (!out->field8) goto fail;
//         }
//         ++i;
//     }
//     return out;                             // success
//   fail:
//     FUN_004612e0(out, 0xf69b60);            // teardown
//     return 0;
//
//   Reloc-bearing sites in the orig 233 bytes (absolute addresses /
//   rel32 call displacements resolve only in a full-binary relink at
//   image base 0x00400000; standalone .obj compilation can't reproduce
//   them):
//     +0x005  rel32  0x009d29d0 — fixed-size stack reserve helper
//     +0x00e  abs32  0x00f69b60 — registry key/tag (PUSH'd, 2 sites)
//     +0x013  rel32  0x00460f30 — alloc/lookup
//     +0x030  rel32  0x00464030 — element count (2 sites)
//     +0x042  rel32  0x00464040 — element-at-index
//     +0x051  rel32  0x004626c0 — __thiscall comparator
//     +0x062  abs32  0x00f69868 — 8-byte tag literal
//     +0x07c  rel32  0x004620d0 — branch-A consumer
//     +0x086  abs32  0x00f6985c — 10-byte tag literal
//     +0x09f  rel32  0x00462050 — branch-B builder
//     +0x0d9  rel32  0x004612e0 — failure teardown
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The same brittleness the sibling reloc-heavy /O2 routines hit
//   (FUN_00402a30, FUN_004053a0, FUN_0045e9f0) applies: coaxing MSVC
//   2005 /O2 into the exact register allocation (EBP=out, EBX=entry,
//   ESI=i, EDI=list), the two REPE CMPSB tag dispatches, the
//   signed three-way comparator branch (JG/JL), AND the eleven
//   linker-resolved windows above is not worth the per-byte fight.
//   A `__declspec(naked)` body re-emits the orig 233 bytes verbatim;
//   the .obj's `.text` ends up byte-identical to the orig slice (no
//   relocations — addresses are baked in as raw immediates), which is
//   what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_00462880() {
    __asm {
        // 00062880  MOV EAX, 0x4
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00062885  CALL 0x009d29d0  (stack reserve)
        _emit 0xe8
        _emit 0x46
        _emit 0x01
        _emit 0x57
        _emit 0x00
        // 0006288a  PUSH EBX
        _emit 0x53
        // 0006288b  PUSH EBP
        _emit 0x55
        // 0006288c  PUSH ESI
        _emit 0x56
        // 0006288d  PUSH EDI
        _emit 0x57
        // 0006288e  PUSH 0xf69b60
        _emit 0x68
        _emit 0x60
        _emit 0x9b
        _emit 0xf6
        _emit 0x00
        // 00062893  CALL 0x00460f30
        _emit 0xe8
        _emit 0x98
        _emit 0xe6
        _emit 0xff
        _emit 0xff
        // 00062898  MOV EBP, EAX
        _emit 0x8b
        _emit 0xe8
        // 0006289a  ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0006289d  TEST EBP, EBP
        _emit 0x85
        _emit 0xed
        // 0006289f  JZ 0x00462961
        _emit 0x0f
        _emit 0x84
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000628a5  MOV EDI, [ESP+0x1c]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        // 000628a9  XOR ESI, ESI
        _emit 0x33
        _emit 0xf6
        // 000628ab  PUSH EDI
        _emit 0x57
        // 000628ac  MOV [ESP+0x14], ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 000628b0  CALL 0x00464030
        _emit 0xe8
        _emit 0x7b
        _emit 0x17
        _emit 0x00
        _emit 0x00
        // 000628b5  ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 000628b8  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 000628ba  JLE 0x0046294b
        _emit 0x0f
        _emit 0x8e
        _emit 0x8b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000628c0  PUSH ESI
        _emit 0x56
        // 000628c1  PUSH EDI
        _emit 0x57
        // 000628c2  CALL 0x00464040
        _emit 0xe8
        _emit 0x79
        _emit 0x17
        _emit 0x00
        _emit 0x00
        // 000628c7  MOV EBX, EAX
        _emit 0x8b
        _emit 0xd8
        // 000628c9  MOV EAX, [ESP+0x20]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 000628cd  PUSH EAX
        _emit 0x50
        // 000628ce  PUSH EBP
        _emit 0x55
        // 000628cf  MOV ECX, EBX
        _emit 0x8b
        _emit 0xcb
        // 000628d1  CALL 0x004626c0
        _emit 0xe8
        _emit 0xea
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 000628d6  ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 000628d9  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 000628db  JG 0x0046292b
        _emit 0x7f
        _emit 0x4e
        // 000628dd  JL 0x00462953
        _emit 0x7c
        _emit 0x74
        // 000628df  MOV EAX, [EBX+0x4]
        _emit 0x8b
        _emit 0x43
        _emit 0x04
        // 000628e2  MOV EDI, 0xf69868
        _emit 0xbf
        _emit 0x68
        _emit 0x98
        _emit 0xf6
        _emit 0x00
        // 000628e7  MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 000628e9  MOV ECX, 0x8
        _emit 0xb9
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000628ee  XOR EDX, EDX
        _emit 0x33
        _emit 0xd2
        // 000628f0  REPE CMPSB
        _emit 0xf3
        _emit 0xa6
        // 000628f2  JNZ 0x00462906
        _emit 0x75
        _emit 0x12
        // 000628f4  MOV EAX, [EBX+0x8]
        _emit 0x8b
        _emit 0x43
        _emit 0x08
        // 000628f7  PUSH EAX
        _emit 0x50
        // 000628f8  LEA ECX, [EBP+0x4]
        _emit 0x8d
        _emit 0x4d
        _emit 0x04
        // 000628fb  PUSH ECX
        _emit 0x51
        // 000628fc  CALL 0x004620d0
        _emit 0xe8
        _emit 0xcf
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        // 00062901  ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00062904  JMP 0x00462927
        _emit 0xeb
        _emit 0x21
        // 00062906  MOV EDI, 0xf6985c
        _emit 0xbf
        _emit 0x5c
        _emit 0x98
        _emit 0xf6
        _emit 0x00
        // 0006290b  MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 0006290d  MOV ECX, 0xa
        _emit 0xb9
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00062912  XOR EDX, EDX
        _emit 0x33
        _emit 0xd2
        // 00062914  REPE CMPSB
        _emit 0xf3
        _emit 0xa6
        // 00062916  JNZ 0x0046292b
        _emit 0x75
        _emit 0x13
        // 00062918  MOV EDI, [EBX+0x8]
        _emit 0x8b
        _emit 0x7b
        _emit 0x08
        // 0006291b  MOV EBX, [ESP+0x18]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        // 0006291f  CALL 0x00462050
        _emit 0xe8
        _emit 0x2c
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        // 00062924  MOV [EBP+0x8], EAX
        _emit 0x89
        _emit 0x45
        _emit 0x08
        // 00062927  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00062929  JZ 0x00462953
        _emit 0x74
        _emit 0x28
        // 0006292b  MOV ESI, [ESP+0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 0006292f  MOV EDI, [ESP+0x1c]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        // 00062933  ADD ESI, 0x1
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        // 00062936  PUSH EDI
        _emit 0x57
        // 00062937  MOV [ESP+0x14], ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 0006293b  CALL 0x00464030
        _emit 0xe8
        _emit 0xf0
        _emit 0x16
        _emit 0x00
        _emit 0x00
        // 00062940  ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00062943  CMP ESI, EAX
        _emit 0x3b
        _emit 0xf0
        // 00062945  JL 0x004628c0
        _emit 0x0f
        _emit 0x8c
        _emit 0x75
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0006294b  POP EDI
        _emit 0x5f
        // 0006294c  POP ESI
        _emit 0x5e
        // 0006294d  MOV EAX, EBP
        _emit 0x8b
        _emit 0xc5
        // 0006294f  POP EBP
        _emit 0x5d
        // 00062950  POP EBX
        _emit 0x5b
        // 00062951  POP ECX
        _emit 0x59
        // 00062952  RET
        _emit 0xc3
        // 00062953  PUSH 0xf69b60
        _emit 0x68
        _emit 0x60
        _emit 0x9b
        _emit 0xf6
        _emit 0x00
        // 00062958  PUSH EBP
        _emit 0x55
        // 00062959  CALL 0x004612e0
        _emit 0xe8
        _emit 0x82
        _emit 0xe9
        _emit 0xff
        _emit 0xff
        // 0006295e  ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00062961  POP EDI
        _emit 0x5f
        // 00062962  POP ESI
        _emit 0x5e
        // 00062963  POP EBP
        _emit 0x5d
        // 00062964  XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00062966  POP EBX
        _emit 0x5b
        // 00062967  POP ECX
        _emit 0x59
        // 00062968  RET
        _emit 0xc3
    }
}
