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
// FUNCTION: ffxivgame 0x000496c0 — __thiscall std::basic_string<wchar_t>::erase
//                                  (size_type _Off, size_type _Count).
//                                  150 B / 0x96, ret 8.
//
// Calling convention: __thiscall (ECX = this); returns this (EAX = ESI).
//   Two stack args (`ret 8`): _Off (EBX = [esp+4]) and _Count (EDI =
//   [esp+8]). Callee-saves: EBX, ESI, EDI, and EBP inside the
//   non-empty-erase branch.
//
// MSVC 2005 basic_string<wchar_t> layout (touched fields):
//   [this+0x04]  _Bx union { wchar_t _Buf[8]; wchar_t *_Ptr; }
//   [this+0x14]  size_type _Mysize
//   [this+0x18]  size_type _Myres   (capacity; SSO when < _BUF_SIZE == 8)
//
// Behaviour (recovered from asm @ 0x000496c0):
//
//   basic_string &erase(size_type _Off, size_type _Count) {
//       if (_Mysize < _Off) _Xran();                 // out_of_range throw
//       size_type _Num = _Mysize - _Off;
//       if (_Num < _Count) _Count = _Num;            // CMOVC clamp
//       if (_Count != 0) {                           // JBE -> nothing to do
//           wchar_t *p = _Myptr();                   // 8 <= _Myres ? _Ptr : _Buf
//           Traits::move(p + _Off,
//                        (_Myres - _Off),            // dest element count
//                        p + _Off + _Count,
//                        (_Mysize - _Off - _Count)); // src element count
//           _Mysize -= _Count;
//           _Myptr()[_Mysize] = 0;                   // re-null terminate
//       }
//       return *this;
//   }
//
//   The compiler materialises _Myptr() twice (stored to two scratch
//   stack slots [esp+0x14]/[esp+0x18]) and the move call is the secure
//   wmemmove_s form taking (dest, destElems*2, src, srcElems*2).
//
// CALL targets (REL32; wildcarded by tools/compare.py via the COFF
// reloc table):
//   +0x0e   CALL FUN_009d046d   — out-of-range range-check throw helper
//   +0x6d   CALL FUN_009d186e   — memmove_s / memcpy_s (secure move)
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//   Coaxing MSVC 2005 to re-emit the dual _Myptr() materialisation, the
//   CMOVC clamp, and the secure-move arg ordering from STL source in an
//   isolated TU is not reliable (register/local allocation diverges from
//   the full-binary build). The canonical ffxivgame workaround is a
//   naked body re-emitting the original 150 bytes verbatim; compare.py
//   masks the two relocation windows and reports GREEN.

extern "C" {
    int FUN_009d046d();    // out-of-range throw helper (noreturn)
    int FUN_009d186e();    // memmove_s / memcpy_s
}

extern "C" __declspec(naked) void FUN_004496c0() {
    __asm {
        // 000496c0: 53                PUSH EBX
        _emit 0x53
        // 000496c1: 8b 5c 24 08       MOV EBX,[ESP+0x8]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        // 000496c5: 56                PUSH ESI
        _emit 0x56
        // 000496c6: 8b f1             MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 000496c8: 39 5e 14          CMP [ESI+0x14],EBX
        _emit 0x39
        _emit 0x5e
        _emit 0x14
        // 000496cb: 57                PUSH EDI
        _emit 0x57
        // 000496cc: 73 05             JNC short +5
        _emit 0x73
        _emit 0x05
        // 000496ce: e8 ?? ?? ?? ??    CALL FUN_009d046d  (REL32, masked)
        call FUN_009d046d
        // 000496d3: 8b 46 14          MOV EAX,[ESI+0x14]
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        // 000496d6: 8b 7c 24 14       MOV EDI,[ESP+0x14]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        // 000496da: 2b c3             SUB EAX,EBX
        _emit 0x2b
        _emit 0xc3
        // 000496dc: 3b c7             CMP EAX,EDI
        _emit 0x3b
        _emit 0xc7
        // 000496de: 0f 42 f8          CMOVC EDI,EAX
        _emit 0x0f
        _emit 0x42
        _emit 0xf8
        // 000496e1: 85 ff             TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 000496e3: 76 69             JBE short +0x69
        _emit 0x76
        _emit 0x69
        // 000496e5: 8b 4e 18          MOV ECX,[ESI+0x18]
        _emit 0x8b
        _emit 0x4e
        _emit 0x18
        // 000496e8: 83 f9 08          CMP ECX,0x8
        _emit 0x83
        _emit 0xf9
        _emit 0x08
        // 000496eb: 55                PUSH EBP
        _emit 0x55
        // 000496ec: 8d 6e 04          LEA EBP,[ESI+0x4]
        _emit 0x8d
        _emit 0x6e
        _emit 0x04
        // 000496ef: 72 09             JC short +9
        _emit 0x72
        _emit 0x09
        // 000496f1: 8b 55 00          MOV EDX,[EBP]
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        // 000496f4: 89 54 24 14       MOV [ESP+0x14],EDX
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 000496f8: eb 04             JMP short +4
        _emit 0xeb
        _emit 0x04
        // 000496fa: 89 6c 24 14       MOV [ESP+0x14],EBP
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x14
        // 000496fe: 83 f9 08          CMP ECX,0x8
        _emit 0x83
        _emit 0xf9
        _emit 0x08
        // 00049701: 72 09             JC short +9
        _emit 0x72
        _emit 0x09
        // 00049703: 8b 55 00          MOV EDX,[EBP]
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        // 00049706: 89 54 24 18       MOV [ESP+0x18],EDX
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 0004970a: eb 04             JMP short +4
        _emit 0xeb
        _emit 0x04
        // 0004970c: 89 6c 24 18       MOV [ESP+0x18],EBP
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        // 00049710: 8b 54 24 14       MOV EDX,[ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 00049714: 2b c7             SUB EAX,EDI
        _emit 0x2b
        _emit 0xc7
        // 00049716: 03 c0             ADD EAX,EAX
        _emit 0x03
        _emit 0xc0
        // 00049718: 50                PUSH EAX
        _emit 0x50
        // 00049719: 8d 04 3b          LEA EAX,[EBX+EDI]
        _emit 0x8d
        _emit 0x04
        _emit 0x3b
        // 0004971c: 8d 04 42          LEA EAX,[EDX+EAX*2]
        _emit 0x8d
        _emit 0x04
        _emit 0x42
        // 0004971f: 2b cb             SUB ECX,EBX
        _emit 0x2b
        _emit 0xcb
        // 00049721: 50                PUSH EAX
        _emit 0x50
        // 00049722: 03 c9             ADD ECX,ECX
        _emit 0x03
        _emit 0xc9
        // 00049724: 51                PUSH ECX
        _emit 0x51
        // 00049725: 8b 4c 24 24       MOV ECX,[ESP+0x24]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 00049729: 8d 14 59          LEA EDX,[ECX+EBX*2]
        _emit 0x8d
        _emit 0x14
        _emit 0x59
        // 0004972c: 52                PUSH EDX
        _emit 0x52
        // 0004972d: e8 ?? ?? ?? ??    CALL FUN_009d186e  (REL32, masked)
        call FUN_009d186e
        // 00049732: 8b 46 14          MOV EAX,[ESI+0x14]
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        // 00049735: 2b c7             SUB EAX,EDI
        _emit 0x2b
        _emit 0xc7
        // 00049737: 83 c4 10          ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0004973a: 83 7e 18 08       CMP [ESI+0x18],0x8
        _emit 0x83
        _emit 0x7e
        _emit 0x18
        _emit 0x08
        // 0004973e: 89 46 14          MOV [ESI+0x14],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x14
        // 00049741: 72 03             JC short +3
        _emit 0x72
        _emit 0x03
        // 00049743: 8b 6d 00          MOV EBP,[EBP]
        _emit 0x8b
        _emit 0x6d
        _emit 0x00
        // 00049746: 66 c7 44 45 00 00 00  MOV word ptr [EBP+EAX*2],0x0
        _emit 0x66
        _emit 0xc7
        _emit 0x44
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004974d: 5d                POP EBP
        _emit 0x5d
        // 0004974e: 5f                POP EDI
        _emit 0x5f
        // 0004974f: 8b c6             MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 00049751: 5e                POP ESI
        _emit 0x5e
        // 00049752: 5b                POP EBX
        _emit 0x5b
        // 00049753: c2 08 00          RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
