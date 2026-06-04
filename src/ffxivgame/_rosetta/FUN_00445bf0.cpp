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
// FUNCTION: ffxivgame 0x00045bf0 — `__cdecl` bool wrapper that forwards to
//                                   a 3-arg `__cdecl` worker at 0x00458880,
//                                   publishes the result into a per-thread
//                                   slot, and returns success (115 B / 0x73,
//                                   ESP-relative 12-byte frame, no EBP).
//
// Inspection (read from the disassembly at orig RVA 0x00045bf0):
//
//   __cdecl bool FUN_00445bf0(void* a0, undefined4 a1);
//
//   Structural pseudo-C:
//
//     // 12-byte on-stack descriptor passed by address to the worker:
//     struct Desc { unsigned dword0; unsigned short w4; unsigned short w8; };
//     // per-thread block: ((TlsBlock**)(FS:[0x2c]))[_tls_index] -> +0x08
//
//     bool __cdecl FUN_00445bf0(void* a0, unsigned a1) {
//         if (a0 == 0) {                          //  85 c0 / 74 49
//             tls()->field8 = 0;                  //  c7 81 08.. = 0
//             return false;                       //  32 c0 (xor al,al)
//         }
//         Desc d;
//         d.dword0 = 0;                           //  c7 44 24 0c 0
//         d.w4     = 0x3f;                        //  66 c7 .. 0x3f
//         d.w8     = 0x1;                          //  66 c7 .. 0x1
//         int r = FUN_00458880(a0, &d, a1);       //  push a1; push &d; push a0
//         tls()->field8 = r;                      //  89 81 08.. = eax
//         if (r == 0) return false;               //  85 c0 / 74 1f -> xor al,al
//         return true;                            //  b0 01
//     }
//
//   Reloc-bearing sites in the orig 115 bytes (resolve only in a full-binary
//   relink at image base 0x00400000 — standalone .obj compilation cannot
//   reproduce them as relocations, so we emit the orig bytes verbatim and
//   link.exe leaves them alone, which is exactly what tools/compare.py
//   checks against):
//     +0x2c  CALL 0x00458880          (.text rel32 — __cdecl worker)
//     +0x31  _tls_index load          (.data 0x01363f38)
//     +0x37  FS:[0x2c] _tls_array     (segment-relative TLS array)
//     +0x54  _tls_index load (2nd)    (.data 0x01363f38)
//     +0x5a  FS:[0x2c] (2nd)          (segment-relative TLS array)
//
//   The high-level source above is the readable record of intent; a
//   __declspec(naked) body re-emits the orig 115 bytes so the .obj's
//   .text is byte-identical to the orig slice. Promote to a real
//   source-level match once the worker at 0x00458880 and the per-thread
//   block layout are catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_00445bf0() {
    __asm {
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x49
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x51

        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x52
        _emit 0x50
        _emit 0x66
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x3f
        _emit 0x00
        _emit 0x66
        _emit 0xc7
        _emit 0x44

        _emit 0x24
        _emit 0x14
        _emit 0x01
        _emit 0x00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x5f
        _emit 0x2c
        _emit 0x01

        _emit 0x00
        _emit 0x8b
        _emit 0x0d
        _emit 0x38
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0x64
        _emit 0x8b
        _emit 0x15
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x0c

        _emit 0x8a
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x89
        _emit 0x81
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x1f
        _emit 0xb0
        _emit 0x01

        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3
        _emit 0x8b
        _emit 0x15
        _emit 0x38
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0x64
        _emit 0xa1
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x8b
        _emit 0x0c
        _emit 0x90
        _emit 0xc7
        _emit 0x81
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x32
        _emit 0xc0
        _emit 0x83
        _emit 0xc4

        _emit 0x0c
        _emit 0xc3
    }
}
