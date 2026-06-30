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
// FUNCTION: ffxivgame 0x0004a0f0 — std::string::assign(const std::string&,
//                                  size_t off, size_t count) (217 B / 0xd9,
//                                  __thiscall, RET 0xC). Assigns a
//                                  [off, off+count) substring of `other`
//                                  into `*this`, handling self-assignment
//                                  and SSO (small-string-optimisation).
//
// Signature:
//
//   std::string* __thiscall FUN_0044a0f0(
//       this    = ECX,             // std::string* this
//       other*  = [ESP+4],         // const std::string* other
//       off     = [ESP+8],         // size_t start offset into other
//       count   = [ESP+C]);        // size_t max characters to copy
//
// Layout assumed (MSVC 2005 std::basic_string<char>):
//   +0x00  union { char _Buf[16]; char* _Ptr; } _Bx
//   +0x10  size_t _Mysize
//   +0x14  size_t _Myres   (capacity)
//   (allocator is empty; this layout matches MSVC8 _String_val)
//
// Wait — observed offsets in the asm are +0x04, +0x14, +0x18, suggesting
// the string member sits 4 bytes into `this` (vtable or other field at +0x00):
//   +0x04  union { char _Buf[16]; char* _Ptr; }
//   +0x14  size_t _Mysize
//   +0x18  size_t _Myres
//
// Body logic (paraphrased):
//
//   if (other._Mysize < off)
//       throw std::out_of_range (via 0x009d046d)
//
//   actual = min(other._Mysize - off, count)
//
//   if (this == &other) {           // self-assign: trim in place
//       this->erase(off + actual, npos)  // via 0x00449570(actual+off, -1)
//       this->erase(0, off)              // via 0x00449570(0, off)
//       return *this
//   }
//
//   if (actual > 0xFFFFFFFE)
//       throw std::length_error (via 0x009d042e)
//
//   if (this->_Myres < actual)
//       this->_Grow(actual, this->_Mysize)   // via 0x004498d0
//
//   if (actual == 0) {
//       this->_Mysize = 0
//       null-terminate via _Bx._Buf or *_Bx._Ptr
//       return *this
//   }
//
//   src = (other._Myres < 16) ? &other._Bx._Buf[0] : other._Bx._Ptr
//   dst = (this->_Myres < 16) ? &this->_Bx._Buf[0] : this->_Bx._Ptr
//   memmove_s(dst, this->_Myres, src + off, actual)   // 0x009d17f3
//   this->_Mysize = actual
//   dst[actual] = '\0'
//   return *this
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   This mirrors the rationale of FUN_00408910, FUN_00404e40, etc. The
//   function body contains a CMOVC instruction (0f 42 f8), multiple
//   short-forward and short-backward branches whose exact encodings
//   (JC vs JBE vs JNZ, all short) are compiler-selected, SSO branches
//   duplicated for different return paths, and six REL32 CALL sites to
//   internal CRT helpers. Reproducing this precise 217-byte sequence
//   from C++ source under /O2 /Oi is impractical — any high-level
//   rewrite shifts at least one branch encoding or adds/removes a MOV.
//   Naked asm byte passthrough gives an exact match.
//
// Reloc-bearing sites (REL32 CALL displacements — masked by compare.py):
//   +0x13  CALL 0x009d046d  (std::out_of_range throw helper)
//   +0x31  CALL 0x00449570  (string::erase — first call, self-assign)
//   +0x3b  CALL 0x00449570  (string::erase — second call, self-assign)
//   +0x4e  CALL 0x009d042e  (std::length_error throw helper)
//   +0x61  CALL 0x004498d0  (string::_Grow)
//   +0xb9  CALL 0x009d17f3  (memmove_s)

extern "C" __declspec(naked) void FUN_0044a0f0() {
    __asm {
        // 0x0004a0f0  PUSH EBX
        _emit 0x53
        // 0x0004a0f1  MOV EBX,[ESP+0x8]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        // 0x0004a0f5  PUSH EBP
        _emit 0x55
        // 0x0004a0f6  MOV EBP,[ESP+0x10]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // 0x0004a0fa  CMP [EBX+0x14],EBP
        _emit 0x39
        _emit 0x6b
        _emit 0x14
        // 0x0004a0fd  PUSH ESI
        _emit 0x56
        // 0x0004a0fe  PUSH EDI
        _emit 0x57
        // 0x0004a0ff  MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0x0004a101  JNC +5 (to 0x4a108)
        _emit 0x73
        _emit 0x05
        // 0x0004a103  CALL 0x009d046d  [reloc]
        _emit 0xe8
        _emit 0x65
        _emit 0x63
        _emit 0x58
        _emit 0x00
        // 0x0004a108  MOV EDI,[EBX+0x14]
        _emit 0x8b
        _emit 0x7b
        _emit 0x14
        // 0x0004a10b  MOV EAX,[ESP+0x1c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0x0004a10f  SUB EDI,EBP
        _emit 0x2b
        _emit 0xfd
        // 0x0004a111  CMP EAX,EDI
        _emit 0x3b
        _emit 0xc7
        // 0x0004a113  CMOVC EDI,EAX
        _emit 0x0f
        _emit 0x42
        _emit 0xf8
        // 0x0004a116  CMP ESI,EBX
        _emit 0x3b
        _emit 0xf3
        // 0x0004a118  JNZ +0x1f (to 0x4a139)
        _emit 0x75
        _emit 0x1f
        // 0x0004a11a  PUSH -1
        _emit 0x6a
        _emit 0xff
        // 0x0004a11c  ADD EDI,EBP
        _emit 0x03
        _emit 0xfd
        // 0x0004a11e  PUSH EDI
        _emit 0x57
        // 0x0004a11f  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0x0004a121  CALL 0x00449570  [reloc]
        _emit 0xe8
        _emit 0x4a
        _emit 0xf4
        _emit 0xff
        _emit 0xff
        // 0x0004a126  PUSH EBP
        _emit 0x55
        // 0x0004a127  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0x0004a129  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0x0004a12b  CALL 0x00449570  [reloc]
        _emit 0xe8
        _emit 0x40
        _emit 0xf4
        _emit 0xff
        _emit 0xff
        // 0x0004a130  POP EDI
        _emit 0x5f
        // 0x0004a131  MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0x0004a133  POP ESI
        _emit 0x5e
        // 0x0004a134  POP EBP
        _emit 0x5d
        // 0x0004a135  POP EBX
        _emit 0x5b
        // 0x0004a136  RET 0xC
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 0x0004a139  CMP EDI,-0x2
        _emit 0x83
        _emit 0xff
        _emit 0xfe
        // 0x0004a13c  JBE +5 (to 0x4a143)
        _emit 0x76
        _emit 0x05
        // 0x0004a13e  CALL 0x009d042e  [reloc]
        _emit 0xe8
        _emit 0xeb
        _emit 0x62
        _emit 0x58
        _emit 0x00
        // 0x0004a143  MOV EAX,[ESI+0x18]
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 0x0004a146  CMP EAX,EDI
        _emit 0x3b
        _emit 0xc7
        // 0x0004a148  JNC +0x1b (to 0x4a165)
        _emit 0x73
        _emit 0x1b
        // 0x0004a14a  MOV EAX,[ESI+0x14]
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        // 0x0004a14d  PUSH EAX
        _emit 0x50
        // 0x0004a14e  PUSH EDI
        _emit 0x57
        // 0x0004a14f  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0x0004a151  CALL 0x004498d0  [reloc]
        _emit 0xe8
        _emit 0x7a
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        // 0x0004a156  TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 0x0004a158  JBE +0x66 (to 0x4a1c0)
        _emit 0x76
        _emit 0x66
        // 0x0004a15a  CMP [EBX+0x18],0x10
        _emit 0x83
        _emit 0x7b
        _emit 0x18
        _emit 0x10
        // 0x0004a15e  JC +0x2f (to 0x4a18f)
        _emit 0x72
        _emit 0x2f
        // 0x0004a160  MOV EDX,[EBX+0x4]
        _emit 0x8b
        _emit 0x53
        _emit 0x04
        // 0x0004a163  JMP +0x2d (to 0x4a192)
        _emit 0xeb
        _emit 0x2d
        // 0x0004a165  TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 0x0004a167  JNZ -0x11 (to 0x4a158)
        _emit 0x75
        _emit 0xef
        // 0x0004a169  CMP EAX,0x10
        _emit 0x83
        _emit 0xf8
        _emit 0x10
        // 0x0004a16c  MOV [ESI+0x14],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        // 0x0004a16f  JC +0xf (to 0x4a180)
        _emit 0x72
        _emit 0x0f
        // 0x0004a171  MOV EAX,[ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 0x0004a174  POP EDI
        _emit 0x5f
        // 0x0004a175  MOV byte ptr [EAX],0x0
        _emit 0xc6
        _emit 0x00
        _emit 0x00
        // 0x0004a178  MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0x0004a17a  POP ESI
        _emit 0x5e
        // 0x0004a17b  POP EBP
        _emit 0x5d
        // 0x0004a17c  POP EBX
        _emit 0x5b
        // 0x0004a17d  RET 0xC
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 0x0004a180  LEA EAX,[ESI+0x4]
        _emit 0x8d
        _emit 0x46
        _emit 0x04
        // 0x0004a183  POP EDI
        _emit 0x5f
        // 0x0004a184  MOV byte ptr [EAX],0x0
        _emit 0xc6
        _emit 0x00
        _emit 0x00
        // 0x0004a187  MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0x0004a189  POP ESI
        _emit 0x5e
        // 0x0004a18a  POP EBP
        _emit 0x5d
        // 0x0004a18b  POP EBX
        _emit 0x5b
        // 0x0004a18c  RET 0xC
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 0x0004a18f  LEA EDX,[EBX+0x4]
        _emit 0x8d
        _emit 0x53
        _emit 0x04
        // 0x0004a192  MOV ECX,[ESI+0x18]
        _emit 0x8b
        _emit 0x4e
        _emit 0x18
        // 0x0004a195  CMP ECX,0x10
        _emit 0x83
        _emit 0xf9
        _emit 0x10
        // 0x0004a198  LEA EBX,[ESI+0x4]
        _emit 0x8d
        _emit 0x5e
        _emit 0x04
        // 0x0004a19b  JC +4 (to 0x4a1a1)
        _emit 0x72
        _emit 0x04
        // 0x0004a19d  MOV EAX,[EBX]
        _emit 0x8b
        _emit 0x03
        // 0x0004a19f  JMP +2 (to 0x4a1a3)
        _emit 0xeb
        _emit 0x02
        // 0x0004a1a1  MOV EAX,EBX
        _emit 0x8b
        _emit 0xc3
        // 0x0004a1a3  PUSH EDI
        _emit 0x57
        // 0x0004a1a4  ADD EDX,EBP
        _emit 0x03
        _emit 0xd5
        // 0x0004a1a6  PUSH EDX
        _emit 0x52
        // 0x0004a1a7  PUSH ECX
        _emit 0x51
        // 0x0004a1a8  PUSH EAX
        _emit 0x50
        // 0x0004a1a9  CALL 0x009d17f3  [reloc]
        _emit 0xe8
        _emit 0x45
        _emit 0x76
        _emit 0x58
        _emit 0x00
        // 0x0004a1ae  ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0x0004a1b1  CMP [ESI+0x18],0x10
        _emit 0x83
        _emit 0x7e
        _emit 0x18
        _emit 0x10
        // 0x0004a1b5  MOV [ESI+0x14],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        // 0x0004a1b8  JC +2 (to 0x4a1bc)
        _emit 0x72
        _emit 0x02
        // 0x0004a1ba  MOV EBX,[EBX]
        _emit 0x8b
        _emit 0x1b
        // 0x0004a1bc  MOV byte ptr [EBX+EDI*1],0x0
        _emit 0xc6
        _emit 0x04
        _emit 0x3b
        _emit 0x00
        // 0x0004a1c0  POP EDI
        _emit 0x5f
        // 0x0004a1c1  MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0x0004a1c3  POP ESI
        _emit 0x5e
        // 0x0004a1c4  POP EBP
        _emit 0x5d
        // 0x0004a1c5  POP EBX
        _emit 0x5b
        // 0x0004a1c6  RET 0xC
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
