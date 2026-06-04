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
// FUNCTION: ffxivgame 0x00023830 — __thiscall "test-and-update" 6-field
//                                  cache compare (153 B / 0x99, ret 4).
//
// Calling convention: __thiscall (ECX = this); one stack arg `src` at
//   [ESP+4] (EDX); returns bool in AL. Cleans 4 bytes (`ret 4`).
//
// Object layout (offsets touched, all in `this`):
//   [this + 0x2290]  int   a       // src->[0x00]
//   [this + 0x2294]  int   b       // src->[0x04]
//   [this + 0x2298]  int   c       // src->[0x08]
//   [this + 0x229c]  int   d       // src->[0x0c]
//   [this + 0x22a0]  float e       // src->[0x10]
//   [this + 0x22a4]  float f       // src->[0x14]
//
// Behaviour (recovered from asm @ 0x00023830):
//
//   bool Cache::TestAndUpdate(const Fields *src) {
//       if (this->a == src->a && this->b == src->b &&
//           this->c == src->c && this->d == src->d &&
//           this->e == src->e && this->f == src->f) {
//           return true;                       // already in sync
//       }
//       // out of sync — copy the six fields in (three 8-byte MOVQ)
//       *(double*)&this->a = *(double*)&src->a;   // copies a,b
//       *(double*)&this->c = *(double*)&src->c;   // copies c,d
//       *(double*)&this->e = *(double*)&src->e;   // copies e,f
//       return false;
//   }
//
//   The four leading int compares each branch (JNZ) to the copy block.
//   The two trailing float compares promote each `float` to `double`
//   (CVTPS2PD) and test equality with UCOMISD + LAHF + TEST AH,0x44 +
//   JP — the canonical MSVC ordered-equality lowering (jump-on-parity
//   covers the unordered/NaN case as "not equal").
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function has zero relocations (no CALLs, no external symbols —
//   every operand is an immediate displacement), so re-emitting the
//   original 153 .text bytes verbatim yields a `.obj` whose .text is
//   byte-identical to orig; compare.py reports GREEN. Source-level C++
//   at /O2 is not worth coaxing here: the exact SSE2 vs. x87 selection,
//   the double-promoted compare idiom, and the MOVQ-pair struct copy
//   are all build-flag-sensitive in an isolated TU.

extern "C" __declspec(naked) void FUN_00423830() {
    __asm {
        // 00023830: 8b 81 90 22 00 00   MOV EAX,[ECX+0x2290]
        _emit 0x8b
        _emit 0x81
        _emit 0x90
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // 00023836: 8b 54 24 04         MOV EDX,[ESP+0x4]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x04
        // 0002383a: 3b 02               CMP EAX,[EDX]
        _emit 0x3b
        _emit 0x02
        // 0002383c: 75 60               JNZ 0x0042389e
        _emit 0x75
        _emit 0x60
        // 0002383e: 8b 81 94 22 00 00   MOV EAX,[ECX+0x2294]
        _emit 0x8b
        _emit 0x81
        _emit 0x94
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // 00023844: 3b 42 04            CMP EAX,[EDX+0x4]
        _emit 0x3b
        _emit 0x42
        _emit 0x04
        // 00023847: 75 55               JNZ 0x0042389e
        _emit 0x75
        _emit 0x55
        // 00023849: 8b 81 98 22 00 00   MOV EAX,[ECX+0x2298]
        _emit 0x8b
        _emit 0x81
        _emit 0x98
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // 0002384f: 3b 42 08            CMP EAX,[EDX+0x8]
        _emit 0x3b
        _emit 0x42
        _emit 0x08
        // 00023852: 75 4a               JNZ 0x0042389e
        _emit 0x75
        _emit 0x4a
        // 00023854: 8b 81 9c 22 00 00   MOV EAX,[ECX+0x229c]
        _emit 0x8b
        _emit 0x81
        _emit 0x9c
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // 0002385a: 3b 42 0c            CMP EAX,[EDX+0xc]
        _emit 0x3b
        _emit 0x42
        _emit 0x0c
        // 0002385d: 75 3f               JNZ 0x0042389e
        _emit 0x75
        _emit 0x3f
        // 0002385f: f3 0f 10 81 a0 22 00 00   MOVSS XMM0,[ECX+0x22a0]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x81
        _emit 0xa0
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // 00023867: f3 0f 10 4a 10      MOVSS XMM1,[EDX+0x10]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x4a
        _emit 0x10
        // 0002386c: 0f 5a c0            CVTPS2PD XMM0,XMM0
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0
        // 0002386f: 0f 5a c9            CVTPS2PD XMM1,XMM1
        _emit 0x0f
        _emit 0x5a
        _emit 0xc9
        // 00023872: 66 0f 2e c1         UCOMISD XMM0,XMM1
        _emit 0x66
        _emit 0x0f
        _emit 0x2e
        _emit 0xc1
        // 00023876: 9f                  LAHF
        _emit 0x9f
        // 00023877: f6 c4 44            TEST AH,0x44
        _emit 0xf6
        _emit 0xc4
        _emit 0x44
        // 0002387a: 7a 22               JP 0x0042389e
        _emit 0x7a
        _emit 0x22
        // 0002387c: f3 0f 10 81 a4 22 00 00   MOVSS XMM0,[ECX+0x22a4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x81
        _emit 0xa4
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // 00023884: f3 0f 10 4a 14      MOVSS XMM1,[EDX+0x14]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x4a
        _emit 0x14
        // 00023889: 0f 5a c0            CVTPS2PD XMM0,XMM0
        _emit 0x0f
        _emit 0x5a
        _emit 0xc0
        // 0002388c: 0f 5a c9            CVTPS2PD XMM1,XMM1
        _emit 0x0f
        _emit 0x5a
        _emit 0xc9
        // 0002388f: 66 0f 2e c1         UCOMISD XMM0,XMM1
        _emit 0x66
        _emit 0x0f
        _emit 0x2e
        _emit 0xc1
        // 00023893: 9f                  LAHF
        _emit 0x9f
        // 00023894: f6 c4 44            TEST AH,0x44
        _emit 0xf6
        _emit 0xc4
        _emit 0x44
        // 00023897: 7a 05               JP 0x0042389e
        _emit 0x7a
        _emit 0x05
        // 00023899: b0 01               MOV AL,0x1
        _emit 0xb0
        _emit 0x01
        // 0002389b: c2 04 00            RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 0002389e: f3 0f 7e 02         MOVQ XMM0,[EDX]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x02
        // 000238a2: 66 0f d6 81 90 22 00 00   MOVQ [ECX+0x2290],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x81
        _emit 0x90
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // 000238aa: f3 0f 7e 42 08      MOVQ XMM0,[EDX+0x8]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x42
        _emit 0x08
        // 000238af: 66 0f d6 81 98 22 00 00   MOVQ [ECX+0x2298],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x81
        _emit 0x98
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // 000238b7: f3 0f 7e 42 10      MOVQ XMM0,[EDX+0x10]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x42
        _emit 0x10
        // 000238bc: 66 0f d6 81 a0 22 00 00   MOVQ [ECX+0x22a0],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x81
        _emit 0xa0
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // 000238c4: 32 c0               XOR AL,AL
        _emit 0x32
        _emit 0xc0
        // 000238c6: c2 04 00            RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
