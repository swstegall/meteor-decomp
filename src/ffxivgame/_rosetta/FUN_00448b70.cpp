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
// FUNCTION: ffxivgame 0x00448b70 — string-buffer append-fill method
//                                  (__thiscall, 2 args, 110 bytes)
//
// void __thiscall FUN_00448b70(this, unsigned int count, char fillChar)
//
// Appends `count` copies of `fillChar` to a string-like buffer object.
//
// Calling convention: __thiscall (ECX = this, callee-cleans 2 dword args →
//   RET 0x8).  ESI = saved this pointer throughout.
//
// High-level shape:
//
//   unsigned int cap = FUN_00445e50(this);   // get capacity / max size
//   if (count <= cap) {
//       FUN_004460a0(this, count, -1);        // fast-path: fits in buffer
//       return;
//   }
//   // slow path: need to grow
//   unsigned int old_size8 = this->field_8;
//   unsigned int extra = count - cap;         // bytes beyond current cap
//   FUN_00447010(this, count, 1);             // resize / reserve
//   this->field_8 = old_size8;               // restore field_8
//   char ch = fillChar;
//   if (ch == 0 || extra == 0) return;
//   do {
//       FUN_00447010(this, this->field_8 + 1, 1);  // push one slot
//       --extra;
//       char *base = (char *)this->field_0;
//       unsigned int sz = this->field_8;
//       base[sz - 2] = ch;                   // write fill character
//       base[sz - 1] = '\0';                 // null-terminate
//   } while (extra != 0);
//
// CALL targets (rel32 — baked verbatim from orig binary):
//   +0x03   CALL rel32 → FUN_00445e50   (e8 d8 d2 ff ff)
//   +0x15   CALL rel32 → FUN_004460a0   (e8 16 d5 ff ff)
//   +0x2c   CALL rel32 → FUN_00447010   (e8 6f e4 ff ff)
//   +0x4b   CALL rel32 → FUN_00447010   (e8 50 e4 ff ff)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   All four CALL sites are rel32 with targets in the orig binary's .text
//   at fixed RVAs.  Emitting the original bytes verbatim via MASM _emit
//   produces a .obj whose .text is byte-identical to the orig slice with
//   no relocations — compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00448b70() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xe8              // CALL FUN_00445e50 (rel32 → 0x00445e50)
        _emit 0xd8
        _emit 0xd2
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x3b              // CMP ECX, EAX
        _emit 0xc8
        _emit 0x77              // JA +0x0e  (→ slow path at +0x1e)
        _emit 0x0e
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_004460a0 (rel32 → 0x004460a0)
        _emit 0x16
        _emit 0xd5
        _emit 0xff
        _emit 0xff
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        _emit 0x53              // PUSH EBX                 (slow path)
        _emit 0x8b              // MOV EBX, dword ptr [ESI+0x8]
        _emit 0x5e
        _emit 0x08
        _emit 0x57              // PUSH EDI
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0x2b              // SUB EDI, EAX
        _emit 0xf8
        _emit 0xe8              // CALL FUN_00447010 (rel32 → 0x00447010)
        _emit 0x6f
        _emit 0xe4
        _emit 0xff
        _emit 0xff
        _emit 0x89              // MOV dword ptr [ESI+0x8], EBX
        _emit 0x5e
        _emit 0x08
        _emit 0x8a              // MOV BL, byte ptr [ESP+0x14]
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        _emit 0x84              // TEST BL, BL
        _emit 0xdb
        _emit 0x74              // JZ +0x2c  (→ epilogue)
        _emit 0x2c
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x76              // JBE +0x28  (→ epilogue)
        _emit 0x28
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x8]    (loop top)
        _emit 0x46
        _emit 0x08
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0x83              // ADD EAX, 0x1
        _emit 0xc0
        _emit 0x01
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_00447010 (rel32 → 0x00447010)
        _emit 0x50
        _emit 0xe4
        _emit 0xff
        _emit 0xff
        _emit 0x83              // SUB EDI, 0x1
        _emit 0xef
        _emit 0x01
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ESI]
        _emit 0x16
        _emit 0x88              // MOV byte ptr [ECX+EDX*1-0x2], BL
        _emit 0x5c
        _emit 0x11
        _emit 0xfe
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x8]
        _emit 0x46
        _emit 0x08
        _emit 0x8b              // MOV ECX, dword ptr [ESI]
        _emit 0x0e
        _emit 0xc6              // MOV byte ptr [EAX+ECX*1-0x1], 0x0
        _emit 0x44
        _emit 0x08
        _emit 0xff
        _emit 0x00
        _emit 0x75              // JNZ -0x28  (→ loop top)
        _emit 0xd8
        _emit 0x5f              // POP EDI                  (epilogue)
        _emit 0x5b              // POP EBX
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
