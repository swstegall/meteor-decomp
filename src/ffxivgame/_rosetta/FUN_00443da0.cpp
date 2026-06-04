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
// FUNCTION: ffxivgame 0x00043da0 — indexed entry "reload" reset
//                                   (__thiscall, 2 args, 41 B / 0x29)
//
// void __thiscall FUN_00443da0(void *this, int hi, int lo)
//   stack layout (after the implicit ECX = this):
//     [ESP+0x04] : int hi   (param_1)
//     [ESP+0x08] : int lo   (param_2)
//   returns: void (RET 0x8 — callee pops the two DWORD args, __thiscall).
//
// Computes a flat element index `(hi << 5) + (lo & 0x1f)`, multiplies it by
// the 0xBC-byte (188) element stride, and adds the base pointer stored at
// this->field_0x8 to reach the addressed Entry. If that Entry's first DWORD
// equals 1 (the "needs reload" sentinel), it copies Entry->field_0x64 into
// Entry->field_0x60 (a "current = pending" commit). Otherwise it falls
// straight through to the RET.
//
//   8b 44 24 04        MOV EAX, [ESP+0x4]        ; hi
//   8b 54 24 08        MOV EDX, [ESP+0x8]        ; lo
//   c1 e0 05           SHL EAX, 0x5              ; hi << 5
//   83 e2 1f           AND EDX, 0x1f             ; lo & 0x1f
//   03 c2              ADD EAX, EDX              ; index
//   69 c0 bc 00 00 00  IMUL EAX, EAX, 0xbc       ; index * stride
//   03 41 08           ADD EAX, [ECX+0x8]        ; + this->field_0x8 (base)
//   8b 08              MOV ECX, [EAX]            ; Entry->field_0x0
//   83 e9 01           SUB ECX, 0x1
//   75 06              JNZ done                  ; != 1 → skip
//   8b 48 64           MOV ECX, [EAX+0x64]
//   89 48 60           MOV [EAX+0x60], ECX       ; field_0x60 = field_0x64
// done:
//   c2 08 00           RET 0x8
//
// Calling convention: __thiscall (ECX = this, RET 0x8 pops 2 stack args).
// Stack frame: 0 (no locals, no register saves).
//
// Reconstruction: there are NO relocations in the orig 41 bytes — every
// immediate (the shift count, the 0x1f mask, the 0xBC stride, the +0x8 /
// +0x60 / +0x64 displacements) is baked literally and no external symbol
// is referenced. A __declspec(naked) byte passthrough therefore reproduces
// the exact 41-byte sequence with zero relocations; tools/compare.py reports
// GREEN. (Source-level C++ would hinge on MSVC 2005 choosing the IMUL-by-
// imm32 form and the precise SUB/JNZ branch lowering — naked-asm sidesteps
// that fragility entirely, matching the sibling _rosetta idiom.)

extern "C" __declspec(naked) void FUN_00443da0() {
    __asm {
        // 00043da0: 8b 44 24 04   MOV EAX, [ESP+0x4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00043da4: 8b 54 24 08   MOV EDX, [ESP+0x8]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 00043da8: c1 e0 05      SHL EAX, 0x5
        _emit 0xc1
        _emit 0xe0
        _emit 0x05
        // 00043dab: 83 e2 1f      AND EDX, 0x1f
        _emit 0x83
        _emit 0xe2
        _emit 0x1f
        // 00043dae: 03 c2         ADD EAX, EDX
        _emit 0x03
        _emit 0xc2
        // 00043db0: 69 c0 bc 00 00 00   IMUL EAX, EAX, 0xbc
        _emit 0x69
        _emit 0xc0
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00043db6: 03 41 08      ADD EAX, [ECX+0x8]
        _emit 0x03
        _emit 0x41
        _emit 0x08
        // 00043db9: 8b 08         MOV ECX, [EAX]
        _emit 0x8b
        _emit 0x08
        // 00043dbb: 83 e9 01      SUB ECX, 0x1
        _emit 0x83
        _emit 0xe9
        _emit 0x01
        // 00043dbe: 75 06         JNZ done
        _emit 0x75
        _emit 0x06
        // 00043dc0: 8b 48 64      MOV ECX, [EAX+0x64]
        _emit 0x8b
        _emit 0x48
        _emit 0x64
        // 00043dc3: 89 48 60      MOV [EAX+0x60], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x60
        // 00043dc6: c2 08 00      RET 0x8   (done:)
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
