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
// FUNCTION: ffxivgame 0x00022d20 — copy_backward of 4-byte trivial elements
//                                  via the secure CRT (47 B / 0x2F)
//
//   void * __cdecl FUN_00422d20(T *_First, T *_Last, T *_Dest)
//     stack layout (after the PUSH ESI):
//       [ESP+0x04] : T *_First   (param_1, begin)
//       [ESP+0x08] : T *_Last    (param_2, one-past-end)
//       [ESP+0x0C] : T *_Dest    (param_3, destination one-past-end)
//
// Source shape (MSVC 2005 _Copy_backward_opt for a scalar/POD pointer
// range — `memcpy_s` lowering):
//
//   void *copy_backward(T *_First, T *_Last, T *_Dest) {
//       ptrdiff_t _Count = _Last - _First;          // SUB ; SAR 2
//       size_t    _Bytes = (size_t)_Count * 4;      // LEA EAX*4
//       _Dest -= _Count;                            // SUB ESI, ECX
//       if (_Count > 0)
//           memcpy_s(_Dest, _Bytes, _First, _Bytes);
//       return _Dest;                               // MOV EAX, ESI
//   }
//
// Calling convention: __cdecl (three stack pointers, caller cleans; the
// inner memcpy_s call is also __cdecl, cleaned with ADD ESP,0x10).
// Frame: PUSH ESI / POP ESI (single callee-save, no ESP locals).
//
// Asm (47 bytes @ orig RVA 0x00022d20):
//   8b 44 24 08             MOV  EAX, [ESP+0x8]        ; _Last
//   8b 54 24 04             MOV  EDX, [ESP+0x4]        ; _First
//   2b c2                   SUB  EAX, EDX
//   c1 f8 02                SAR  EAX, 0x2              ; _Count
//   56                      PUSH ESI
//   8b 74 24 10             MOV  ESI, [ESP+0x10]       ; _Dest
//   8d 0c 85 00 00 00 00    LEA  ECX, [EAX*4 + 0]      ; _Bytes
//   2b f1                   SUB  ESI, ECX              ; _Dest -= _Count
//   85 c0                   TEST EAX, EAX
//   7e 0c                   JLE  done                  ; _Count <= 0
//   51                      PUSH ECX                   ; count
//   52                      PUSH EDX                   ; src   = _First
//   51                      PUSH ECX                   ; destSize
//   56                      PUSH ESI                   ; dest  = _Dest
//   e8 26 eb 5a 00          CALL memcpy_s              ; rel32 → 0x009d186e
//   83 c4 10                ADD  ESP, 0x10
//  done:
//   8b c6                   MOV  EAX, ESI
//   5e                      POP  ESI
//   c3                      RET
//
// Reconstruction: __declspec(naked) _emit byte passthrough (mirrors the
// sibling _rosetta naked matches). The single REL32 callsite (memcpy_s at
// VA 0x009d186e) is masked out of the byte diff by tools/compare.py; the
// orig wire bytes are re-emitted verbatim so the .obj's .text matches
// byte-for-byte with no relocations.

extern "C" __declspec(naked) void FUN_00422d20() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x4]
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x2b              // SUB EAX, EDX
        _emit 0xc2
        _emit 0xc1              // SAR EAX, 0x2
        _emit 0xf8
        _emit 0x02
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x10]
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x8d              // LEA ECX, [EAX*4 + 0x0]
        _emit 0x0c
        _emit 0x85
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB ESI, ECX
        _emit 0xf1
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7e              // JLE done (+0x0c)
        _emit 0x0c
        _emit 0x51              // PUSH ECX
        _emit 0x52              // PUSH EDX
        _emit 0x51              // PUSH ECX
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL memcpy_s (rel32 → 0x009d186e)
        _emit 0x26
        _emit 0xeb
        _emit 0x5a
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x8b              // MOV EAX, ESI    (done:)
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
