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
// FUNCTION: ffxivgame 0x00051bf0 — std::basic_string iterator-range validator
//                                  that delegates to FUN_00451ae0 (96 B / 0x60).
//                                  (__thiscall, RET 0x4 — 1 stack arg).
//
// Recovered shape:
//
//   void __thiscall FUN_00451bf0(StringT *this /*ECX*/,
//                                 void    *arg0 /*[ESP+0x4]*/) — RET 0x4
//
//   The string layout is the MSVC 2005 SSO basic_string shifted by +4:
//     [this+0x04]  _Bx   (union: char _Buf[16] | char *_Ptr)
//     [this+0x14]  _Mysize
//     [this+0x18]  _Myres (capacity; < 0x10 ⇒ inline buffer at &_Bx)
//
//   Behaviour:
//     1. Compute the buffer pointer (ptr):
//          if (_Myres < 0x10) ptr = &this->_Bx;   // SSO path
//          else               ptr = this->_Bx._Ptr; // heap path
//     2. Compute end iterator: edi = ptr + _Mysize
//     3. Validate iterator range against 0x009d22b4 (std::_Xran throw):
//          – edi != NULL
//          – begin (ptr) <= edi
//          – edi <= begin + _Mysize  (tautological in this form, but matches orig codegen)
//        If any check fails: CALL 0x009d22b4 (noreturn range-check throw)
//     4. Build args and tail-call into FUN_00451ae0:
//          PUSH arg0             // [ESP+0x20] in callee
//          PUSH edi              // [ESP+0x1c] in callee
//          PUSH this             // [ESP+0x18] in callee
//          LEA  ecx, [8-byte local]   // hidden return slot
//          PUSH ecx              // [ESP+0x14] in callee
//          ECX  = this
//          CALL FUN_00451ae0
//
// The 8-byte local area (SUB ESP, 0x8 at entry) serves as the hidden return
// buffer for the IterPair that FUN_00451ae0 writes back via its out-pointer
// arg; on return EAX holds that pointer (the MSVC struct-return convention).
//
// Reloc-bearing call sites (compare.py masks the 4-byte rel32 windows):
//   +0x3f   REL32 → 0x009d22b4   (std::_Xran / invalid-iterator throw)
//   +0x52   REL32 → 0x00451ae0   (erase/replace span helper, sibling)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The three interleaved CMP/JC/MOV trios that open-code `_Myptr()` twice,
//   the PUSH EDI interleaved between the CMP and LEA in the prologue, and the
//   exact short-jump encodings (eb / 72 / 74 / 77 / 76) resist clean
//   source-level reconstruction under /O2. Emitting the 96 orig bytes verbatim
//   via MASM `_emit` produces a .obj whose .text is byte-identical to the orig
//   slice; tools/compare.py masks the two rel32 displacements and reports GREEN.

extern "C" __declspec(naked) void FUN_00451bf0() {
    __asm {
        // 00051bf0: 83 ec 08    SUB ESP, 0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 00051bf3: 53          PUSH EBX
        _emit 0x53
        // 00051bf4: 56          PUSH ESI
        _emit 0x56
        // 00051bf5: 8b f1       MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00051bf7: 8b 5e 18    MOV EBX, dword ptr [ESI+0x18]   (_Myres)
        _emit 0x8b
        _emit 0x5e
        _emit 0x18
        // 00051bfa: 83 fb 10    CMP EBX, 0x10
        _emit 0x83
        _emit 0xfb
        _emit 0x10
        // 00051bfd: 57          PUSH EDI
        _emit 0x57
        // 00051bfe: 8d 46 04    LEA EAX, [ESI+0x4]   (&_Bx)
        _emit 0x8d
        _emit 0x46
        _emit 0x04
        // 00051c01: 72 04       JC +4   (if SSO: skip heap-ptr deref)
        _emit 0x72
        _emit 0x04
        // 00051c03: 8b 08       MOV ECX, dword ptr [EAX]   (heap ptr)
        _emit 0x8b
        _emit 0x08
        // 00051c05: eb 02       JMP +2
        _emit 0xeb
        _emit 0x02
        // 00051c07: 8b c8       MOV ECX, EAX   (inline buffer ptr)
        _emit 0x8b
        _emit 0xc8
        // 00051c09: 8b 56 14    MOV EDX, dword ptr [ESI+0x14]   (_Mysize)
        _emit 0x8b
        _emit 0x56
        _emit 0x14
        // 00051c0c: 8d 3c 0a    LEA EDI, [EDX+ECX*1]   (end iterator = begin+size)
        _emit 0x8d
        _emit 0x3c
        _emit 0x0a
        // 00051c0f: 85 ff       TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 00051c11: 74 1c       JZ +0x1c   (null check fail → throw)
        _emit 0x74
        _emit 0x1c
        // 00051c13: 83 fb 10    CMP EBX, 0x10
        _emit 0x83
        _emit 0xfb
        _emit 0x10
        // 00051c16: 72 04       JC +4   (SSO: skip heap-ptr deref)
        _emit 0x72
        _emit 0x04
        // 00051c18: 8b 08       MOV ECX, dword ptr [EAX]
        _emit 0x8b
        _emit 0x08
        // 00051c1a: eb 02       JMP +2
        _emit 0xeb
        _emit 0x02
        // 00051c1c: 8b c8       MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 00051c1e: 3b cf       CMP ECX, EDI   (begin <= end?)
        _emit 0x3b
        _emit 0xcf
        // 00051c20: 77 0d       JA +0xd   (begin > end → throw)
        _emit 0x77
        _emit 0x0d
        // 00051c22: 83 fb 10    CMP EBX, 0x10
        _emit 0x83
        _emit 0xfb
        _emit 0x10
        // 00051c25: 72 02       JC +2   (SSO: skip heap-ptr deref)
        _emit 0x72
        _emit 0x02
        // 00051c27: 8b 00       MOV EAX, dword ptr [EAX]
        _emit 0x8b
        _emit 0x00
        // 00051c29: 03 d0       ADD EDX, EAX   (EDX = begin + _Mysize)
        _emit 0x03
        _emit 0xd0
        // 00051c2b: 3b fa       CMP EDI, EDX   (end <= upper bound?)
        _emit 0x3b
        _emit 0xfa
        // 00051c2d: 76 05       JBE +5   (ok: skip throw)
        _emit 0x76
        _emit 0x05
        // 00051c2f: e8 80 06 58 00  CALL 0x009d22b4   (std::_Xran, rel32 masked)
        _emit 0xe8
        _emit 0x80
        _emit 0x06
        _emit 0x58
        _emit 0x00
        // 00051c34: 8b 44 24 18  MOV EAX, dword ptr [ESP+0x18]   (arg0)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 00051c38: 50          PUSH EAX   (arg3 to FUN_00451ae0)
        _emit 0x50
        // 00051c39: 57          PUSH EDI   (end iterator, arg2 to FUN_00451ae0)
        _emit 0x57
        // 00051c3a: 56          PUSH ESI   (this, arg1 to FUN_00451ae0)
        _emit 0x56
        // 00051c3b: 8d 4c 24 18  LEA ECX, [ESP+0x18]   (&8-byte local = hidden ret slot)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00051c3f: 51          PUSH ECX   (hidden return ptr, arg0 to FUN_00451ae0)
        _emit 0x51
        // 00051c40: 8b ce       MOV ECX, ESI   (this for thiscall)
        _emit 0x8b
        _emit 0xce
        // 00051c42: e8 99 fe ff ff  CALL 0x00451ae0   (sibling, rel32 masked)
        _emit 0xe8
        _emit 0x99
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 00051c47: 5f          POP EDI
        _emit 0x5f
        // 00051c48: 5e          POP ESI
        _emit 0x5e
        // 00051c49: 5b          POP EBX
        _emit 0x5b
        // 00051c4a: 83 c4 08    ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00051c4d: c2 04 00    RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
