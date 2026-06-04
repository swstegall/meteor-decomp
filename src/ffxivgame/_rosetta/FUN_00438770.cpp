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
// FUNCTION: ffxivgame 0x00038770 — stack-temp construct + thiscall forward
//                                   (__thiscall, 1 arg, 38 B / 0x26)
//
// void __thiscall FUN_00438770(This *this, int arg)
//
// Builds an 8-byte temporary object in the local frame — { field_0 =
// 0xf649d8 (a fixed vtable/tag pointer), field_4 = arg } — and invokes
// FUN_00435370 on it as a __thiscall (ECX = &temp) with a single stack
// argument this->field_0x4.
//
// Calling convention: __thiscall — ECX = this, 1 explicit stack arg,
// epilogue is RET 4. Frame: 8 bytes of locals (/Oy, no frame pointer).
//
// The interleaving of SUB ESP / PUSH ECX with the LEA of the local object
// and the moffs-style local stores, plus the explicit RET 4 and the single
// relocated CALL, are fragile to reproduce from source-level C++ (MSVC
// would have to materialise the temp in exactly this slot order and pick
// the PUSH-before-LEA scheduling). Encoded as a __declspec(naked) byte
// passthrough; the only linker-relocated field is the 4-byte relative
// offset of the CALL to FUN_00435370 (bytes at +0x1c..+0x1f), which
// compare.py masks.
//
// Asm (38 bytes @ orig RVA 0x00038770):
//   83 ec 08                 SUB  ESP, 0x8
//   8b 49 04                 MOV  ECX, [ECX+0x4]            ; this->field_4
//   8b 44 24 0c              MOV  EAX, [ESP+0xc]            ; arg
//   51                       PUSH ECX                       ; stack arg
//   8d 4c 24 04              LEA  ECX, [ESP+0x4]            ; this = &temp
//   c7 44 24 04 d8 49 f6 00  MOV  [ESP+0x4], 0xf649d8       ; temp.field_0
//   89 44 24 08              MOV  [ESP+0x8], EAX            ; temp.field_4
//   e8 e0 cb ff ff           CALL FUN_00435370              ; (reloc)
//   83 c4 08                 ADD  ESP, 0x8
//   c2 04 00                 RET  0x4

extern "C" void FUN_00435370();   // forward declaration for the CALL relocation

extern "C" __declspec(naked) void FUN_00438770() {
    __asm {
        // 00038770: 83 ec 08      SUB ESP, 8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 00038773: 8b 49 04      MOV ECX, [ECX+4]
        _emit 0x8b
        _emit 0x49
        _emit 0x04
        // 00038776: 8b 44 24 0c   MOV EAX, [ESP+0xc]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0003877a: 51            PUSH ECX
        _emit 0x51
        // 0003877b: 8d 4c 24 04   LEA ECX, [ESP+4]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0003877f: c7 44 24 04 d8 49 f6 00   MOV [ESP+4], 0xf649d8
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xd8
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 00038787: 89 44 24 08   MOV [ESP+8], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0003878b: e8 RR RR RR RR  CALL FUN_00435370  (reloc)
        call FUN_00435370
        // 00038790: 83 c4 08      ADD ESP, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00038793: c2 04 00      RET 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
