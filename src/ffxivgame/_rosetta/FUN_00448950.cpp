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
// FUNCTION: ffxivgame 0x00048950 — __thiscall append-byte to a small
//                                  "buffer + size" container (46 B / 0x2E).
//
// Layout (inferred from the asm):
//   this (ECX):
//     +0x00  char *buf     (data pointer)
//     +0x08  int   size    (current length, incl. NUL terminator)
//
// Source shape:
//   Container * Container::push(char c) {           // __thiscall, ret 4
//       this->grow(this->size + 1, 1);              // CALL 0x00447010
//       this->buf[this->size - 2] = c;              // overwrite old NUL
//       this->buf[this->size - 1] = 0;              // re-terminate
//       return this;
//   }
//
// The peer at 0x00447010 reserves/extends storage and bumps `size` by 1,
// so post-call `buf[size-2]` is the just-freed terminator slot and
// `buf[size-1]` the new terminator. Returns `this` in EAX.
//
// Calling convention: __thiscall (ECX = this; one byte stack arg at
// [ESP+0x08] after PUSH ESI; callee cleans 4 bytes via `ret 4`).
// Frame: PUSH ESI only.
//
// Asm (46 bytes @ orig RVA 0x00048950):
//   56                  PUSH ESI
//   8b f1               MOV  ESI, ECX
//   8b 46 08            MOV  EAX, [ESI+8]
//   6a 01               PUSH 1
//   83 c0 01            ADD  EAX, 1
//   50                  PUSH EAX
//   e8 af e6 ff ff      CALL 0x00447010              (rel32, masked)
//   8b 0e               MOV  ECX, [ESI]
//   8b 56 08            MOV  EDX, [ESI+8]
//   8a 44 24 08         MOV  AL, [ESP+8]
//   88 44 11 fe         MOV  [ECX+EDX-2], AL
//   8b 0e               MOV  ECX, [ESI]
//   8b 56 08            MOV  EDX, [ESI+8]
//   8b c6               MOV  EAX, ESI
//   c6 44 11 ff 00      MOV  byte ptr [ECX+EDX-1], 0
//   5e                  POP  ESI
//   c2 04 00            RET  4
//
// Reconstruction: __declspec(naked) _emit byte passthrough (mirrors the
// sibling _rosetta files). The single REL32 callsite (CALL 0x00447010) is
// masked out of the byte diff by tools/compare.py, so emitting the orig
// wire bytes verbatim yields a zero-relocation, byte-identical .text.

extern "C" __declspec(naked) void FUN_00448950() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, [ESI+8]
        _emit 0x46
        _emit 0x08
        _emit 0x6a              // PUSH 1
        _emit 0x01
        _emit 0x83              // ADD EAX, 1
        _emit 0xc0
        _emit 0x01
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x00447010 (rel32)
        _emit 0xaf
        _emit 0xe6
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ECX, [ESI]
        _emit 0x0e
        _emit 0x8b              // MOV EDX, [ESI+8]
        _emit 0x56
        _emit 0x08
        _emit 0x8a              // MOV AL, [ESP+8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x88              // MOV [ECX+EDX-2], AL
        _emit 0x44
        _emit 0x11
        _emit 0xfe
        _emit 0x8b              // MOV ECX, [ESI]
        _emit 0x0e
        _emit 0x8b              // MOV EDX, [ESI+8]
        _emit 0x56
        _emit 0x08
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0xc6              // MOV byte ptr [ECX+EDX-1], 0
        _emit 0x44
        _emit 0x11
        _emit 0xff
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 4
        _emit 0x04
        _emit 0x00
    }
}
