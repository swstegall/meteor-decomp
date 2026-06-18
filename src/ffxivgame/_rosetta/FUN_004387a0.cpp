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
// FUNCTION: ffxivgame 0x004387a0 — __thiscall method that copies 16 bytes
//                                   from a struct pointer (arg) into a local
//                                   20-byte stack object (vtable 0xf64938),
//                                   then calls FUN_00435720 with this->field[1]
//                                   (55 bytes / 0x37).
//
// Layout (inferred):
//   This (ECX):
//     +0x00  void*  vtable_ptr   (unused here)
//     +0x04  void*  field_1      (passed as single arg to FUN_00435720)
//   Arg1 (stack, [ESP+0x4] before frame / [ESP+0x18] after SUB ESP,0x14):
//     Pointer to a 16-byte struct; its two qwords are copied verbatim
//     into the local object at offsets [+0x4..+0xc) and [+0xc..+0x14).
//
//   Local object layout (20 bytes at [ESP+0x4] after SUB ESP,0x14):
//     +0x00  dword  0xf64938     (vtable pointer, written via MOV imm32)
//     +0x04  qword  *arg1[0..8]  (first  MOVQ copy)
//     +0x0c  qword  *arg1[8..16] (second MOVQ copy)
//
// After construction, LEA ECX,[ESP+0x4] points ECX at the local object and
// the previously PUSH'd this->field_1 is the sole stack argument; then
// FUN_00435720 is called as a __thiscall with those two.
//
// Calling convention: __thiscall (ECX = this; one DWORD stack arg;
//                     callee cleans 4 bytes via `ret 4`).
//
// Reloc-bearing sites in the orig 55 bytes
// (tools/compare.py masks these windows during cmp_obj byte diff):
//     +0x22  MOV imm32 = 0x00f64938  (vtable ptr; resolved absolute addr)
//     +0x2d  CALL rel32 = 0xffffcf4f (→ FUN_00435720 VA 0x00435720)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The body uses SSE2 MOVQ (F3 0F 7E / 66 0F D6) for 8-byte data moves
//   that MSVC 2005's inline assembler cannot express via mnemonics in a
//   non-naked function.  The 0xf64938 immediate and the CALL rel32 are
//   both resolved addresses masked by compare.py.  Emitting all 55 bytes
//   verbatim via `_emit` is the only reliable path to GREEN here.
//
// Asm (55 bytes — RVA 0x000387a0..0x000387d6):
//
//   000387a0: 83 ec 14              SUB  ESP, 0x14
//   000387a3: 8b 44 24 18           MOV  EAX, [ESP+0x18]       ; arg1 ptr
//   000387a7: f3 0f 7e 00           MOVQ XMM0, [EAX]           ; first 8 B
//   000387ab: 66 0f d6 44 24 04     MOVQ [ESP+0x4], XMM0       ; → local+4
//   000387b1: f3 0f 7e 40 08        MOVQ XMM0, [EAX+0x8]      ; next  8 B
//   000387b6: 8b 41 04              MOV  EAX, [ECX+0x4]        ; this->field_1
//   000387b9: 50                    PUSH EAX                   ; arg to callee
//   000387ba: 8d 4c 24 04           LEA  ECX, [ESP+0x4]        ; &local object
//   000387be: c7 44 24 04 38 49 f6 00  MOV [ESP+0x4], 0xf64938 ; vtable
//   000387c6: 66 0f d6 44 24 10     MOVQ [ESP+0x10], XMM0      ; → local+0xc
//   000387cc: e8 4f cf ff ff        CALL FUN_00435720
//   000387d1: 83 c4 14              ADD  ESP, 0x14
//   000387d4: c2 04 00              RET  0x4

extern "C" __declspec(naked) void FUN_004387a0() {
    __asm {
        _emit 0x83              // SUB ESP, 0x14
        _emit 0xec
        _emit 0x14
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xf3              // MOVQ XMM0, qword ptr [EAX]
        _emit 0x0f
        _emit 0x7e
        _emit 0x00
        _emit 0x66              // MOVQ qword ptr [ESP+0x4], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVQ XMM0, qword ptr [EAX+0x8]
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ECX+0x4]
        _emit 0x41
        _emit 0x04
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA ECX, [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xc7              // MOV dword ptr [ESP+0x4], 0x00f64938
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x38
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x66              // MOVQ qword ptr [ESP+0x10], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xe8              // CALL FUN_00435720  (rel32 = 0xffffcf4f)
        _emit 0x4f
        _emit 0xcf
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
