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
// FUNCTION: ffxivgame 0x00019d60 — __thiscall method that packs a flags word
//                                   from arg3, calls an inner function via a
//                                   global pointer, clears a dirty byte on
//                                   this+0x30, and returns a bounds-checked
//                                   pointer addition. (102 B / 0x66)
//
// Signature:
//   __thiscall int FUN_00419d60(SomeObj *this, int arg1, int arg2, int arg3)
//
// Stack at entry (thiscall — ECX = this, callee cleans 0xc bytes):
//   [ESP+0x04]  int arg1
//   [ESP+0x08]  int arg2
//   [ESP+0x0c]  int arg3
//
// Object fields accessed (layout offsets):
//   +0x18  dword — passed as first arg to the inner call
//   +0x20  dword — used as base pointer / start for return value & bounds check
//   +0x24  dword — used as end pointer for bounds check
//   +0x30  byte  — saved early, passed as 5th arg to inner call, zeroed after
//
// Logic outline:
//   1. byte_flag = (byte)this->field_0x30
//   2. flags = ((((arg3 & 3) * 8) | (arg3 & 4)) << 9 | (arg3 & 8)) << 1
//      (implemented with three ADD ECX,ECX for *8, then SHL ECX,9, then ADD ECX,ECX)
//   3. Call FUN_00423340(__thiscall via ECX=[0x0132987c]):
//        push this->field_0x18 (1st arg), arg1, arg2, flags, byte_flag (5th)
//        → callee does ret 0x14, cleaning all 5 args
//   4. this->field_0x30 = 0
//   5. Bounds check: assert(this->field_0x20 && arg1 < (this->field_0x24 - this->field_0x20))
//      — on failure calls FUN_009d22b4 (assert/trap)
//   6. return this->field_0x20 + arg1
//
// Absolute address references (resolved in the orig PE at load-base 0x00400000):
//   0x0132987c — global pointer whose value is used as ECX for the inner call
//
// CALL reloc sites (compare.py masks the rel32 bytes):
//   +0x3e   CALL 0x00423340 (rel32 = 9d 95 00 00)
//   +0x57   CALL 0x009d22b4 (rel32 = f8 84 5b 00)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The exact instruction selection — three ADD ECX,ECX for the ×8 step,
//   the precise register allocation across the bitfield build, the absolute
//   address MOV ECX,[0x0132987c] — cannot be reliably coaxed from MSVC 2005
//   at source level. Raw _emit passthrough produces a byte-identical .text.

extern "C" __declspec(naked) void FUN_00419d60() {
    __asm {
        // 00019d60: PUSH ESI
        _emit 0x56
        // 00019d61: MOV ESI, ECX  (ESI = this)
        _emit 0x8b
        _emit 0xf1
        // 00019d63: MOVZX EAX, byte ptr [ESI+0x30]
        _emit 0x0f
        _emit 0xb6
        _emit 0x46
        _emit 0x30
        // 00019d67: PUSH EDI
        _emit 0x57
        // 00019d68: MOV EDI, dword ptr [ESP+0xc]  (EDI = arg1)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // 00019d6c: PUSH EAX  (push byte_flag as 5th arg to inner call)
        _emit 0x50
        // 00019d6d: MOV EAX, dword ptr [ESP+0x18]  (EAX = arg3)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 00019d71: MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 00019d73: AND ECX, 3
        _emit 0x83
        _emit 0xe1
        _emit 0x03
        // 00019d76: ADD ECX, ECX  (× 2)
        _emit 0x03
        _emit 0xc9
        // 00019d78: ADD ECX, ECX  (× 4)
        _emit 0x03
        _emit 0xc9
        // 00019d7a: ADD ECX, ECX  (× 8)
        _emit 0x03
        _emit 0xc9
        // 00019d7c: MOV EDX, EAX
        _emit 0x8b
        _emit 0xd0
        // 00019d7e: AND EDX, 4
        _emit 0x83
        _emit 0xe2
        _emit 0x04
        // 00019d81: OR ECX, EDX
        _emit 0x0b
        _emit 0xca
        // 00019d83: SHL ECX, 9
        _emit 0xc1
        _emit 0xe1
        _emit 0x09
        // 00019d86: AND EAX, 8
        _emit 0x83
        _emit 0xe0
        _emit 0x08
        // 00019d89: OR ECX, EAX
        _emit 0x0b
        _emit 0xc8
        // 00019d8b: MOV EAX, dword ptr [ESP+0x14]  (EAX = arg2)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00019d8f: ADD ECX, ECX  (final ×2 / shift left 1)
        _emit 0x03
        _emit 0xc9
        // 00019d91: PUSH ECX  (push flags — 4th arg)
        _emit 0x51
        // 00019d92: MOV ECX, dword ptr [ESI+0x18]  (ECX = this->field_0x18)
        _emit 0x8b
        _emit 0x4e
        _emit 0x18
        // 00019d95: PUSH EAX  (push arg2 — 3rd arg)
        _emit 0x50
        // 00019d96: PUSH EDI  (push arg1 — 2nd arg)
        _emit 0x57
        // 00019d97: PUSH ECX  (push this->field_0x18 — 1st arg)
        _emit 0x51
        // 00019d98: MOV ECX, dword ptr [0x0132987c]  (ECX = global this)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 00019d9e: CALL 0x00423340  (rel32 = 9d 95 00 00, masked by compare.py)
        _emit 0xe8
        _emit 0x9d
        _emit 0x95
        _emit 0x00
        _emit 0x00
        // 00019da3: MOV byte ptr [ESI+0x30], 0  (this->field_0x30 = 0)
        _emit 0xc6
        _emit 0x46
        _emit 0x30
        _emit 0x00
        // 00019da7: MOV ECX, dword ptr [ESI+0x20]  (ECX = this->field_0x20)
        _emit 0x8b
        _emit 0x4e
        _emit 0x20
        // 00019daa: TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 00019dac: JZ +0x09  (jump to assert if field_0x20 == 0)
        _emit 0x74
        _emit 0x09
        // 00019dae: MOV EAX, dword ptr [ESI+0x24]
        _emit 0x8b
        _emit 0x46
        _emit 0x24
        // 00019db1: SUB EAX, ECX  (EAX = field_0x24 - field_0x20)
        _emit 0x2b
        _emit 0xc1
        // 00019db3: CMP EDI, EAX  (arg1 < capacity?)
        _emit 0x3b
        _emit 0xf8
        // 00019db5: JC +0x05  (skip assert if arg1 < capacity)
        _emit 0x72
        _emit 0x05
        // 00019db7: CALL 0x009d22b4  (assert/trap; rel32 = f8 84 5b 00, masked)
        _emit 0xe8
        _emit 0xf8
        _emit 0x84
        _emit 0x5b
        _emit 0x00
        // 00019dbc: MOV EAX, dword ptr [ESI+0x20]  (EAX = this->field_0x20)
        _emit 0x8b
        _emit 0x46
        _emit 0x20
        // 00019dbf: ADD EAX, EDI  (EAX = field_0x20 + arg1)
        _emit 0x03
        _emit 0xc7
        // 00019dc1: POP EDI
        _emit 0x5f
        // 00019dc2: POP ESI
        _emit 0x5e
        // 00019dc3: RET 0xc  (thiscall, clean 3 args = 12 bytes)
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
