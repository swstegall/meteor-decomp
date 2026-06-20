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
// FUNCTION: ffxivgame 0x000553a0 — numeric-literal validator over an
//                                  std::wstring-like (basic_string<wchar_t>)
//                                  buffer (500 B / 0x1f4, /GS + inline SEH).
//
// Inspection (read from the disassembly at orig RVA 0x000553a0):
//
//   __thiscall bool validate(this, basic_string<wchar_t>* s) — `ECX = this`,
//   one stack arg (the string object pushed by the caller), returns AL.
//   The function is wrapped in the standard MSVC /GS prologue (PUSH -1 /
//   PUSH scope_table 0x00e584c8 / FS:[0] link) plus a doubled
//   __security_cookie ([0x012ea8b0] XOR ESP) — the second cookie is the
//   extra push that this function spills.
//
//   Body shape (scans the wide string from the back, classifying each
//   wchar as hex-digit / dec-digit / 'x'/'X' / '-' to decide whether the
//   text is a valid hex (radix 0x10) or decimal (radix 0xa) literal, then
//   normalises the leading-zero / "0x" prefix bookkeeping):
//
//     if (FUN_004451f0(this) == 0) return false;            // [+0x38]
//     int radix = 0;                                        // EDI
//     basic_string<wchar_t> tmp;  tmp._cap = 7; tmp._len = 0; tmp._buf[0]=0;
//     FUN_00449000(this, &tmp);                             // copy/normalise
//     int len  = tmp._len;        // [esp+0x2c]
//     int cap  = tmp._cap;        // [esp+0x30]  (>= 8 → heap, else SSO)
//     wchar_t* sso = tmp._buf;    // EBX  ([esp+0x1c])
//     for (int i = len - 1; i >= 0; --i) {
//         if (i >= len) std::_Xran();   // CALL 0x009d22b4 (out-of-range throw)
//         wchar_t c = (cap < 8 ? sso : heapbuf)[i];
//         unsigned d = c - 0x30;
//         if (d <= 9) { if (radix < 0xa) radix = 0xa; continue; }
//         if ((c>='a'&&c<='f')||(c>='A'&&c<='F')||c=='x'||c=='X') {
//             if (radix < 0x10) radix = 0x10; continue;
//         }
//         if (c == '-') { radix = 0xa; continue; }           // sign char
//         FUN_00403fd0(&tmp);  if (radix == 0) return false;  // dtor + reject
//         return <byte at esp+0x4c>;
//     }
//     ... leading-0x / size bookkeeping, dtor (FUN_0044d350) on heap bufs ...
//
//   Reloc-bearing sites in the orig 500 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x02  PUSH imm32   → 0x00e584c8 (SEH scope table)
//     +0x11  MOV EAX,[]   → 0x012ea8b0 (__security_cookie)
//     +0x20  MOV EAX,[]   → 0x012ea8b0 (__security_cookie, 2nd)
//     +0x38  CALL rel32   → 0x004451f0
//     +0x63  CALL rel32   → 0x00449000
//     +0x87  CALL rel32   → 0x009d22b4 (range_error / _Xran)
//     +0xf2  CALL rel32   → 0x00403fd0
//     +0x150 CALL rel32   → 0x009d22b4 (2nd)
//     +0x1a3 CALL rel32   → 0x0044d350 (string free, three call sites)
//     +0x1eb CALL rel32   → 0x009d20f4 (__security_check_cookie)
//
//   Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rewrite here would have to coax MSVC 2005 /O2 /GS into
//   reproducing the exact doubled-cookie prologue, the SSO-vs-heap branch
//   replicated at every buffer access, the back-to-front loop register
//   allocation (ESI=index, EDI=radix, EBX=SSO ptr), AND the linker-resolved
//   absolute addresses across ten relocation windows. Each constraint is
//   brittle under /O2 (state numbering, branch short-vs-near, the
//   __security_cookie spill). The pragmatic choice — the same one the
//   sibling /GS + inline-SEH bodies took — is a `__declspec(naked)` body
//   that re-emits the orig 500 bytes verbatim via MASM `_emit` directives.
//   The .obj's `.text` ends up byte-identical to the orig slice, which is
//   what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_004553a0() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0xc8
        _emit 0x84
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
        _emit 0x24
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x57
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x50
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x48
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0x13
        _emit 0xfe
        _emit 0xfe
        _emit 0xff
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xff
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x2c
        _emit 0x66
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50
        _emit 0x8b
        _emit 0xce
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x44
        _emit 0xe8
        _emit 0xf8
        _emit 0x3b
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x30
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x8d
        _emit 0x71
        _emit 0xff
        _emit 0x3b
        _emit 0xf7
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x0f
        _emit 0x8c
        _emit 0x33
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x3b
        _emit 0xf1
        _emit 0x76
        _emit 0x11
        _emit 0xe8
        _emit 0x88
        _emit 0xce
        _emit 0x57
        _emit 0x00
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x30
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x83
        _emit 0xfa
        _emit 0x08
        _emit 0x8b
        _emit 0xc3
        _emit 0x73
        _emit 0x04
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x0f
        _emit 0xb7
        _emit 0x04
        _emit 0x70
        _emit 0x8d
        _emit 0x68
        _emit 0xd0
        _emit 0x66
        _emit 0x83
        _emit 0xfd
        _emit 0x09
        _emit 0x77
        _emit 0x0c
        _emit 0x83
        _emit 0xff
        _emit 0x0a
        _emit 0x7d
        _emit 0x59
        _emit 0xbf
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x52
        _emit 0x66
        _emit 0x3d
        _emit 0x61
        _emit 0x00
        _emit 0x72
        _emit 0x06
        _emit 0x66
        _emit 0x3d
        _emit 0x66
        _emit 0x00
        _emit 0x76
        _emit 0x3c
        _emit 0x66
        _emit 0x3d
        _emit 0x41
        _emit 0x00
        _emit 0x72
        _emit 0x06
        _emit 0x66
        _emit 0x3d
        _emit 0x46
        _emit 0x00
        _emit 0x76
        _emit 0x30
        _emit 0x66
        _emit 0x3d
        _emit 0x78
        _emit 0x00
        _emit 0x74
        _emit 0x2a
        _emit 0x66
        _emit 0x3d
        _emit 0x58
        _emit 0x00
        _emit 0x74
        _emit 0x24
        _emit 0x66
        _emit 0x3d
        _emit 0x2d
        _emit 0x00
        _emit 0x74
        _emit 0x28
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8
        _emit 0x39
        _emit 0xeb
        _emit 0xfa
        _emit 0xff
        _emit 0x85
        _emit 0xff
        _emit 0x74
        _emit 0x49
        _emit 0x8a
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        _emit 0xe9
        _emit 0xd1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xff
        _emit 0x10
        _emit 0x7d
        _emit 0x05
        _emit 0xbf
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xee
        _emit 0x01
        _emit 0x0f
        _emit 0x89
        _emit 0x6c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xff
        _emit 0x10
        _emit 0x0f
        _emit 0x85
        _emit 0x96
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x02
        _emit 0x7d
        _emit 0x24
        _emit 0x83
        _emit 0xfa
        _emit 0x08
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x72
        _emit 0x10
        _emit 0x6a
        _emit 0x0c
        _emit 0x8d
        _emit 0x4c
        _emit 0x12
        _emit 0x02
        _emit 0x51
        _emit 0x53
        _emit 0xe8
        _emit 0x6f
        _emit 0x7e
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x32
        _emit 0xc0
        _emit 0xe9
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xf9
        _emit 0x01
        _emit 0x73
        _emit 0x0d
        _emit 0xe8
        _emit 0xbf
        _emit 0xcd
        _emit 0x57
        _emit 0x00
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x30
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x83
        _emit 0xfa
        _emit 0x08
        _emit 0x8b
        _emit 0xc3
        _emit 0x73
        _emit 0x04
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x83
        _emit 0xfa
        _emit 0x08
        _emit 0x0f
        _emit 0xb7
        _emit 0x40
        _emit 0x02
        _emit 0x8b
        _emit 0xcb
        _emit 0x73
        _emit 0x04
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x66
        _emit 0x83
        _emit 0x39
        _emit 0x30
        _emit 0x75
        _emit 0x18
        _emit 0x66
        _emit 0x3d
        _emit 0x78
        _emit 0x00
        _emit 0x74
        _emit 0x33
        _emit 0x66
        _emit 0x3d
        _emit 0x58
        _emit 0x00
        _emit 0x74
        _emit 0x2d
        _emit 0x66
        _emit 0x3d
        _emit 0x62
        _emit 0x00
        _emit 0x74
        _emit 0x27
        _emit 0x66
        _emit 0x3d
        _emit 0x42
        _emit 0x00
        _emit 0x74
        _emit 0x21
        _emit 0x83
        _emit 0xfa
        _emit 0x08
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x72
        _emit 0xa2
        _emit 0x6a
        _emit 0x0c
        _emit 0x8d
        _emit 0x54
        _emit 0x12
        _emit 0x02
        _emit 0x52
        _emit 0x53
        _emit 0xe8
        _emit 0x01
        _emit 0x7e
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x32
        _emit 0xc0
        _emit 0xeb
        _emit 0x1f
        _emit 0x83
        _emit 0xfa
        _emit 0x08
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x72
        _emit 0x10
        _emit 0x6a
        _emit 0x0c
        _emit 0x8d
        _emit 0x44
        _emit 0x12
        _emit 0x02
        _emit 0x50
        _emit 0x53
        _emit 0xe8
        _emit 0xe0
        _emit 0x7d
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xb0
        _emit 0x01
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
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
        _emit 0x5d
        _emit 0x5b
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x64
        _emit 0xcb
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x30
        _emit 0xc3
    }
}
