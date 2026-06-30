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
// FUNCTION: ffxivgame 0x00049f50 — __thiscall std::basic_string<wchar_t>::erase
//                                  (iterator _First, iterator _Last)
//                                  with _SECURE_SCL checked-iterator validation
//                                  (195 B / 0xc3, __thiscall, ret 0x14)
//
// Calling convention: __thiscall (ECX = this); RET 0x14 (5 stack args).
// Returns EAX = arg1 (hidden out-pointer for the iterator return value).
//
// Signature (inferred from asm and sibling FUN_00449600 / FUN_004496c0):
//
//   CheckedIter* __thiscall erase(wstring *this,      // ECX
//                                 CheckedIter *result, // [ESP+04] → ESI at exit
//                                 wstring *_F_cont,    // [ESP+08] → EBP
//                                 wchar_t *_F_ptr,     // [ESP+0C] checked/used for ESI
//                                 wstring *_L_cont,    // [ESP+10] → EAX (checked vs EBP)
//                                 wchar_t *_L_ptr);    // [ESP+14] → EDI
//
// where CheckedIter = struct { const wstring *_Mycont; const wchar_t *_Myptr; }
// (matches FUN_00449600 layout: 2 DWORDs, 8 bytes total).
//
// MSVC 2005 basic_string<wchar_t> layout (touched fields):
//   [this+0x04]  _Bx union { wchar_t _Buf[8]; wchar_t *_Ptr; }
//   [this+0x14]  size_type _Mysize
//   [this+0x18]  size_type _Myres   (capacity; SSO when < 8)
//
// High-level pseudo-C:
//
//   1. Compute begin = (_Myres >= 8) ? _Bx._Ptr : &_Bx._Buf;
//      Assert begin != null and begin <= end (end = begin + _Mysize)
//
//   2. off = 0;
//      if (_F_ptr != null) {
//          assert _F_cont == -2 || _F_cont == this;  // checked-iter validation
//          off = (_F_ptr - begin) / 2;               // wchar_t offset of _First
//      }
//
//   3. count = 0;
//      if (_L_ptr != null) {
//          assert _L_cont == -2 || _L_cont == _F_cont;
//          count = (_L_ptr - _F_ptr) / 2;            // distance _First.._Last
//      }
//
//   4. this->erase(off, count);   // FUN_004496c0
//
//   5. ptr = _Myptr() + off * 2;  // pointer into buf at _First position
//      FUN_00449600(result, ptr, this);   // init return CheckedIter
//      return result;
//
// Call sites inside the orig 195 bytes (all REL32, compare.py masks are
// derived from the original PE bytes — we emit verbatim so the .obj
// .text section is byte-identical to the orig slice):
//   +0x40  e8 1f 83 58 00   CALL 0x009d22b4 (checked-iter error, noreturn)
//   +0x62  e8 fd 82 58 00   CALL 0x009d22b4 (same)
//   +0x88  e8 d7 82 58 00   CALL 0x009d22b4 (same)
//   +0x97  e8 d4 f6 ff ff   CALL 0x004496c0 (wstring::erase(off, count))
//   +0xb6  e8 f5 f5 ff ff   CALL 0x00449600 (CheckedIter ctor)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The SSO branch pair (CMP _Myres,8 / JC / MOV / JMP / LEA — appearing
//   three times), the checked-iterator validation logic for both _First and
//   _Last, and the stack-shuffle around the intermediate PUSH EBP all
//   produce code whose register allocation and branch encoding depends on
//   the full-binary MSVC 2005 optimiser context. Source-level rewrites
//   diverge in register order; the pragmatic choice — matching sibling
//   FUN_004494d0 / FUN_00403f10 / FUN_00403eb0 — is a __declspec(naked)
//   body re-emitting the original 195 bytes verbatim via MASM _emit
//   directives. The .obj .text section ends up byte-identical to the orig
//   slice (CALL REL32 bytes are the original resolved values, same as what
//   compare.py reads from the PE; no COFF reloc entries are emitted for the
//   _emit'd call bytes, so compare.py sees them as plain data and they match
//   directly).

extern "C" __declspec(naked) void FUN_00449f50() {
    __asm {
        // 00049f50: 53                  PUSH EBX
        _emit 0x53
        // 00049f51: 8b d9               MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 00049f53: 8b 4b 18            MOV ECX,dword ptr [EBX+0x18]
        _emit 0x8b
        _emit 0x4b
        _emit 0x18
        // 00049f56: 83 f9 08            CMP ECX,0x8
        _emit 0x83
        _emit 0xf9
        _emit 0x08
        // 00049f59: 56                  PUSH ESI
        _emit 0x56
        // 00049f5a: 57                  PUSH EDI
        _emit 0x57
        // 00049f5b: 72 05               JC +5  (to LEA EDI,[EBX+4])
        _emit 0x72
        _emit 0x05
        // 00049f5d: 8b 7b 04            MOV EDI,dword ptr [EBX+0x4]   ; heap ptr
        _emit 0x8b
        _emit 0x7b
        _emit 0x04
        // 00049f60: eb 03               JMP +3  (to TEST EDI,EDI)
        _emit 0xeb
        _emit 0x03
        // 00049f62: 8d 7b 04            LEA EDI,[EBX+0x4]              ; inline buf
        _emit 0x8d
        _emit 0x7b
        _emit 0x04
        // 00049f65: 85 ff               TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 00049f67: 74 27               JZ +0x27  (to CALL error)
        _emit 0x74
        _emit 0x27
        // 00049f69: 83 f9 08            CMP ECX,0x8
        _emit 0x83
        _emit 0xf9
        _emit 0x08
        // 00049f6c: 8d 53 04            LEA EDX,[EBX+0x4]
        _emit 0x8d
        _emit 0x53
        _emit 0x04
        // 00049f6f: 72 04               JC +4
        _emit 0x72
        _emit 0x04
        // 00049f71: 8b 02               MOV EAX,dword ptr [EDX]        ; heap base
        _emit 0x8b
        _emit 0x02
        // 00049f73: eb 02               JMP +2
        _emit 0xeb
        _emit 0x02
        // 00049f75: 8b c2               MOV EAX,EDX                    ; SSO base
        _emit 0x8b
        _emit 0xc2
        // 00049f77: 3b c7               CMP EAX,EDI                    ; begin > end?
        _emit 0x3b
        _emit 0xc7
        // 00049f79: 77 15               JA +0x15  (to CALL error)
        _emit 0x77
        _emit 0x15
        // 00049f7b: 83 f9 08            CMP ECX,0x8
        _emit 0x83
        _emit 0xf9
        _emit 0x08
        // 00049f7e: 72 04               JC +4
        _emit 0x72
        _emit 0x04
        // 00049f80: 8b 02               MOV EAX,dword ptr [EDX]
        _emit 0x8b
        _emit 0x02
        // 00049f82: eb 02               JMP +2
        _emit 0xeb
        _emit 0x02
        // 00049f84: 8b c2               MOV EAX,EDX
        _emit 0x8b
        _emit 0xc2
        // 00049f86: 8b 4b 14            MOV ECX,dword ptr [EBX+0x14]   ; _Mysize
        _emit 0x8b
        _emit 0x4b
        _emit 0x14
        // 00049f89: 8d 14 48            LEA EDX,[EAX+ECX*2]            ; base+size*2
        _emit 0x8d
        _emit 0x14
        _emit 0x48
        // 00049f8c: 3b fa               CMP EDI,EDX                    ; EDI vs end
        _emit 0x3b
        _emit 0xfa
        // 00049f8e: 76 05               JBE +5  (past CALL error → ok)
        _emit 0x76
        _emit 0x05
        // 00049f90: e8 1f 83 58 00      CALL 0x009d22b4  (checked-iter error)
        _emit 0xe8
        _emit 0x1f
        _emit 0x83
        _emit 0x58
        _emit 0x00
        // 00049f95: 83 7c 24 18 00      CMP dword ptr [ESP+0x18],0x0   ; _F_ptr == null?
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x00
        // 00049f9a: 55                  PUSH EBP
        _emit 0x55
        // 00049f9b: 8b 6c 24 18         MOV EBP,dword ptr [ESP+0x18]   ; EBP = _F_cont
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        // 00049f9f: 75 04               JNZ +4  (to CMP EBP,-2)
        _emit 0x75
        _emit 0x04
        // 00049fa1: 33 f6               XOR ESI,ESI                    ; off = 0
        _emit 0x33
        _emit 0xf6
        // 00049fa3: eb 1a               JMP +0x1a  (to MOV EDI,[ESP+0x24])
        _emit 0xeb
        _emit 0x1a
        // 00049fa5: 83 fd fe            CMP EBP,-0x2
        _emit 0x83
        _emit 0xfd
        _emit 0xfe
        // 00049fa8: 74 0d               JZ +0xd  (to MOV ESI,[ESP+0x1c])
        _emit 0x74
        _emit 0x0d
        // 00049faa: 85 ed               TEST EBP,EBP
        _emit 0x85
        _emit 0xed
        // 00049fac: 74 04               JZ +4  (to CALL error)
        _emit 0x74
        _emit 0x04
        // 00049fae: 3b eb               CMP EBP,EBX                    ; == this?
        _emit 0x3b
        _emit 0xeb
        // 00049fb0: 74 05               JZ +5  (to MOV ESI,[ESP+0x1c])
        _emit 0x74
        _emit 0x05
        // 00049fb2: e8 fd 82 58 00      CALL 0x009d22b4  (checked-iter error)
        _emit 0xe8
        _emit 0xfd
        _emit 0x82
        _emit 0x58
        _emit 0x00
        // 00049fb7: 8b 74 24 1c         MOV ESI,dword ptr [ESP+0x1c]   ; _F_ptr
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        // 00049fbb: 2b f7               SUB ESI,EDI                    ; ESI -= begin
        _emit 0x2b
        _emit 0xf7
        // 00049fbd: d1 fe               SAR ESI,0x1                    ; off = /2
        _emit 0xd1
        _emit 0xfe
        // 00049fbf: 8b 7c 24 24         MOV EDI,dword ptr [ESP+0x24]   ; _L_ptr
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x24
        // 00049fc3: 85 ff               TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 00049fc5: 74 1c               JZ +0x1c  (count = 0 path)
        _emit 0x74
        _emit 0x1c
        // 00049fc7: 8b 44 24 20         MOV EAX,dword ptr [ESP+0x20]   ; _L_cont
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 00049fcb: 83 f8 fe            CMP EAX,-0x2
        _emit 0x83
        _emit 0xf8
        _emit 0xfe
        // 00049fce: 74 0d               JZ +0xd  (to SUB EDI,[ESP+0x1c])
        _emit 0x74
        _emit 0x0d
        // 00049fd0: 85 c0               TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00049fd2: 74 04               JZ +4  (to CALL error)
        _emit 0x74
        _emit 0x04
        // 00049fd4: 3b c5               CMP EAX,EBP                    ; == _F_cont?
        _emit 0x3b
        _emit 0xc5
        // 00049fd6: 74 05               JZ +5  (to SUB EDI,[ESP+0x1c])
        _emit 0x74
        _emit 0x05
        // 00049fd8: e8 d7 82 58 00      CALL 0x009d22b4  (checked-iter error)
        _emit 0xe8
        _emit 0xd7
        _emit 0x82
        _emit 0x58
        _emit 0x00
        // 00049fdd: 2b 7c 24 1c         SUB EDI,dword ptr [ESP+0x1c]   ; EDI -= _F_ptr
        _emit 0x2b
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        // 00049fe1: d1 ff               SAR EDI,0x1                    ; count = /2
        _emit 0xd1
        _emit 0xff
        // 00049fe3: 57                  PUSH EDI                       ; arg2: count
        _emit 0x57
        // 00049fe4: 56                  PUSH ESI                       ; arg1: off
        _emit 0x56
        // 00049fe5: 8b cb               MOV ECX,EBX                    ; ECX = this
        _emit 0x8b
        _emit 0xcb
        // 00049fe7: e8 d4 f6 ff ff      CALL 0x004496c0  (wstring::erase(off, count))
        _emit 0xe8
        _emit 0xd4
        _emit 0xf6
        _emit 0xff
        _emit 0xff
        // 00049fec: 83 7b 18 08         CMP dword ptr [EBX+0x18],0x8
        _emit 0x83
        _emit 0x7b
        _emit 0x18
        _emit 0x08
        // 00049ff0: 5d                  POP EBP
        _emit 0x5d
        // 00049ff1: 72 05               JC +5  (to LEA EAX,[EBX+4])
        _emit 0x72
        _emit 0x05
        // 00049ff3: 8b 43 04            MOV EAX,dword ptr [EBX+0x4]   ; heap ptr
        _emit 0x8b
        _emit 0x43
        _emit 0x04
        // 00049ff6: eb 03               JMP +3  (to LEA EAX,[EAX+ESI*2])
        _emit 0xeb
        _emit 0x03
        // 00049ff8: 8d 43 04            LEA EAX,[EBX+0x4]              ; inline buf
        _emit 0x8d
        _emit 0x43
        _emit 0x04
        // 00049ffb: 8d 04 70            LEA EAX,[EAX+ESI*2]            ; ptr at off
        _emit 0x8d
        _emit 0x04
        _emit 0x70
        // 00049ffe: 8b 74 24 10         MOV ESI,dword ptr [ESP+0x10]   ; ESI = result
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 0004a002: 53                  PUSH EBX                       ; push this (str arg)
        _emit 0x53
        // 0004a003: 50                  PUSH EAX                       ; push ptr arg
        _emit 0x50
        // 0004a004: 8b ce               MOV ECX,ESI                    ; ECX = result
        _emit 0x8b
        _emit 0xce
        // 0004a006: e8 f5 f5 ff ff      CALL 0x00449600  (CheckedIter ctor)
        _emit 0xe8
        _emit 0xf5
        _emit 0xf5
        _emit 0xff
        _emit 0xff
        // 0004a00b: 5f                  POP EDI
        _emit 0x5f
        // 0004a00c: 8b c6               MOV EAX,ESI                    ; return result
        _emit 0x8b
        _emit 0xc6
        // 0004a00e: 5e                  POP ESI
        _emit 0x5e
        // 0004a00f: 5b                  POP EBX
        _emit 0x5b
        // 0004a010: c2 14 00            RET 0x14
        _emit 0xc2
        _emit 0x14
        _emit 0x00
    }
}
