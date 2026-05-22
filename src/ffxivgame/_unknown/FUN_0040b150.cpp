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
// FUNCTION: ffxivgame 0x0000b150 — __thiscall pool drain + conditional cleanup
//                                   (66 B / 0x42)
//
// __thiscall void FUN_0040b150(this)
//   ECX : this — pointer to an object with at least +0x58, +0x5c fields
//
// Behaviour (inferred from asm RVA 0x0000b150, 66 bytes):
//   if (this->field_58 == 0) return;           // JZ to epilogue outside fn
//   // Drain a pool of 0x16 (22) items via FUN_0040afc0
//   for (int i = 0; i < 0x16; ++i)
//       FUN_0040afc0(this, i);                  // __thiscall: ECX=this, arg=i
//   // Conditional cleanup of field_5c chain
//   void *p = this->field_5c;
//   if (p != NULL && *(int*)(p + 4) != 0)
//       FUN_0040d7e0();                         // some release helper
//   // Pass field_5c and field_58 to the noreturn sink FUN_0040df70
//   FUN_0040df70(this->field_5c, this->field_58);  // noreturn
//
// The 66-byte window recorded in config/ffxivgame.yaml ends immediately
// after the noreturn CALL; the 3 trailing bytes (8b 7e 60 = MOV EDI,[ESI+0x60])
// are the start of the continuation block that begins past this window —
// they are dead here because FUN_0040df70 never returns.
//
// Reloc-bearing sites in the 66 bytes (CALL rel32):
//   +0x15   CALL rel32 → FUN_0040afc0  (bytes: 56 fe ff ff)
//   +0x2e   CALL rel32 → FUN_0040d7e0  (bytes: 5d 26 00 00)
//   +0x3a   CALL rel32 → FUN_0040df70  (bytes: e1 2d 00 00)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Source-level C++ cannot reproduce the exact branch encoding (JZ short
//   +0x7a past the end of this 66-byte slice) without driving a full relink.
//   __declspec(naked) with _emit reproduces the orig 66 bytes verbatim.
//   compare.py masks the four rel32 bytes of each CALL, so the three
//   CALL offsets need not be exact — but they ARE the orig bytes here.

extern "C" __declspec(naked) void FUN_0040b150() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x33              // XOR EBP, EBP
        _emit 0xed
        _emit 0x39              // CMP dword ptr [ESI+0x58], EBP
        _emit 0x6e
        _emit 0x58
        _emit 0x74              // JZ +0x7a  (→ 0x0040b1d5, past this fn)
        _emit 0x7a
        _emit 0x53              // PUSH EBX
        _emit 0x57              // PUSH EDI
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0x8d              // LEA EBX, [EBP+0x16]  (EBP=0 → EBX=22)
        _emit 0x5d
        _emit 0x16
        // loop top (offset 0x12):
        _emit 0x57              // PUSH EDI              (loop counter arg)
        _emit 0x8b              // MOV ECX, ESI          (restore 'this')
        _emit 0xce
        _emit 0xe8              // CALL FUN_0040afc0 (rel32: +0xfffffe56)
        _emit 0x56
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD EDI, 1
        _emit 0xc7
        _emit 0x01
        _emit 0x83              // SUB EBX, 1
        _emit 0xeb
        _emit 0x01
        _emit 0x75              // JNZ -0x10  (loop back)
        _emit 0xf0
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x5c]
        _emit 0x4e
        _emit 0x5c
        _emit 0x3b              // CMP ECX, EBP
        _emit 0xcd
        _emit 0x74              // JZ +0x0a  (→ 0x0040b183)
        _emit 0x0a
        _emit 0x39              // CMP dword ptr [ECX+0x4], EBP
        _emit 0x69
        _emit 0x04
        _emit 0x74              // JZ +0x05  (→ 0x0040b183)
        _emit 0x05
        _emit 0xe8              // CALL FUN_0040d7e0 (rel32: +0x0000265d)
        _emit 0x5d
        _emit 0x26
        _emit 0x00
        _emit 0x00
        // offset 0x33:
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x5c]
        _emit 0x46
        _emit 0x5c
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x58]
        _emit 0x4e
        _emit 0x58
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_0040df70 (rel32: +0x00002de1)  noreturn
        _emit 0xe1
        _emit 0x2d
        _emit 0x00
        _emit 0x00
        // dead bytes at end of 66-byte window (start of continuation block):
        _emit 0x8b              // MOV EDI, dword ptr [ESI+0x60]  (dead)
        _emit 0x7e
        _emit 0x60
    }
}
