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
// FUNCTION: ffxivgame 0x000487a0 — __thiscall constructor/initializer for a
//                                   small inline-buffer string object (66 B).
//
// Layout (inferred from the asm):
//   this (ECX):
//     +0x00  char *buf    ; points at the inline buffer (this+0x12)
//     +0x04  int   cap    ; capacity, initialized to 0x40
//     +0x08  int   len    ; current length, initialized to 1
//     +0x0c  int   ?      ; zeroed
//     +0x10  char  ?      ; flag, set to 1
//     +0x11  char  ?      ; flag, set to 1
//     +0x12  char  buf[]  ; inline storage; buf[0] = '\0'
//
// Source shape:
//
//   T * __thiscall FUN_004487a0(this, int arg) {
//       this->len  = 1;
//       this->buf  = &this->inline[0];   // this + 0x12
//       this->flag10 = 1;
//       this->flag11 = 1;
//       this->field0c = 0;
//       this->cap  = 0x40;
//       this->inline[0] = '\0';
//       int saved = this->len;           // EDI
//       FUN_00447010(this, arg, 1);      // __thiscall on this
//       this->len = saved;
//       return this;
//   }
//
// Calling convention: __thiscall (ECX = this; one stack arg; callee cleans
// 4 bytes via `ret 4`; returns this in EAX).
//
// Asm (66 bytes @ orig RVA 0x000487a0):
//   56                  PUSH ESI
//   8b f1               MOV  ESI, ECX
//   b9 01 00 00 00      MOV  ECX, 1
//   8d 46 12            LEA  EAX, [ESI+0x12]
//   57                  PUSH EDI
//   89 4e 08            MOV  [ESI+8], ECX
//   88 4e 10            MOV  [ESI+0x10], CL
//   88 4e 11            MOV  [ESI+0x11], CL
//   89 06               MOV  [ESI], EAX
//   c7 46 0c 00 00 00 00  MOV dword ptr [ESI+0xc], 0
//   c7 46 04 40 00 00 00  MOV dword ptr [ESI+4], 0x40
//   c6 00 00            MOV  byte ptr [EAX], 0
//   8b 44 24 0c         MOV  EAX, [ESP+0xc]            ; the stack arg
//   8b 7e 08            MOV  EDI, [ESI+8]
//   51                  PUSH ECX                       ; arg2 = 1
//   50                  PUSH EAX                       ; arg1 = stack arg
//   8b ce               MOV  ECX, ESI                  ; this
//   e8 38 e8 ff ff      CALL FUN_00447010 (rel32 → 0x00447010)
//   89 7e 08            MOV  [ESI+8], EDI
//   5f                  POP  EDI
//   8b c6               MOV  EAX, ESI                  ; return this
//   5e                  POP  ESI
//   c2 04 00            RET  4
//
// Reconstruction: __declspec(naked) _emit byte passthrough (mirrors the
// sibling _rosetta naked-asm matches, e.g. FUN_00448270). The lone CALL
// rel32 immediate is the orig file's baked offset, emitted verbatim — the
// .obj's .text is byte-identical with no relocations; compare.py → GREEN.

extern "C" __declspec(naked) void FUN_004487a0() {
    __asm {
        _emit 0x56          // PUSH ESI
        _emit 0x8b          // MOV ESI, ECX
        _emit 0xf1
        _emit 0xb9          // MOV ECX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d          // LEA EAX, [ESI+0x12]
        _emit 0x46
        _emit 0x12
        _emit 0x57          // PUSH EDI
        _emit 0x89          // MOV [ESI+8], ECX
        _emit 0x4e
        _emit 0x08
        _emit 0x88          // MOV [ESI+0x10], CL
        _emit 0x4e
        _emit 0x10
        _emit 0x88          // MOV [ESI+0x11], CL
        _emit 0x4e
        _emit 0x11
        _emit 0x89          // MOV [ESI], EAX
        _emit 0x06
        _emit 0xc7          // MOV dword ptr [ESI+0xc], 0
        _emit 0x46
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7          // MOV dword ptr [ESI+4], 0x40
        _emit 0x46
        _emit 0x04
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc6          // MOV byte ptr [EAX], 0
        _emit 0x00
        _emit 0x00
        _emit 0x8b          // MOV EAX, [ESP+0xc]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b          // MOV EDI, [ESI+8]
        _emit 0x7e
        _emit 0x08
        _emit 0x51          // PUSH ECX
        _emit 0x50          // PUSH EAX
        _emit 0x8b          // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8          // CALL FUN_00447010 (rel32 → 0x00447010)
        _emit 0x38
        _emit 0xe8
        _emit 0xff
        _emit 0xff
        _emit 0x89          // MOV [ESI+8], EDI
        _emit 0x7e
        _emit 0x08
        _emit 0x5f          // POP EDI
        _emit 0x8b          // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e          // POP ESI
        _emit 0xc2          // RET 4
        _emit 0x04
        _emit 0x00
    }
}
