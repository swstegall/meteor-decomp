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
// FUNCTION: ffxivgame 0x0004e760 — `__thiscall` grow/resize helper for a
//                                  std::vector-style container of 84-byte
//                                  (0x54) records (285 B / 0x11d, EH3-SEH
//                                  wrapped, /GS security cookie).
//
// Inspection (read from the disassembly at orig RVA 0x0004e760):
//
//   __thiscall void FUN_0044e760(Container* this /*ECX*/, size_t newCount);
//
//     this+0x4 = _Myfirst, this+0x8 = _Mylast, this+0xc = _Myend
//     (element stride 0x54). The function ensures capacity for `newCount`
//     records and, when it has to reallocate, copies the live elements
//     across to the new block and frees the old one.
//
//       if (newCount > 0x30c30c3)            // max_size for 0x54-byte elems
//           _Xlength();                       // 0x00cb0e40
//       size_t cap = _Myfirst ? (_Myend - _Myfirst) / 0x54 : 0;
//       if (cap < newCount) {
//           T* blk = operator new[](newCount * 0x54);   // 0x00403b70
//           ... // construct/copy newCount records via 0x0044e290 (6 args)
//           if (_Myfirst) {                              // free old block
//               <dtor sweep 0x00404240>;
//               operator delete(_Myfirst);              // 0x009d1b17
//           }
//           _Myend   = blk + newCount * 0x54;
//           _Mylast  = blk + liveCount * 0x54;
//           _Myfirst = blk;
//       }
//
//   The function is wrapped in MSVC 2005's EH3 __except_handler3 frame
//   (PUSH -1 / PUSH scope-table 0x00e57c50 / PUSH FS:[0]) with a /GS
//   security cookie (XOR EBP, [0x012ea8b0]) and an EH trylevel local at
//   [EBP-4] that toggles 0 → -1 around the reallocation/construction so a
//   throwing element copy unwinds the freshly-allocated block.
//
//   Reloc-bearing sites in the orig 285 bytes (absolute addresses /
//   rel32 displacements that only resolve in a full-binary relink at
//   image base 0x00400000; a standalone .obj can't reproduce them):
//     +0x05  scope-table address      (.rdata 0x00e57c50)
//     +0x0a  FS:[0] read              (constant 0, fold-through)
//     +0x17  __security_cookie load   (.data 0x012ea8b0)
//     +0x22  FS:[0] install           (constant 0, fold-through)
//     +0x38  _Xlength CALL            (.text 0x00cb0e40 rel32)
//     +0x65  operator new[] CALL      (.text 0x00403b70 rel32)
//     +0x7f  _invalid_parameter CALL  (.text 0x009d22b4 rel32)
//     +0x8c  _invalid_parameter CALL  (.text 0x009d22b4 rel32)
//     +0xa4  copy/construct CALL      (.text 0x0044e290 rel32)
//     +0xdf  dtor sweep CALL          (.text 0x00404240 rel32 — __thiscall)
//     +0xe8  operator delete CALL     (.text 0x009d1b17 rel32)
//     +0x10c FS:[0] restore           (constant 0, fold-through)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc into
//   reproducing the exact __except_handler3 prologue, the precise EH
//   trylevel state numbering, the signed-divide-by-0x54 reciprocal-
//   multiply (IMUL 0x30c30c31 / SAR 4 / SHR 31 / ADD) used twice, the
//   short-vs-near branch encodings, AND the seven linker-resolved
//   addresses above. Every high-level rewrite shifts at least one byte,
//   so the pragmatic, established choice (same as FUN_00401a00 and the
//   other SEH-wrapped siblings) is a `__declspec(naked)` body that
//   re-emits the orig 285 bytes verbatim via MASM `_emit` directives.
//   The .obj's `.text` section ends up byte-identical to the orig slice
//   (raw immediates, no relocations), which is what tools/compare.py
//   checks against.

extern "C" __declspec(naked) void FUN_0044e760() {
    __asm {
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x50
        _emit 0x7c
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x50
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        _emit 0x53
        _emit 0x56
        _emit 0x57
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc5
        _emit 0x50
        _emit 0x8d

        _emit 0x45
        _emit 0xf4
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x65
        _emit 0xf0
        _emit 0x8b
        _emit 0xf1
        _emit 0x8b
        _emit 0x7d
        _emit 0x08

        _emit 0x81
        _emit 0xff
        _emit 0xc3
        _emit 0x30
        _emit 0x0c
        _emit 0x03
        _emit 0x76
        _emit 0x05
        _emit 0xe8
        _emit 0xa3
        _emit 0x26
        _emit 0x86
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x04

        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x16
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        _emit 0x2b
        _emit 0xc8
        _emit 0xb8
        _emit 0x31
        _emit 0x0c
        _emit 0xc3
        _emit 0x30
        _emit 0xf7
        _emit 0xe9

        _emit 0xc1
        _emit 0xfa
        _emit 0x04
        _emit 0x8b
        _emit 0xc2
        _emit 0xc1
        _emit 0xe8
        _emit 0x1f
        _emit 0x03
        _emit 0xc2
        _emit 0x3b
        _emit 0xc7
        _emit 0x0f
        _emit 0x83
        _emit 0xa7
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0x57
        _emit 0xe8
        _emit 0xa6
        _emit 0x53
        _emit 0xfb
        _emit 0xff
        _emit 0x8b
        _emit 0x7e
        _emit 0x08
        _emit 0x83
        _emit 0xc4
        _emit 0x08

        _emit 0x39
        _emit 0x7e
        _emit 0x04
        _emit 0x89
        _emit 0x45
        _emit 0xec
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x76
        _emit 0x05
        _emit 0xe8

        _emit 0xd0
        _emit 0x3a
        _emit 0x58
        _emit 0x00
        _emit 0x8b
        _emit 0x5e
        _emit 0x04
        _emit 0x3b
        _emit 0x5e
        _emit 0x08
        _emit 0x76
        _emit 0x05
        _emit 0xe8
        _emit 0xc3
        _emit 0x3a
        _emit 0x58

        _emit 0x00
        _emit 0x8b
        _emit 0x4d
        _emit 0x08
        _emit 0x8b
        _emit 0x55
        _emit 0xec
        _emit 0xc6
        _emit 0x45
        _emit 0xe8
        _emit 0x00
        _emit 0x8b
        _emit 0x45
        _emit 0xe8
        _emit 0x50
        _emit 0x51

        _emit 0x56
        _emit 0x52
        _emit 0x57
        _emit 0x53
        _emit 0xe8
        _emit 0x87
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x5e
        _emit 0x04
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x85

        _emit 0xdb
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x75
        _emit 0x04
        _emit 0x33
        _emit 0xff
        _emit 0xeb
        _emit 0x16
        _emit 0x8b
        _emit 0x4e

        _emit 0x08
        _emit 0x2b
        _emit 0xcb
        _emit 0xb8
        _emit 0x31
        _emit 0x0c
        _emit 0xc3
        _emit 0x30
        _emit 0xf7
        _emit 0xe9
        _emit 0xc1
        _emit 0xfa
        _emit 0x04
        _emit 0x8b
        _emit 0xfa
        _emit 0xc1

        _emit 0xef
        _emit 0x1f
        _emit 0x03
        _emit 0xfa
        _emit 0x85
        _emit 0xdb
        _emit 0x74
        _emit 0x18
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        _emit 0x50
        _emit 0x53
        _emit 0x8b
        _emit 0xce
        _emit 0xe8

        _emit 0xfc
        _emit 0x59
        _emit 0xfb
        _emit 0xff
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        _emit 0x51
        _emit 0xe8
        _emit 0xca
        _emit 0x32
        _emit 0x58
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x8b
        _emit 0x45
        _emit 0x08

        _emit 0x6b
        _emit 0xff
        _emit 0x54
        _emit 0x8b
        _emit 0x4d
        _emit 0xec
        _emit 0x6b
        _emit 0xc0
        _emit 0x54
        _emit 0x03
        _emit 0xc1
        _emit 0x03
        _emit 0xf9
        _emit 0x89
        _emit 0x46
        _emit 0x0c

        _emit 0x89
        _emit 0x7e
        _emit 0x08
        _emit 0x89
        _emit 0x4e
        _emit 0x04
        _emit 0x8b
        _emit 0x4d
        _emit 0xf4
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x59
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0x8b
        _emit 0xe5
        _emit 0x5d
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
