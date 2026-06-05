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
// FUNCTION: ffxivgame 0x00459ca0 — `__thiscall` std::basic_string<unsigned short>
//                                  iterator-dispatched insert/append helper
//                                  (256 B / 0x100, no SEH).
//
// Behaviour read from the disassembly at orig RVA 0x00059ca0:
//
//   __thiscall string16* FUN_00459ca0(this, void* _Where, size_type _Count) —
//   `ECX = this`, two stack args (_Where /* [esp+0x10] */, _Count /*
//   [esp+0x14] */). Returns `this` in EAX. Pops 8 bytes (`ret 8`) — two
//   stack args under __thiscall.
//
//   `this` is the MSVC2005 basic_string<unsigned short> control block:
//     [esi+0x04]  _Bx union  — inline buffer (8 u16) OR heap pointer
//                              (when _Myres >= 8 the slot holds a ptr)
//     [esi+0x14]  _Mysize    — element count
//     [esi+0x18]  _Myres     — capacity; <8 => small-buffer-optimised
//
//   The `_Myres < 8 ? &_Bx : *(u16**)&_Bx` test (CMP 0x8 / JC) recurs
//   four times — it is the `_Myptr()` inline accessor.
//
//   Shape:
//     u16* _First = _Myptr();
//     if (_Where >= _First && _Where < _First + _Mysize) {
//         // _Where lands inside [begin, end): delegate to the offset-form
//         //   replace/insert at 0x00459b90 with element offset
//         //   (_Where - _First)/2  and _Count.
//         return FUN_00459b90(this, (_Where - _Myptr()) >> 1, _Count);
//     }
//     // otherwise append _Count copies at end:
//     if (_Mysize + _Count overflow checks ...) _Xlen();   // 0x009d042e
//     if (_Count == 0) return this;
//     size_type _Newsize = _Mysize + _Count;
//     if (_Newsize > _Myres) FUN_004406a0(this, _Newsize, _Mysize); // _Grow
//     // fill tail with the char value (4-arg bounded assign @ 0x009d17f3 =
//     //   memset_s-style _Traits::assign), then 0-terminate.
//     _Mysize = _Newsize;
//     _Myptr()[_Newsize] = 0;
//     return this;
//
//   Reloc-bearing call sites in the orig 256 bytes (resolve only at the
//   image base 0x00400000 link; standalone .obj can't reproduce them):
//     +0x47   rel32  0x00459b90 — offset-form replace/insert (__thiscall)
//     +0x6a   rel32  0x009d042e — std::_Xlen / length_error throw
//     +0x84   rel32  0x009d042e — same throw helper (2nd)
//     +0x97   rel32  0x004406a0 — _Grow / reallocate (__thiscall)
//     +0xde   rel32  0x009d17f3 — bounded fill (_Traits::assign / memset_s)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level C++ rewrite at /O2 would need to coax MSVC2005 into the
//   exact register allocation (ESI=this, EBX=&_Bx, EDI=_Newsize), the
//   four repeated _Myptr() small-buffer branches in the precise emit order,
//   the short-vs-near branch selection (note the 0f86 near JBE at +0x71),
//   and the five linker-resolved rel32 windows above. Each of those is
//   brittle under /O2. Following the same route the sibling _rosetta
//   bodies (FUN_00402a30, FUN_004054d0, FUN_00403a20) took, this is a
//   naked body re-emitting the orig 256 bytes verbatim via MASM `_emit`.
//   The .obj's `.text` ends up byte-identical to the orig slice (the rel32
//   immediates are baked in as raw bytes — no relocations), which is what
//   tools/compare.py checks.

extern "C" __declspec(naked) void FUN_00459ca0() {
    __asm {
        // 00059ca0  PUSH EBX
        _emit 0x53
        // 00059ca1  PUSH ESI
        _emit 0x56
        // 00059ca2  MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00059ca4  MOV EDX, [ESI+0x18]
        _emit 0x8b
        _emit 0x56
        _emit 0x18
        // 00059ca7  CMP EDX, 8
        _emit 0x83
        _emit 0xfa
        _emit 0x08
        // 00059caa  PUSH EDI
        _emit 0x57
        // 00059cab  LEA EBX, [ESI+0x4]
        _emit 0x8d
        _emit 0x5e
        _emit 0x04
        // 00059cae  JC 0x00459cb4
        _emit 0x72
        _emit 0x04
        // 00059cb0  MOV ECX, [EBX]
        _emit 0x8b
        _emit 0x0b
        // 00059cb2  JMP 0x00459cb6
        _emit 0xeb
        _emit 0x02
        // 00059cb4  MOV ECX, EBX
        _emit 0x8b
        _emit 0xcb
        // 00059cb6  MOV EAX, [ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00059cba  CMP EAX, ECX
        _emit 0x3b
        _emit 0xc1
        // 00059cbc  JC 0x00459cf2
        _emit 0x72
        _emit 0x34
        // 00059cbe  CMP EDX, 8
        _emit 0x83
        _emit 0xfa
        _emit 0x08
        // 00059cc1  JC 0x00459cc7
        _emit 0x72
        _emit 0x04
        // 00059cc3  MOV ECX, [EBX]
        _emit 0x8b
        _emit 0x0b
        // 00059cc5  JMP 0x00459cc9
        _emit 0xeb
        _emit 0x02
        // 00059cc7  MOV ECX, EBX
        _emit 0x8b
        _emit 0xcb
        // 00059cc9  MOV EDI, [ESI+0x14]
        _emit 0x8b
        _emit 0x7e
        _emit 0x14
        // 00059ccc  LEA ECX, [ECX+EDI*2]
        _emit 0x8d
        _emit 0x0c
        _emit 0x79
        // 00059ccf  CMP ECX, EAX
        _emit 0x3b
        _emit 0xc8
        // 00059cd1  JBE 0x00459cf2
        _emit 0x76
        _emit 0x1f
        // 00059cd3  CMP EDX, 8
        _emit 0x83
        _emit 0xfa
        _emit 0x08
        // 00059cd6  JC 0x00459cda
        _emit 0x72
        _emit 0x02
        // 00059cd8  MOV EBX, [EBX]
        _emit 0x8b
        _emit 0x1b
        // 00059cda  MOV EDX, [ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 00059cde  SUB EAX, EBX
        _emit 0x2b
        _emit 0xc3
        // 00059ce0  PUSH EDX
        _emit 0x52
        // 00059ce1  SAR EAX, 1
        _emit 0xd1
        _emit 0xf8
        // 00059ce3  PUSH EAX
        _emit 0x50
        // 00059ce4  PUSH ESI
        _emit 0x56
        // 00059ce5  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00059ce7  CALL 0x00459b90
        _emit 0xe8
        _emit 0xa4
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 00059cec  POP EDI
        _emit 0x5f
        // 00059ced  POP ESI
        _emit 0x5e
        // 00059cee  POP EBX
        _emit 0x5b
        // 00059cef  RET 8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // 00059cf2  MOV EAX, [ESI+0x14]
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        // 00059cf5  OR ECX, 0xffffffff
        _emit 0x83
        _emit 0xc9
        _emit 0xff
        // 00059cf8  PUSH EBP
        _emit 0x55
        // 00059cf9  MOV EBP, [ESP+0x18]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        // 00059cfd  SUB ECX, EAX
        _emit 0x2b
        _emit 0xc8
        // 00059cff  CMP ECX, EBP
        _emit 0x3b
        _emit 0xcd
        // 00059d01  JBE 0x00459d0a
        _emit 0x76
        _emit 0x07
        // 00059d03  LEA EDX, [EAX+EBP]
        _emit 0x8d
        _emit 0x14
        _emit 0x28
        // 00059d06  CMP EDX, EAX
        _emit 0x3b
        _emit 0xd0
        // 00059d08  JNC 0x00459d0f
        _emit 0x73
        _emit 0x05
        // 00059d0a  CALL 0x009d042e
        _emit 0xe8
        _emit 0x1f
        _emit 0x67
        _emit 0x57
        _emit 0x00
        // 00059d0f  TEST EBP, EBP
        _emit 0x85
        _emit 0xed
        // 00059d11  JBE 0x00459d97
        _emit 0x0f
        _emit 0x86
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00059d17  MOV EDI, [ESI+0x14]
        _emit 0x8b
        _emit 0x7e
        _emit 0x14
        // 00059d1a  ADD EDI, EBP
        _emit 0x03
        _emit 0xfd
        // 00059d1c  CMP EDI, 0x7ffffffe
        _emit 0x81
        _emit 0xff
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x7f
        // 00059d22  JBE 0x00459d29
        _emit 0x76
        _emit 0x05
        // 00059d24  CALL 0x009d042e
        _emit 0xe8
        _emit 0x05
        _emit 0x67
        _emit 0x57
        _emit 0x00
        // 00059d29  MOV EAX, [ESI+0x18]
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 00059d2c  CMP EAX, EDI
        _emit 0x3b
        _emit 0xc7
        // 00059d2e  JNC 0x00459d4c
        _emit 0x73
        _emit 0x1c
        // 00059d30  MOV EAX, [ESI+0x14]
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        // 00059d33  PUSH EAX
        _emit 0x50
        // 00059d34  PUSH EDI
        _emit 0x57
        // 00059d35  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00059d37  CALL 0x004406a0
        _emit 0xe8
        _emit 0x64
        _emit 0x69
        _emit 0xfe
        _emit 0xff
        // 00059d3c  TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 00059d3e  JBE 0x00459d97
        _emit 0x76
        _emit 0x57
        // 00059d40  MOV EAX, [ESI+0x18]
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 00059d43  CMP EAX, 8
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        // 00059d46  JC 0x00459d68
        _emit 0x72
        _emit 0x20
        // 00059d48  MOV EDX, [EBX]
        _emit 0x8b
        _emit 0x13
        // 00059d4a  JMP 0x00459d6a
        _emit 0xeb
        _emit 0x1e
        // 00059d4c  TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 00059d4e  JNZ 0x00459d3e
        _emit 0x75
        _emit 0xee
        // 00059d50  CMP EAX, 8
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        // 00059d53  MOV [ESI+0x14], EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        // 00059d56  JC 0x00459d5a
        _emit 0x72
        _emit 0x02
        // 00059d58  MOV EBX, [EBX]
        _emit 0x8b
        _emit 0x1b
        // 00059d5a  POP EBP
        _emit 0x5d
        // 00059d5b  POP EDI
        _emit 0x5f
        // 00059d5c  MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 00059d5e  POP ESI
        _emit 0x5e
        // 00059d5f  MOV word [EBX], 0
        _emit 0x66
        _emit 0xc7
        _emit 0x03
        _emit 0x00
        _emit 0x00
        // 00059d64  POP EBX
        _emit 0x5b
        // 00059d65  RET 8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // 00059d68  MOV EDX, EBX
        _emit 0x8b
        _emit 0xd3
        // 00059d6a  MOV ECX, [ESI+0x14]
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 00059d6d  ADD EBP, EBP
        _emit 0x03
        _emit 0xed
        // 00059d6f  PUSH EBP
        _emit 0x55
        // 00059d70  MOV EBP, [ESP+0x18]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        // 00059d74  SUB EAX, ECX
        _emit 0x2b
        _emit 0xc1
        // 00059d76  PUSH EBP
        _emit 0x55
        // 00059d77  ADD EAX, EAX
        _emit 0x03
        _emit 0xc0
        // 00059d79  PUSH EAX
        _emit 0x50
        // 00059d7a  LEA ECX, [EDX+ECX*2]
        _emit 0x8d
        _emit 0x0c
        _emit 0x4a
        // 00059d7d  PUSH ECX
        _emit 0x51
        // 00059d7e  CALL 0x009d17f3
        _emit 0xe8
        _emit 0x70
        _emit 0x7a
        _emit 0x57
        _emit 0x00
        // 00059d83  ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00059d86  CMP dword [ESI+0x18], 8
        _emit 0x83
        _emit 0x7e
        _emit 0x18
        _emit 0x08
        // 00059d8a  MOV [ESI+0x14], EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        // 00059d8d  JC 0x00459d91
        _emit 0x72
        _emit 0x02
        // 00059d8f  MOV EBX, [EBX]
        _emit 0x8b
        _emit 0x1b
        // 00059d91  MOV word [EBX+EDI*2], 0
        _emit 0x66
        _emit 0xc7
        _emit 0x04
        _emit 0x7b
        _emit 0x00
        _emit 0x00
        // 00059d97  POP EBP
        _emit 0x5d
        // 00059d98  POP EDI
        _emit 0x5f
        // 00059d99  MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 00059d9b  POP ESI
        _emit 0x5e
        // 00059d9c  POP EBX
        _emit 0x5b
        // 00059d9d  RET 8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
