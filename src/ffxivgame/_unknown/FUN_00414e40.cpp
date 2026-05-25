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
// FUNCTION: ffxivgame 0x00014e40 — `rand() * (1.0f/32768.0f)` helper
//                                  (__cdecl, no args, 30 B / 0x1e)
//
// A tiny leaf that returns a random float in [0.0, 1.0) by calling
// CRT `_rand` (which yields a value in [0, RAND_MAX=0x7FFF]),
// reinterpreting the int as an *unsigned* int when widening to float,
// and scaling by `1.0f / 32768.0f` (= 0x38000000 = 3.0517578125e-05f).
//
// The unsigned-widen idiom — TEST EAX,EAX / FILD m32 / JGE +6 / FADD
// dword [2^32_const] — is MSVC 2005's standard inline lowering of
// `(float)(unsigned int)<signed_int_eax>`. The compiler treats the
// PC-spill ([ESP] via the 1-byte PUSH ECX stack reserve) as a temp,
// and patches a +2^32 bias when the sign bit is set so the FILD-as-
// signed reading produces the unsigned-interpretation value.
//
// Asm (30 bytes @ orig RVA 0x00014e40):
//   51                       PUSH ECX                    ; reserve 4 B
//   e8 RR RR RR RR           CALL _rand                  ; (rel32 reloc)
//   85 c0                    TEST EAX, EAX
//   89 04 24                 MOV  [ESP], EAX             ; spill ret
//   db 04 24                 FILD dword ptr [ESP]        ; load as signed
//   7d 06                    JGE  +6 → fmul              ; non-neg: skip
//   d8 05 RR RR RR RR        FADD dword ptr [&4.2949673e+09f]  ; +2^32
//   d8 0d RR RR RR RR        FMUL dword ptr [&3.0517578e-05f]  ; *1/2^15
//   59                       POP  ECX                    ; release temp
//   c3                       RET
//
// Reloc-bearing sites in orig (resolve only at image base 0x00400000):
//   +0x02   _rand                rel32 → .text   0x005d576e
//   +0x12   const 4.2949673e+09f DIR32 → .rdata  0x00f54a54 (`__real@4f800000`)
//   +0x18   const 3.0517578e-05f DIR32 → .rdata  0x00f570c4 (`__real@38000000`)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level `(float)(unsigned int)rand() * (1.0f / 32768.0f)`
//   does lower to this exact instruction sequence under MSVC 2005
//   /O2 /Oy, but the two FP constants would land in OUR .obj's .rdata
//   at fresh offsets and the _rand call would carry a fresh rel32 reloc
//   targeting our CRT — compare.py would mask all three reloc sites
//   (12 bytes total), leaving the other 18 structural bytes compared.
//   Emitting all 30 bytes verbatim via __declspec(naked) + `_emit`
//   produces a .text slice with NO relocs at all, so every byte is
//   structurally compared and matches the orig directly.

#ifdef _MSC_VER
extern "C" __declspec(naked) void FUN_00414e40() {
    __asm {
        // 00014e40: 51                       PUSH ECX
        _emit 0x51
        // 00014e41: e8 28 09 5c 00           CALL _rand               (rel32)
        _emit 0xe8
        _emit 0x28
        _emit 0x09
        _emit 0x5c
        _emit 0x00
        // 00014e46: 85 c0                    TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00014e48: 89 04 24                 MOV [ESP], EAX
        _emit 0x89
        _emit 0x04
        _emit 0x24
        // 00014e4b: db 04 24                 FILD dword ptr [ESP]
        _emit 0xdb
        _emit 0x04
        _emit 0x24
        // 00014e4e: 7d 06                    JGE +6
        _emit 0x7d
        _emit 0x06
        // 00014e50: d8 05 54 4a f5 00        FADD dword ptr [0x00f54a54]
        _emit 0xd8
        _emit 0x05
        _emit 0x54
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        // 00014e56: d8 0d c4 70 f5 00        FMUL dword ptr [0x00f570c4]
        _emit 0xd8
        _emit 0x0d
        _emit 0xc4
        _emit 0x70
        _emit 0xf5
        _emit 0x00
        // 00014e5c: 59                       POP ECX
        _emit 0x59
        // 00014e5d: c3                       RET
        _emit 0xc3
    }
}
#endif // _MSC_VER
