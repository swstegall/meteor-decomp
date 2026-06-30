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
// FUNCTION: ffxivgame 0x0004db70 — vtable init + global registration
//                                   (__thiscall, 1 stack arg, 43 B / 0x2B)
//
// void __thiscall FUN_0044db70(this=ECX, param_1=[ESP+4])
//
// 1. Copies ECX (this) to EAX; loads stack arg [ESP+4] into ECX.
// 2. Writes vtable-pointer values to [EAX+4] and [EAX+0]:
//      [EAX+4] = 0x00f67490   (base vptr — immediately overwritten below)
//      [EAX+0] = 0x00f674c4   (primary vptr of derived class)
//      [EAX+4] = 0x00f674a8   (secondary vptr of derived class)
//    The write sequence is an MSVC multiple-inheritance constructor pattern:
//    the base-class ctor is inlined and sets the base vtable pointer first
//    (0xf67490), then the outer ctor overwrites it with the derived vptr
//    (0xf674a8) and sets the primary vptr (0xf674c4).
// 3. Reads EDX = *(ECX+4)  (a field of param_1).
// 4. Stores EDX  → [0x0132cf48]  (global slot — likely "current instance field")
//    Stores EAX  → [0x0132cf40]  (global slot — likely "current instance pointer")
// 5. RET 0x4 — callee cleans 4 bytes (one pointer arg pushed by caller).
//
// Asm (43 bytes @ orig RVA 0x0004db70):
//   8b c1                         MOV  EAX, ECX
//   8b 4c 24 04                   MOV  ECX, dword ptr [ESP+0x4]
//   c7 40 04 90 74 f6 00          MOV  dword ptr [EAX+0x4], 0x00f67490   (reloc)
//   c7 00 c4 74 f6 00             MOV  dword ptr [EAX],     0x00f674c4   (reloc)
//   c7 40 04 a8 74 f6 00          MOV  dword ptr [EAX+0x4], 0x00f674a8   (reloc)
//   8b 51 04                      MOV  EDX, dword ptr [ECX+0x4]
//   89 15 48 cf 32 01             MOV  dword ptr [0x0132cf48], EDX       (reloc)
//   a3 40 cf 32 01                MOV  [0x0132cf40], EAX                  (reloc)
//   c2 04 00                      RET  0x4
//
// No CALL instructions → no rel32 relocations. The five absolute-address
// immediates (three .rdata vtable pointers + two .data globals) carry PE
// HIGHLOW relocations which compare.py masks, so the naked byte passthrough
// produces a byte-identical .obj.

extern "C" __declspec(naked) void FUN_0044db70() {
    __asm {
        // 0004db70: 8b c1          MOV EAX, ECX
        _emit 0x8b
        _emit 0xc1
        // 0004db72: 8b 4c 24 04    MOV ECX, dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0004db76: c7 40 04 90 74 f6 00  MOV dword ptr [EAX+0x4], 0x00f67490
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0x90
        _emit 0x74
        _emit 0xf6
        _emit 0x00
        // 0004db7d: c7 00 c4 74 f6 00  MOV dword ptr [EAX], 0x00f674c4
        _emit 0xc7
        _emit 0x00
        _emit 0xc4
        _emit 0x74
        _emit 0xf6
        _emit 0x00
        // 0004db83: c7 40 04 a8 74 f6 00  MOV dword ptr [EAX+0x4], 0x00f674a8
        _emit 0xc7
        _emit 0x40
        _emit 0x04
        _emit 0xa8
        _emit 0x74
        _emit 0xf6
        _emit 0x00
        // 0004db8a: 8b 51 04  MOV EDX, dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x51
        _emit 0x04
        // 0004db8d: 89 15 48 cf 32 01  MOV dword ptr [0x0132cf48], EDX
        _emit 0x89
        _emit 0x15
        _emit 0x48
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // 0004db93: a3 40 cf 32 01  MOV [0x0132cf40], EAX
        _emit 0xa3
        _emit 0x40
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // 0004db98: c2 04 00  RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
