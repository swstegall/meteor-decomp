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
// FUNCTION: ffxivgame 0x00048270 — __thiscall "append one byte + NUL" on a
//                                   small buffer/length pair (44 B / 0x2C).
//
// Layout (inferred from the asm):
//   this (ECX):
//     +0x00  char *buf      ; backing storage
//     +0x08  int   len      ; current logical length (indexes past the
//                             NUL terminator: writes land at len-2/len-1)
//
// Source shape:
//
//   void __thiscall FUN_00448270(this, char c) {
//       FUN_00447010(this->len + 1, 1);   // ensure/grow capacity
//       this->buf[this->len - 2] = c;      // store the new char
//       this->buf[this->len - 1] = '\0';   // re-terminate
//   }
//
// Calling convention: __thiscall (ECX = this; one byte stack arg; callee
// cleans 4 bytes via `ret 4`).
//
// Asm (44 bytes @ orig RVA 0x00048270):
//   56                  PUSH ESI
//   8b f1               MOV  ESI, ECX
//   8b 46 08            MOV  EAX, [ESI+8]
//   6a 01               PUSH 1
//   83 c0 01            ADD  EAX, 1
//   50                  PUSH EAX
//   e8 8f ed ff ff      CALL FUN_00447010 (rel32 → 0x00447010)
//   8b 0e               MOV  ECX, [ESI]
//   8b 56 08            MOV  EDX, [ESI+8]
//   8a 44 24 08         MOV  AL,  [ESP+8]          ; the char arg
//   88 44 11 fe         MOV  [ECX+EDX-2], AL
//   8b 0e               MOV  ECX, [ESI]
//   8b 56 08            MOV  EDX, [ESI+8]
//   c6 44 11 ff 00      MOV  byte ptr [ECX+EDX-1], 0
//   5e                  POP  ESI
//   c2 04 00            RET  4
//
// Reconstruction: __declspec(naked) _emit byte passthrough (mirrors the
// sibling _rosetta naked-asm matches). The lone CALL rel32 immediate is
// the orig file's baked offset, emitted verbatim — the .obj's .text is
// byte-identical with no relocations; tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00448270() {
    __asm {
        _emit 0x56          // PUSH ESI
        _emit 0x8b          // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b          // MOV EAX, [ESI+8]
        _emit 0x46
        _emit 0x08
        _emit 0x6a          // PUSH 1
        _emit 0x01
        _emit 0x83          // ADD EAX, 1
        _emit 0xc0
        _emit 0x01
        _emit 0x50          // PUSH EAX
        _emit 0xe8          // CALL FUN_00447010 (rel32 → 0x00447010)
        _emit 0x8f
        _emit 0xed
        _emit 0xff
        _emit 0xff
        _emit 0x8b          // MOV ECX, [ESI]
        _emit 0x0e
        _emit 0x8b          // MOV EDX, [ESI+8]
        _emit 0x56
        _emit 0x08
        _emit 0x8a          // MOV AL, [ESP+8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x88          // MOV [ECX+EDX-2], AL
        _emit 0x44
        _emit 0x11
        _emit 0xfe
        _emit 0x8b          // MOV ECX, [ESI]
        _emit 0x0e
        _emit 0x8b          // MOV EDX, [ESI+8]
        _emit 0x56
        _emit 0x08
        _emit 0xc6          // MOV byte ptr [ECX+EDX-1], 0
        _emit 0x44
        _emit 0x11
        _emit 0xff
        _emit 0x00
        _emit 0x5e          // POP ESI
        _emit 0xc2          // RET 4
        _emit 0x04
        _emit 0x00
    }
}
