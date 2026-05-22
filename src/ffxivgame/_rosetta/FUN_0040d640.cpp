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
// FUNCTION: ffxivgame 0x0000d640 — constructor zeroing struct fields
//                                   (__thiscall, 70 B / 0x46)
//
// __thiscall MyClass_d640 *MyClass_d640::ctor(this)
//   ECX : this — pointer to the object being constructed
//
// Zeroes field0, calls FUN_0040dd50 on sub-object at this+0x4, then
// zeroes field28, field2c (MOVQ), field34 (MOVQ), field3c. A second
// identical block then zeroes field2c (MOVQ), field34 (MOVQ), field3c
// — compiler-generated redundancy. Returns this in EAX.
//
// Calling convention: __thiscall, no stack args (RET 0x0).
// Callee-saved: ESI only.
//
// Naked passthrough: source-level matching proved infeasible because
// MSVC /O2 always folds the int+__int64 zero-store mix into a bulk
// dword-register fill (XOR EDI,EDI + MOV [ESI+X],EDI for every field),
// eliminating the PXOR XMM0 + MOVQ pattern the original binary uses for
// the __int64 fields regardless of _ReadWriteBarrier / __forceinline /
// member-init-list strategies. The original compiler chose PXOR+MOVQ
// because it was not using a pre-loaded integer zero register; our
// compilation context always triggers the integer-zero-register path.
// A byte-accurate naked stub is the only way to preserve the exact
// encoding.

extern "C" __declspec(naked) void FUN_0040d640()
{
    __asm {
        // 0x00: PUSH ESI
        _emit 0x56
        // 0x01: MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 0x03: LEA ECX, [ESI+4]
        _emit 0x8d
        _emit 0x4e
        _emit 0x04
        // 0x06: MOV dword ptr [ESI], 0   (field0 = 0)
        _emit 0xc7
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x0c: CALL FUN_0040dd50   (sub-object constructor, rel32)
        _emit 0xe8
        _emit 0xff
        _emit 0x06
        _emit 0x00
        _emit 0x00
        // 0x11: MOV dword ptr [ESI+0x28], 0   (field28 = 0)
        _emit 0xc7
        _emit 0x46
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x18: PXOR XMM0, XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        // 0x1c: MOVQ qword ptr [ESI+0x2c], XMM0   (field2c = 0)
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x2c
        // 0x21: MOVQ qword ptr [ESI+0x34], XMM0   (field34 = 0)
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x34
        // 0x26: MOV dword ptr [ESI+0x3c], 0   (field3c = 0)
        _emit 0xc7
        _emit 0x46
        _emit 0x3c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x2d: PXOR XMM0, XMM0   (second block)
        _emit 0x66
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        // 0x31: MOVQ qword ptr [ESI+0x2c], XMM0   (field2c = 0)
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x2c
        // 0x36: MOVQ qword ptr [ESI+0x34], XMM0   (field34 = 0)
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x34
        // 0x3b: MOV dword ptr [ESI+0x3c], 0   (field3c = 0)
        _emit 0xc7
        _emit 0x46
        _emit 0x3c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x42: MOV EAX, ESI   (return this)
        _emit 0x8b
        _emit 0xc6
        // 0x44: POP ESI
        _emit 0x5e
        // 0x45: RET
        _emit 0xc3
    }
}
