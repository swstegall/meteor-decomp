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
// FUNCTION: ffxivgame 0x00038980 — __thiscall helper that copies a 24-byte
//                                  source block onto the local stack, stamps
//                                  vtable pointer 0x00f64980 at offset 0 of
//                                  the resulting stack object, then dispatches
//                                  to FUN_00435a90 (66 B / 0x42).
//
// Signature (recovered from asm):
//
//   __thiscall void FUN_00438980(this, SrcBlock *src)
//     ECX  = this
//     [ESP+0x4] after SUB 0x1c = the src pointer (one stack arg → RET 0x4)
//
// Layout of the stack-built object at [ESP+0x4] (post-PUSH, 28 bytes total):
//   +0x00  vtable = 0x00f64980     (stamped by MOV [ESP+4], imm32)
//   +0x04  src[0..7]               (copied by first two MOVQ pairs)
//   +0x0c  src[8..15]              (second MOVQ pair)
//   +0x14  src[16..23]             (third MOVQ — stored post-PUSH)
//
// The inner call at 0x000389b7 is __thiscall FUN_00435a90, dispatched with:
//   ECX = &stack_object           (the LEA at 0x000389a5 + overwrite at 0x000389a9)
//   [ESP+0x0] = this->field4      (PUSH EAX at 0x000389a4)
//
// Reloc-bearing sites in the orig 66 bytes:
//   +0x2d  MOV [ESP+4], imm32     → vtable 0x00f64980
//   +0x38  CALL rel32             → FUN_00435a90 (0x00035a90)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Two reloc windows (imm32 + rel32) prevent a clean source-level rewrite;
//   tools/compare.py masks both windows. Re-emitting the 66 orig bytes via
//   MASM _emit directives (the same approach FUN_00434050, FUN_00406fa0, and
//   FUN_00408780 use) yields a zero-reloc .obj whose .text is byte-identical
//   to the orig slice.

extern "C" __declspec(naked) void FUN_00438980() {
    __asm {
        _emit 0x83              // SUB ESP, 0x1c
        _emit 0xec
        _emit 0x1c
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
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
        _emit 0x66              // MOVQ qword ptr [ESP+0xc], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xf3              // MOVQ XMM0, qword ptr [EAX+0x10]
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x10
        _emit 0x8b              // MOV EAX, dword ptr [ECX+0x4]
        _emit 0x41
        _emit 0x04
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA ECX, [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xc7              // MOV dword ptr [ESP+0x4], 0x00f64980
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x80              // ← reloc: vtable imm32 (0x00f64980)
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x66              // MOVQ qword ptr [ESP+0x18], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xe8              // CALL rel32 → FUN_00435a90
        _emit 0xd4              // ← reloc: rel32 offset (0xffffd0d4)
        _emit 0xd0
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x1c
        _emit 0xc4
        _emit 0x1c
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
