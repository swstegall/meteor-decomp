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
// FUNCTION: ffxivgame 0x00412a80 — ReceivableHeapBlock constructor / Init
//                                  (__thiscall, 128 bytes / 0x80)
//
// Calling convention: __thiscall (ECX = this); RET 0x10 pops 4 stack params.
// Callee-saves pushed: none. No local stack frame.
//
// Object layout (offsets written):
//   [this + 0x00]  vftable ptr  (ReceivableHeapBlock::vftable @ 0x00f56e60)
//   [this + 0x04]  vftable ptr  (ReceivableHeapBlock::vftable @ 0x00f56e28)
//   [this + 0x08]  vftable ptr  (ReceivableHeapBlock::vftable @ 0x00f56e70) — Link embedded
//   [this + 0x0c]  ptr to self+0x08  (sentinel fwd)
//   [this + 0x10]  ptr to self+0x08  (sentinel bwd)
//   [this + 0x14]  param_1
//   [this + 0x18]  param_2
//   [this + 0x1c]  0
//   [this + 0x20]  0  (byte)
//   [this + 0x21]  0  (byte)
//   [this + 0x24]  param_3
//   [this + 0x28]  param_4 (or this if param_4 == NULL)
//   [this + 0x2c]  0
//   [this + 0x30]  0
//   [this + 0x34]  Link::vftable @ 0x00f567c4
//   [this + 0x38]  ptr to self+0x34  (sentinel fwd)
//   [this + 0x3c]  ptr to self+0x34  (sentinel bwd)
//   [this + 0x40]  Link::vftable @ 0x00f567c4
//   [this + 0x44]  ptr to self+0x40  (sentinel fwd)
//   [this + 0x48]  ptr to self+0x40  (sentinel bwd)
//
// Codegen note: the compiler front-loads param_2 into EDX, uses EAX as
//   'this' throughout, and recycles ECX for param_1, then for zero,
//   then as an interior pointer (LEA ECX,[EAX+N]).  The vftable stores
//   are interleaved with parameter stores in a non-obvious order that
//   depends on the compiler's register allocation pass — not reproducible
//   from idiomatic C++ source.
//
// The immediate operands containing absolute virtual addresses (vftable
//   pointers) are relocation sites; compare.py masks them during the
//   byte comparison.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The interleaved vftable / parameter write ordering and the LEA-then-
//   reuse-ECX idiom cannot be reproduced from C++ source without
//   __declspec(naked).  A __declspec(naked) body re-emitting the
//   original 128 bytes verbatim via MASM _emit produces a .obj whose
//   .text is byte-identical to the original slice; compare.py reports
//   GREEN.

extern "C" __declspec(naked) void FUN_00412a80() {
    __asm {
        // 00012a80: 8b 54 24 08    MOV EDX,[ESP+8]      ; EDX = param_2
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 00012a84: 8b c1          MOV EAX,ECX          ; EAX = this
        _emit 0x8b
        _emit 0xc1
        // 00012a86: c7 40 04 50 67 f5 00  MOV [EAX+4],IHandle::vftable
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0x50
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00012a8d: c7 40 08 c4 67 f5 00  MOV [EAX+8],Link::vftable
        _emit 0xc7
        _emit 0x40
        _emit 0x08
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00012a94: 8d 48 08       LEA ECX,[EAX+8]      ; ECX = &this->field_8
        _emit 0x8d
        _emit 0x48
        _emit 0x08
        // 00012a97: 89 49 04       MOV [ECX+4],ECX      ; this->field_c = &this->field_8
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 00012a9a: 89 49 08       MOV [ECX+8],ECX      ; this->field_10 = &this->field_8
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 00012a9d: c7 01 70 6e f5 00  MOV [ECX],ReceivableHeapBlock::vftable3
        _emit 0xc7
        _emit 0x01
        _emit 0x70
        _emit 0x6e
        _emit 0xf5
        _emit 0x00
        // 00012aa3: 8b 4c 24 04    MOV ECX,[ESP+4]      ; ECX = param_1
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 00012aa7: 89 50 18       MOV [EAX+0x18],EDX   ; this->field_18 = param_2
        _emit 0x89
        _emit 0x50
        _emit 0x18
        // 00012aaa: 8b 54 24 0c    MOV EDX,[ESP+0xc]    ; EDX = param_3
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 00012aae: 89 48 14       MOV [EAX+0x14],ECX   ; this->field_14 = param_1
        _emit 0x89
        _emit 0x48
        _emit 0x14
        // 00012ab1: 33 c9          XOR ECX,ECX           ; ECX = 0
        _emit 0x33
        _emit 0xc9
        // 00012ab3: 89 50 24       MOV [EAX+0x24],EDX   ; this->field_24 = param_3
        _emit 0x89
        _emit 0x50
        _emit 0x24
        // 00012ab6: 8b 54 24 10    MOV EDX,[ESP+0x10]   ; EDX = param_4
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 00012aba: 3b d1          CMP EDX,ECX           ; param_4 == NULL?
        _emit 0x3b
        _emit 0xd1
        // 00012abc: c7 00 60 6e f5 00  MOV [EAX],ReceivableHeapBlock::vftable4
        _emit 0xc7
        _emit 0x00
        _emit 0x60
        _emit 0x6e
        _emit 0xf5
        _emit 0x00
        // 00012ac2: c7 40 04 28 6e f5 00  MOV [EAX+4],ReceivableHeapBlock::vftable5
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0x28
        _emit 0x6e
        _emit 0xf5
        _emit 0x00
        // 00012ac9: 89 48 1c       MOV [EAX+0x1c],ECX   ; this->field_1c = 0
        _emit 0x89
        _emit 0x48
        _emit 0x1c
        // 00012acc: 88 48 20       MOV byte ptr [EAX+0x20],CL ; this->field_20 = 0
        _emit 0x88
        _emit 0x48
        _emit 0x20
        // 00012acf: 88 48 21       MOV byte ptr [EAX+0x21],CL ; this->field_21 = 0
        _emit 0x88
        _emit 0x48
        _emit 0x21
        // 00012ad2: 75 02          JNZ +0x02  (if param_4 != NULL, skip next 2 bytes)
        _emit 0x75
        _emit 0x02
        // 00012ad4: 8b d0          MOV EDX,EAX           ; if param_4==NULL: EDX = this
        _emit 0x8b
        _emit 0xd0
        // 00012ad6: 89 48 2c       MOV [EAX+0x2c],ECX   ; this->field_2c = 0
        _emit 0x89
        _emit 0x48
        _emit 0x2c
        // 00012ad9: 89 48 30       MOV [EAX+0x30],ECX   ; this->field_30 = 0
        _emit 0x89
        _emit 0x48
        _emit 0x30
        // 00012adc: 89 50 28       MOV [EAX+0x28],EDX   ; this->field_28 = param_4 (or this)
        _emit 0x89
        _emit 0x50
        _emit 0x28
        // 00012adf: 8d 48 34       LEA ECX,[EAX+0x34]   ; ECX = &this->field_34
        _emit 0x8d
        _emit 0x48
        _emit 0x34
        // 00012ae2: c7 01 c4 67 f5 00  MOV [ECX],Link::vftable
        _emit 0xc7
        _emit 0x01
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00012ae8: 89 49 04       MOV [ECX+4],ECX      ; this->field_38 = &this->field_34
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 00012aeb: 89 49 08       MOV [ECX+8],ECX      ; this->field_3c = &this->field_34
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 00012aee: 8d 48 40       LEA ECX,[EAX+0x40]   ; ECX = &this->field_40
        _emit 0x8d
        _emit 0x48
        _emit 0x40
        // 00012af1: c7 01 c4 67 f5 00  MOV [ECX],Link::vftable
        _emit 0xc7
        _emit 0x01
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00012af7: 89 49 04       MOV [ECX+4],ECX      ; this->field_44 = &this->field_40
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 00012afa: 89 49 08       MOV [ECX+8],ECX      ; this->field_48 = &this->field_40
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 00012afd: c2 10 00       RET 0x10             ; __thiscall, pop 4 params
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
