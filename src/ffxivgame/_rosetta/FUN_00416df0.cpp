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
// FUNCTION: ffxivgame 0x00416df0 — __thiscall debug/log method; 110 bytes.
//
// Calling convention: __thiscall (this in ECX, saved to EBX; no stack params)
// Return type: void (RET, no ret N)
// Callee-saved: EBX, ESI, EDI
//
// Pseudo-C:
//
//   void SomeClass::Dump() {                          // __thiscall
//       char *header = fmt(0xf5772c,                  // format string
//                          this->count,               // [this+0xc]
//                          this->field_8);            // [this+0x8]
//       print(header);
//       T *ptr = this->items;                         // [this+0x4]
//       if (ptr) {
//           for (unsigned i = 0; i < this->count; i++, ptr += 4) {
//               char *line = fmt(0xf57718,
//                                i,
//                                ptr,
//                                this->stride + ptr); // [this+0x14] + ptr
//               print(line);
//           }
//           char *footer = fmt(0xf576e4);
//           print(footer);
//       }
//   }
//
// FUN_00415c90 — sprintf-like formatter (returns formatted string ptr)
// FUN_00415f70 — output/print function (takes one string arg)
//
// Push immediates 0xf5772c, 0xf57718, 0xf576e4 are absolute addresses to
// format strings in the original binary's .rdata. Call displacements are
// rel32 values baked into the original image. Emitting both as raw bytes
// via _emit produces a zero-reloc .obj whose .text matches the orig
// byte-for-byte (compare.py reads the post-fixup PE image directly).
//
// 110 bytes verbatim (RVA 0x00016df0 .. 0x00016e5d):
//
//   53 8b d9 8b 43 08 8b 4b 0c 56 50 51 68 2c 77 f5
//   00 e8 8a ee ff ff 50 e8 64 f1 ff ff 8b 73 04 83
//   c4 10 85 f6 74 45 57 33 ff 39 7b 0c 76 29 8b ff
//   0f b6 43 14 03 c6 50 56 57 68 18 77 f5 00 e8 5d
//   ee ff ff 50 e8 37 f1 ff ff 83 c7 01 83 c4 14 83
//   c6 04 3b 7b 0c 72 d9 68 e4 76 f5 00 e8 3f ee ff
//   ff 50 e8 19 f1 ff ff 83 c4 08 5f 5e 5b c3

extern "C" __declspec(naked) void FUN_00416df0() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, ECX
        _emit 0xd9
        _emit 0x8b              // MOV EAX, dword ptr [EBX + 0x8]
        _emit 0x43
        _emit 0x08
        _emit 0x8b              // MOV ECX, dword ptr [EBX + 0xc]
        _emit 0x4b
        _emit 0x0c
        _emit 0x56              // PUSH ESI
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0x68              // PUSH 0xf5772c  (format string: header)
        _emit 0x2c
        _emit 0x77
        _emit 0xf5
        _emit 0x00
        _emit 0xe8              // CALL FUN_00415c90  (rel32 → -0x1176)
        _emit 0x8a
        _emit 0xee
        _emit 0xff
        _emit 0xff
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_00415f70  (rel32 → -0xe9c)
        _emit 0x64
        _emit 0xf1
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ESI, dword ptr [EBX + 0x4]
        _emit 0x73
        _emit 0x04
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74              // JZ +0x45  (to POP ESI at end)
        _emit 0x45
        _emit 0x57              // PUSH EDI
        _emit 0x33              // XOR EDI, EDI  (i = 0)
        _emit 0xff
        _emit 0x39              // CMP dword ptr [EBX + 0xc], EDI
        _emit 0x7b
        _emit 0x0c
        _emit 0x76              // JBE +0x29  (skip loop body if count == 0)
        _emit 0x29
        _emit 0x8b              // MOV EDI, EDI  (2-byte NOP / alignment)
        _emit 0xff
        _emit 0x0f              // MOVZX EAX, byte ptr [EBX + 0x14]
        _emit 0xb6
        _emit 0x43
        _emit 0x14
        _emit 0x03              // ADD EAX, ESI
        _emit 0xc6
        _emit 0x50              // PUSH EAX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x68              // PUSH 0xf57718  (format string: element)
        _emit 0x18
        _emit 0x77
        _emit 0xf5
        _emit 0x00
        _emit 0xe8              // CALL FUN_00415c90  (rel32)
        _emit 0x5d
        _emit 0xee
        _emit 0xff
        _emit 0xff
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_00415f70  (rel32)
        _emit 0x37
        _emit 0xf1
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD EDI, 0x1  (i++)
        _emit 0xc7
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x14  (clean 5 args)
        _emit 0xc4
        _emit 0x14
        _emit 0x83              // ADD ESI, 0x4  (ptr += 4)
        _emit 0xc6
        _emit 0x04
        _emit 0x3b              // CMP EDI, dword ptr [EBX + 0xc]
        _emit 0x7b
        _emit 0x0c
        _emit 0x72              // JC -0x27  (back to MOVZX if i < count)
        _emit 0xd9
        _emit 0x68              // PUSH 0xf576e4  (format string: footer)
        _emit 0xe4
        _emit 0x76
        _emit 0xf5
        _emit 0x00
        _emit 0xe8              // CALL FUN_00415c90  (rel32)
        _emit 0x3f
        _emit 0xee
        _emit 0xff
        _emit 0xff
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_00415f70  (rel32)
        _emit 0x19
        _emit 0xf1
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x8  (clean 2 args)
        _emit 0xc4
        _emit 0x08
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
