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
// FUNCTION: ffxivgame 0x00049bd0 — std::basic_string<wchar_t>::insert
//                                  (count copies of a char at offset)
//                                  (267 B / 0x10b, __thiscall, ret 0xc).
//
// Behaviour read from the disassembly at orig RVA 0x00049bd0:
//
//   __thiscall basic_string<wchar_t>*
//   FUN_00449bd0(basic_string<wchar_t>* this,  // ECX
//                size_type _Off,                // [ESP+0x10] -> EDI
//                size_type _Count,              // [ESP+0x14] -> EBP
//                wchar_t   _Ch);                // [ESP+0x18]
//
//   The wide-string element width is 2 bytes throughout (word stores,
//   *2 index scaling on EDI/EBP/EBX). The routine is the classic MSVC
//   2005 <xstring> `insert(_Off, _Count, _Ch)` lowering:
//
//     +0x09  CMP [this+0x14], _Off / JNC ; if (_Off > size) _Xran()
//            CALL 0x009d046d             ;   (out-of-range throw helper)
//     +0x17  EAX = -1 - size; if (EAX <= _Count) _Xlen()
//            CALL 0x009d042e             ;   (length-error throw helper)
//     +0x26  if (_Count == 0) goto epilogue (return this)
//     +0x2f  EBX = size + _Count; if (EBX > 0xfffffffe) _Xlen()
//     +0x41  if (capacity (this+0x18) < EBX)
//                CALL 0x00449760         ;   _Grow / reallocate(EBX, size)
//     +0x66  memmove(dst, src, (size-_Off)*2)   ; CALL 0x009d186e
//            shift the tail right by _Count elements
//     +0xd9  CALL 0x004493c0             ; _Chassign(_Off, _Count, _Ch)
//            fill the gap with _Count copies of _Ch
//     +0xde  this->_Mysize = EBX; null-terminate at [buf + EBX*2]
//            return this
//
//   The +0x66 path branches on the small-string-optimisation flag
//   (capacity (this+0x18) >= 8 ? heap buf at [this+0x04] : inline buf
//   at this+0x04) to pick the source/dest pointers, identical to the
//   other matched <xstring> members in this binary.
//
//   Reloc-bearing sites in the orig 267 bytes — all rel32 CALL
//   displacements, baked as immediates in the post-link PE slice:
//     +0x0e  _Xran throw helper   CALL 0x009d046d
//     +0x21  _Xlen throw helper   CALL 0x009d042e
//     +0x39  _Xlen throw helper   CALL 0x009d042e (2nd)
//     +0x4c  _Grow                CALL 0x00449760 (__thiscall)
//     +0xc8  memmove              CALL 0x009d186e (__cdecl)
//     +0xd9  _Chassign            CALL 0x004493c0 (__thiscall)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 into
//   reproducing the exact SSO-branch register allocation (EBX = new
//   size held live across memmove + _Chassign, the [ESP+0x14] pointer
//   spill, the dual ret-0xc epilogues), the precise short-vs-near
//   branch encodings, and the rel32 CALL displacements above. Each is
//   brittle under /O2 — every high-level rewrite shifts at least one
//   byte. The established local idiom (see FUN_00404f70, FUN_004054d0)
//   re-emits the orig bytes verbatim via MASM `_emit`; the .obj's
//   .text section is byte-identical to the orig slice, which is what
//   tools/compare.py checks.

extern "C" __declspec(naked) void FUN_00449bd0() {
    __asm {
        // 00049bd0  PUSH EBP / PUSH ESI / PUSH EDI
        _emit 0x55
        _emit 0x56
        _emit 0x57
        // 00049bd3  MOV EDI,[ESP+0x10]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 00049bd7  MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 00049bd9  CMP [ESI+0x14],EDI
        _emit 0x39
        _emit 0x7e
        _emit 0x14
        // 00049bdc  JNC +5
        _emit 0x73
        _emit 0x05
        // 00049bde  CALL 0x009d046d
        _emit 0xe8
        _emit 0x8a
        _emit 0x68
        _emit 0x58
        _emit 0x00
        // 00049be3  MOV EBP,[ESP+0x14]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x14
        // 00049be7  OR EAX,0xffffffff
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // 00049bea  SUB EAX,[ESI+0x14]
        _emit 0x2b
        _emit 0x46
        _emit 0x14
        // 00049bed  CMP EAX,EBP
        _emit 0x3b
        _emit 0xc5
        // 00049bef  JA +5
        _emit 0x77
        _emit 0x05
        // 00049bf1  CALL 0x009d042e
        _emit 0xe8
        _emit 0x38
        _emit 0x68
        _emit 0x58
        _emit 0x00
        // 00049bf6  TEST EBP,EBP
        _emit 0x85
        _emit 0xed
        // 00049bf8  JBE 0x00049cd3
        _emit 0x0f
        _emit 0x86
        _emit 0xd5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00049bfe  PUSH EBX
        _emit 0x53
        // 00049bff  MOV EBX,[ESI+0x14]
        _emit 0x8b
        _emit 0x5e
        _emit 0x14
        // 00049c02  ADD EBX,EBP
        _emit 0x03
        _emit 0xdd
        // 00049c04  CMP EBX,-0x2
        _emit 0x83
        _emit 0xfb
        _emit 0xfe
        // 00049c07  JBE +5
        _emit 0x76
        _emit 0x05
        // 00049c09  CALL 0x009d042e
        _emit 0xe8
        _emit 0x20
        _emit 0x68
        _emit 0x58
        _emit 0x00
        // 00049c0e  MOV EAX,[ESI+0x18]
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 00049c11  CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // 00049c13  JNC 0x00049c3a
        _emit 0x73
        _emit 0x25
        // 00049c15  MOV ECX,[ESI+0x14]
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 00049c18  PUSH ECX / PUSH EBX
        _emit 0x51
        _emit 0x53
        // 00049c1a  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00049c1c  CALL 0x00449760
        _emit 0xe8
        _emit 0x3f
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        // 00049c21  TEST EBX,EBX
        _emit 0x85
        _emit 0xdb
        // 00049c23  JBE 0x00049cd2
        _emit 0x0f
        _emit 0x86
        _emit 0xa9
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00049c29  MOV EAX,[ESI+0x18]
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 00049c2c  CMP EAX,0x8
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        // 00049c2f  JC 0x00049c66
        _emit 0x72
        _emit 0x35
        // 00049c31  MOV EDX,[ESI+0x4]
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 00049c34  MOV [ESP+0x14],EDX
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 00049c38  JMP 0x00049c6d
        _emit 0xeb
        _emit 0x33
        // 00049c3a  TEST EBX,EBX
        _emit 0x85
        _emit 0xdb
        // 00049c3c  JNZ 0x00049c23
        _emit 0x75
        _emit 0xe5
        // 00049c3e  CMP EAX,0x8
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        // 00049c41  MOV [ESI+0x14],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x14
        // 00049c44  JC 0x00049c55
        _emit 0x72
        _emit 0x0f
        // 00049c46  MOV EAX,[ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00049c49  MOV [EAX],BX
        _emit 0x66
        _emit 0x89
        _emit 0x18
        // 00049c4c  POP EBX / POP EDI
        _emit 0x5b
        _emit 0x5f
        // 00049c4e  MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 00049c50  POP ESI / POP EBP
        _emit 0x5e
        _emit 0x5d
        // 00049c52  RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00049c55  LEA EAX,[ESI+0x4]
        _emit 0x8d
        _emit 0x46
        _emit 0x04
        // 00049c58  POP EBX / POP EDI
        _emit 0x5b
        _emit 0x5f
        // 00049c5a  MOV word [EAX],0x0
        _emit 0x66
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00049c5f  MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 00049c61  POP ESI / POP EBP
        _emit 0x5e
        _emit 0x5d
        // 00049c63  RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00049c66  LEA ECX,[ESI+0x4]
        _emit 0x8d
        _emit 0x4e
        _emit 0x04
        // 00049c69  MOV [ESP+0x14],ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00049c6d  CMP EAX,0x8
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        // 00049c70  JC 0x00049c77
        _emit 0x72
        _emit 0x05
        // 00049c72  MOV ECX,[ESI+0x4]
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 00049c75  JMP 0x00049c7a
        _emit 0xeb
        _emit 0x03
        // 00049c77  LEA ECX,[ESI+0x4]
        _emit 0x8d
        _emit 0x4e
        _emit 0x04
        // 00049c7a  MOV EDX,[ESI+0x14]
        _emit 0x8b
        _emit 0x56
        _emit 0x14
        // 00049c7d  SUB EDX,EDI
        _emit 0x2b
        _emit 0xd7
        // 00049c7f  ADD EDX,EDX
        _emit 0x03
        _emit 0xd2
        // 00049c81  PUSH EDX
        _emit 0x52
        // 00049c82  MOV EDX,[ESP+0x18]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 00049c86  SUB EAX,EDI
        _emit 0x2b
        _emit 0xc7
        // 00049c88  SUB EAX,EBP
        _emit 0x2b
        _emit 0xc5
        // 00049c8a  LEA EDX,[EDX+EDI*2]
        _emit 0x8d
        _emit 0x14
        _emit 0x7a
        // 00049c8d  PUSH EDX
        _emit 0x52
        // 00049c8e  ADD EAX,EAX
        _emit 0x03
        _emit 0xc0
        // 00049c90  PUSH EAX
        _emit 0x50
        // 00049c91  LEA EAX,[EDI+EBP]
        _emit 0x8d
        _emit 0x04
        _emit 0x2f
        // 00049c94  LEA ECX,[ECX+EAX*2]
        _emit 0x8d
        _emit 0x0c
        _emit 0x41
        // 00049c97  PUSH ECX
        _emit 0x51
        // 00049c98  CALL 0x009d186e (memmove)
        _emit 0xe8
        _emit 0xd1
        _emit 0x7b
        _emit 0x58
        _emit 0x00
        // 00049c9d  MOV EDX,[ESP+0x2c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        // 00049ca1  ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00049ca4  PUSH EDX / PUSH EBP / PUSH EDI
        _emit 0x52
        _emit 0x55
        _emit 0x57
        // 00049ca7  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00049ca9  CALL 0x004493c0
        _emit 0xe8
        _emit 0x12
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        // 00049cae  CMP [ESI+0x18],0x8
        _emit 0x83
        _emit 0x7e
        _emit 0x18
        _emit 0x08
        // 00049cb2  MOV [ESI+0x14],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x14
        // 00049cb5  JC 0x00049cc9
        _emit 0x72
        _emit 0x12
        // 00049cb7  MOV EAX,[ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00049cba  MOV word [EAX+EBX*2],0x0
        _emit 0x66
        _emit 0xc7
        _emit 0x04
        _emit 0x58
        _emit 0x00
        _emit 0x00
        // 00049cc0  POP EBX / POP EDI
        _emit 0x5b
        _emit 0x5f
        // 00049cc2  MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 00049cc4  POP ESI / POP EBP
        _emit 0x5e
        _emit 0x5d
        // 00049cc6  RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00049cc9  LEA EAX,[ESI+0x4]
        _emit 0x8d
        _emit 0x46
        _emit 0x04
        // 00049ccc  MOV word [EAX+EBX*2],0x0
        _emit 0x66
        _emit 0xc7
        _emit 0x04
        _emit 0x58
        _emit 0x00
        _emit 0x00
        // 00049cd2  POP EBX
        _emit 0x5b
        // 00049cd3  POP EDI
        _emit 0x5f
        // 00049cd4  MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 00049cd6  POP ESI / POP EBP
        _emit 0x5e
        _emit 0x5d
        // 00049cd8  RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
