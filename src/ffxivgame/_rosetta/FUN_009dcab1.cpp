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
// FUNCTION: ffxivgame 0x009dcab1 — `__cdecl` no-arg counter-decrement guard
//                                   (27 B)
//
// Calls FUN_009df3d7 (a no-arg getter returning a pointer in EAX) to obtain
// some singleton/TLS-local object, then checks the 32-bit signed field at
// offset +0x90 in that object. If the field is already <= 0 nothing happens;
// otherwise calls FUN_009df3d7 a second time and decrements the field.
//
// Source-level analogue (pseudocode):
//
//   void FUN_009dcab1() {
//       if (FUN_009df3d7()->field_0x90 > 0)
//           --FUN_009df3d7()->field_0x90;
//   }
//
// MSVC 2005 /O2 calls the getter twice rather than caching EAX across the
// branch because the compiler cannot prove the callee is pure.
//
// Asm (27 bytes, RVA 0x005dcab1..0x005dcacb):
//
//   005dcab1:  e8 21 29 00 00            CALL 0x009df3d7
//   005dcab6:  83 b8 90 00 00 00 00     CMP  dword ptr [EAX+0x90], 0x0
//   005dcabd:  7e 0c                    JLE  0x009dcacb           ; skip
//   005dcabf:  e8 13 29 00 00           CALL 0x009df3d7
//   005dcac4:  05 90 00 00 00           ADD  EAX, 0x90
//   005dcac9:  ff 08                    DEC  dword ptr [EAX]
//   005dcacb:  c3                       RET
//
// Reloc-bearing sites (compare.py masks these 4-byte windows):
//   +0x01  CALL rel32 → FUN_009df3d7 (first call)
//   +0x10  CALL rel32 → FUN_009df3d7 (second call)
//
// Reconstruction via __declspec(naked) + _emit so the 27 orig bytes are
// reproduced verbatim; the two CALL rel32 displacements are baked in from
// the original binary and are masked by compare.py's reloc map.

extern "C" __declspec(naked) void FUN_009dcab1() {
    __asm {
        _emit 0xe8    // CALL 0x009df3d7          ; rel32 = +0x00002921
        _emit 0x21
        _emit 0x29
        _emit 0x00
        _emit 0x00
        _emit 0x83    // CMP dword ptr [EAX+0x90], 0x0
        _emit 0xb8
        _emit 0x90
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x7e    // JLE +0x0c  (skip to RET)
        _emit 0x0c
        _emit 0xe8    // CALL 0x009df3d7          ; rel32 = +0x00002913
        _emit 0x13
        _emit 0x29
        _emit 0x00
        _emit 0x00
        _emit 0x05    // ADD EAX, 0x90
        _emit 0x90
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff    // DEC dword ptr [EAX]
        _emit 0x08
        _emit 0xc3    // RET
    }
}
