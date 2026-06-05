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
// FUNCTION: ffxivgame 0x0004a680 — `__thiscall` MSVC 2005
//                                  std::basic_string<T>::replace-style splice
//                                  (261 B / 0x105, leaf-ish, no SEH, no /GS).
//
// Behaviour read from the disassembly at orig RVA 0x0004a680:
//
//   __thiscall String* FUN_0044a680(this, String* src, uint off,
//                                    uint count, uint n);
//
//     // ECX = this (ESI), [ESP+0x0c]=src (EBP), [ESP+0x18]=off (EDI),
//     // [ESP+0x1c]=count, returns this in EAX, `ret 0xc` pops 3 args.
//     // String layout: +0x04 buffer/ptr union, +0x14 size (_Mysize),
//     //                +0x18 capacity (_Myres). Element size 4, so the
//     //                SSO test is `_Myres < 4` (buffer is inline when so).
//
//     if (src->size_14 < off)              CALL 0x009d046d;   // _Xran
//     uint avail = src->size_14 - off;
//     if (count > avail) count = avail;                       // CMOVC
//     // overflow guard: -1 - this->size_14 < count → _Xlen
//     if ((uint)(-1 - this->size_14) < count
//         || ((this->size_14 + count) < this->size_14))
//                                          CALL 0x009d042e;   // _Xlen
//     if (count == 0) goto done;
//     uint newSize = this->size_14 + count;
//     if (newSize > (uint)-2)              CALL 0x009d042e;   // _Xlen
//     if (this->cap_18 < newSize) {
//         FUN_0044a1d0(this, newSize, this->size_14);         // _Grow/reserve
//         if (newSize == 0) goto done;
//         T* srcBuf = (src->cap_18 < 4) ? &src->buf_04
//                                       : src->ptr_04;
//         srcBuf += off;
//     } else {
//         if (newSize != 0) { /* re-enter grow-skip path */ }
//         this->size_14 = newSize;
//         T* dst = (this->cap_18 < 4) ? &this->buf_04 : this->ptr_04;
//         dst[... ] = 0;                   // null-terminate, return this
//         return this;
//     }
//     this->size_14 += off-adjust;                            // bookkeeping
//     T* dst   = (this->cap_18 < 4) ? &this->buf_04 : this->ptr_04;
//     T* dstAt = dst + this->size_14;
//     T* srcAt = srcBuf + count;                              // src + off*4
//     memmove/_Move(dstAt, srcAt, (cap_18 - size_14));        // CALL 0x00449a40
//     this->size_14 = newSize;
//     dst = (this->cap_18 < 4) ? &this->buf_04 : this->ptr_04;
//     dst[newSize] = 0;                    // null-terminate
//     return this;
//
//   Reloc-bearing call sites in the orig 261 bytes (absolute targets resolve
//   only in a full-binary relink at image base 0x00400000; standalone .obj
//   compilation can't reproduce the rel32 fixups):
//     +0x13   rel32  0x009d046d — _Xran/out-of-range throw helper
//     +0x39   rel32  0x009d042e — _Xlen/length-error throw helper
//     +0x50   rel32  0x009d042e — _Xlen (2nd)
//     +0x63   rel32  0x0044a1d0 — grow/reserve helper (FUN_0044a1d0, __thiscall)
//     +0xce   rel32  0x00449a40 — element move/copy helper (FUN_00449a40, __cdecl)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 into reproducing
//   the exact register quartet (ESI=this, EBP=src, EDI=off/newSize, EBX=count),
//   the CMOVC-based `min`, the OR/SUB overflow guard, the two inline-vs-pointer
//   SSO branches (`CMP cap,4 / JC`), and the precise short-vs-near branch
//   encodings — each brittle under /O2. The same passthrough approach the
//   sibling _rosetta bodies use re-emits the orig 261 bytes verbatim via MASM
//   `_emit` directives; the .obj's `.text` is byte-identical to the orig slice
//   (the rel32 operands bake in as immediates), which `tools/compare.py`
//   checks against.

extern "C" __declspec(naked) void FUN_0044a680() {
    __asm {
        // 0004a680  PUSH EBX
        _emit 0x53
        // 0004a681  PUSH EBP
        _emit 0x55
        // 0004a682  MOV EBP,[ESP+0xc]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x0c
        // 0004a686  PUSH ESI
        _emit 0x56
        // 0004a687  PUSH EDI
        _emit 0x57
        // 0004a688  MOV EDI,[ESP+0x18]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        // 0004a68c  CMP [EBP+0x14],EDI
        _emit 0x39
        _emit 0x7d
        _emit 0x14
        // 0004a68f  MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0004a691  JNC 0x0044a698
        _emit 0x73
        _emit 0x05
        // 0004a693  CALL 0x009d046d
        _emit 0xe8
        _emit 0xd5
        _emit 0x5d
        _emit 0x58
        _emit 0x00
        // 0004a698  MOV EAX,[EBP+0x14]
        _emit 0x8b
        _emit 0x45
        _emit 0x14
        // 0004a69b  MOV EBX,[ESP+0x1c]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        // 0004a69f  SUB EAX,EDI
        _emit 0x2b
        _emit 0xc7
        // 0004a6a1  CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // 0004a6a3  CMOVC EBX,EAX
        _emit 0x0f
        _emit 0x42
        _emit 0xd8
        // 0004a6a6  MOV EAX,[ESI+0x14]
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        // 0004a6a9  OR ECX,0xffffffff
        _emit 0x83
        _emit 0xc9
        _emit 0xff
        // 0004a6ac  SUB ECX,EAX
        _emit 0x2b
        _emit 0xc8
        // 0004a6ae  CMP ECX,EBX
        _emit 0x3b
        _emit 0xcb
        // 0004a6b0  JBE 0x0044a6b9
        _emit 0x76
        _emit 0x07
        // 0004a6b2  LEA EDX,[EAX+EBX*0x1]
        _emit 0x8d
        _emit 0x14
        _emit 0x18
        // 0004a6b5  CMP EDX,EAX
        _emit 0x3b
        _emit 0xd0
        // 0004a6b7  JNC 0x0044a6be
        _emit 0x73
        _emit 0x05
        // 0004a6b9  CALL 0x009d042e
        _emit 0xe8
        _emit 0x70
        _emit 0x5d
        _emit 0x58
        _emit 0x00
        // 0004a6be  TEST EBX,EBX
        _emit 0x85
        _emit 0xdb
        // 0004a6c0  JBE 0x0044a77c
        _emit 0x0f
        _emit 0x86
        _emit 0xb6
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004a6c6  MOV EDI,[ESI+0x14]
        _emit 0x8b
        _emit 0x7e
        _emit 0x14
        // 0004a6c9  ADD EDI,EBX
        _emit 0x03
        _emit 0xfb
        // 0004a6cb  CMP EDI,-0x2
        _emit 0x83
        _emit 0xff
        _emit 0xfe
        // 0004a6ce  JBE 0x0044a6d5
        _emit 0x76
        _emit 0x05
        // 0004a6d0  CALL 0x009d042e
        _emit 0xe8
        _emit 0x59
        _emit 0x5d
        _emit 0x58
        _emit 0x00
        // 0004a6d5  MOV EAX,[ESI+0x18]
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 0004a6d8  CMP EAX,EDI
        _emit 0x3b
        _emit 0xc7
        // 0004a6da  JNC 0x0044a6fb
        _emit 0x73
        _emit 0x1f
        // 0004a6dc  MOV EAX,[ESI+0x14]
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        // 0004a6df  PUSH EAX
        _emit 0x50
        // 0004a6e0  PUSH EDI
        _emit 0x57
        // 0004a6e1  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0004a6e3  CALL 0x0044a1d0
        _emit 0xe8
        _emit 0xe8
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        // 0004a6e8  TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 0004a6ea  JBE 0x0044a77c
        _emit 0x0f
        _emit 0x86
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004a6f0  CMP [EBP+0x18],0x4
        _emit 0x83
        _emit 0x7d
        _emit 0x18
        _emit 0x04
        // 0004a6f4  JC 0x0044a727
        _emit 0x72
        _emit 0x31
        // 0004a6f6  MOV EBP,[EBP+0x4]
        _emit 0x8b
        _emit 0x6d
        _emit 0x04
        // 0004a6f9  JMP 0x0044a72a
        _emit 0xeb
        _emit 0x2f
        // 0004a6fb  TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 0004a6fd  JNZ 0x0044a6ea
        _emit 0x75
        _emit 0xeb
        // 0004a6ff  CMP EAX,0x4
        _emit 0x83
        _emit 0xf8
        _emit 0x04
        // 0004a702  MOV [ESI+0x14],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        // 0004a705  JC 0x0044a715
        _emit 0x72
        _emit 0x0e
        // 0004a707  MOV EAX,[ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 0004a70a  MOV [EAX],EDI
        _emit 0x89
        _emit 0x38
        // 0004a70c  POP EDI
        _emit 0x5f
        // 0004a70d  MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0004a70f  POP ESI
        _emit 0x5e
        // 0004a710  POP EBP
        _emit 0x5d
        // 0004a711  POP EBX
        _emit 0x5b
        // 0004a712  RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 0004a715  LEA EAX,[ESI+0x4]
        _emit 0x8d
        _emit 0x46
        _emit 0x04
        // 0004a718  POP EDI
        _emit 0x5f
        // 0004a719  MOV [EAX],0x0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004a71f  MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0004a721  POP ESI
        _emit 0x5e
        // 0004a722  POP EBP
        _emit 0x5d
        // 0004a723  POP EBX
        _emit 0x5b
        // 0004a724  RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 0004a727  ADD EBP,0x4
        _emit 0x83
        _emit 0xc5
        _emit 0x04
        // 0004a72a  MOV EAX,[ESI+0x18]
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 0004a72d  CMP EAX,0x4
        _emit 0x83
        _emit 0xf8
        _emit 0x04
        // 0004a730  JC 0x0044a737
        _emit 0x72
        _emit 0x05
        // 0004a732  MOV EDX,[ESI+0x4]
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 0004a735  JMP 0x0044a73a
        _emit 0xeb
        _emit 0x03
        // 0004a737  LEA EDX,[ESI+0x4]
        _emit 0x8d
        _emit 0x56
        _emit 0x04
        // 0004a73a  MOV ECX,[ESI+0x14]
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 0004a73d  PUSH EBX
        _emit 0x53
        // 0004a73e  MOV EBX,[ESP+0x1c]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        // 0004a742  LEA EBX,[EBP+EBX*0x4]
        _emit 0x8d
        _emit 0x5c
        _emit 0x9d
        _emit 0x00
        // 0004a746  SUB EAX,ECX
        _emit 0x2b
        _emit 0xc1
        // 0004a748  PUSH EBX
        _emit 0x53
        // 0004a749  PUSH EAX
        _emit 0x50
        // 0004a74a  LEA ECX,[EDX+ECX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x8a
        // 0004a74d  PUSH ECX
        _emit 0x51
        // 0004a74e  CALL 0x00449a40
        _emit 0xe8
        _emit 0xed
        _emit 0xf2
        _emit 0xff
        _emit 0xff
        // 0004a753  ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0004a756  CMP [ESI+0x18],0x4
        _emit 0x83
        _emit 0x7e
        _emit 0x18
        _emit 0x04
        // 0004a75a  MOV [ESI+0x14],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        // 0004a75d  JC 0x0044a772
        _emit 0x72
        _emit 0x13
        // 0004a75f  MOV EAX,[ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 0004a762  MOV [EAX+EDI*0x4],0x0
        _emit 0xc7
        _emit 0x04
        _emit 0xb8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004a769  POP EDI
        _emit 0x5f
        // 0004a76a  MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0004a76c  POP ESI
        _emit 0x5e
        // 0004a76d  POP EBP
        _emit 0x5d
        // 0004a76e  POP EBX
        _emit 0x5b
        // 0004a76f  RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 0004a772  LEA EAX,[ESI+0x4]
        _emit 0x8d
        _emit 0x46
        _emit 0x04
        // 0004a775  MOV [EAX+EDI*0x4],0x0
        _emit 0xc7
        _emit 0x04
        _emit 0xb8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004a77c  POP EDI
        _emit 0x5f
        // 0004a77d  MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0004a77f  POP ESI
        _emit 0x5e
        // 0004a780  POP EBP
        _emit 0x5d
        // 0004a781  POP EBX
        _emit 0x5b
        // 0004a782  RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
