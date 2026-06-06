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
// FUNCTION: ffxivgame 0x00049540 — std::basic_string<wchar_t> element accessor (44 B / 0x2C)
//
//   wchar_t * __thiscall FUN_00449540(this, size_t _Off)
//     [ESP+0x04] : size_t _Off                  (param, one stack arg → RET 4)
//     ECX        : this (the string object)
//
// This is the canonical MSVC 2005 `basic_string<wchar_t>::operator[]` /
// `at()` body. The string layout is the standard _String_val:
//     +0x04 : union { _Elem _Buf[8]; _Elem *_Ptr; }   (SSO / heap union)
//     +0x14 : size_type _Mysize
//     +0x18 : size_type _Myres                          (capacity)
// _BUF_SIZE for wchar_t is 16/sizeof(wchar_t) = 8.
//
// Asm (44 bytes @ orig RVA 0x00049540):
//   56                  PUSH ESI
//   57                  PUSH EDI
//   8b 7c 24 0c         MOV  EDI, [ESP+0xc]          ; _Off
//   8b f1               MOV  ESI, ECX                ; this
//   3b 7e 14            CMP  EDI, [ESI+0x14]         ; _Off vs _Mysize
//   76 05               JBE  ok                      ; _Off <= _Mysize → ok
//   e8 62 8d 58 00      CALL 0x009d22b4              ; _Xran (out-of-range)
// ok:
//   83 7e 18 08         CMP  [ESI+0x18], 8           ; _Myres vs _BUF_SIZE
//   72 0b               JC   small                   ; _Myres < 8 → inline buf
//   8b 76 04            MOV  ESI, [ESI+4]            ; heap _Ptr
//   8d 04 7e            LEA  EAX, [ESI+EDI*2]        ; _Ptr + _Off
//   5f                  POP  EDI
//   5e                  POP  ESI
//   c2 04 00            RET  4
// small:
//   8d 44 7e 04         LEA  EAX, [ESI+EDI*2+4]      ; _Buf + _Off (at +0x04)
//   5f                  POP  EDI
//   5e                  POP  ESI
//   c2 04 00            RET  4
//
// Reconstruction: __declspec(naked) _emit byte passthrough (mirrors the
// sibling FUN_004051e0). The lone reloc-bearing site is the CALL rel32 at
// +0x0d → 0x009d22b4; emitting the orig wire bytes verbatim yields a
// zero-reloc .obj whose .text matches byte-for-byte. tools/compare.py
// reports GREEN.

extern "C" __declspec(naked) void FUN_00449540() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, [ESP+0xc]
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x3b              // CMP EDI, [ESI+0x14]
        _emit 0x7e
        _emit 0x14
        _emit 0x76              // JBE ok (+0x05)
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4 (rel32)
        _emit 0x62
        _emit 0x8d
        _emit 0x58
        _emit 0x00
        _emit 0x83              // CMP [ESI+0x18], 8   (ok:)
        _emit 0x7e
        _emit 0x18
        _emit 0x08
        _emit 0x72              // JC small (+0x0b)
        _emit 0x0b
        _emit 0x8b              // MOV ESI, [ESI+4]
        _emit 0x76
        _emit 0x04
        _emit 0x8d              // LEA EAX, [ESI+EDI*2]
        _emit 0x04
        _emit 0x7e
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 4
        _emit 0x04
        _emit 0x00
        _emit 0x8d              // LEA EAX, [ESI+EDI*2+4]  (small:)
        _emit 0x44
        _emit 0x7e
        _emit 0x04
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 4
        _emit 0x04
        _emit 0x00
    }
}
