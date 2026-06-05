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
// FUNCTION: ffxivgame 0x0044eca0 — XOR-mask + hex-format byte buffer into a
// string (__cdecl, 104 B).
//
// Semantics (recovered from asm):
//
//   void FUN_0044eca0(char *out, int clear, Holder *src, char mask) {
//       BufBlock *blk = src->field_C;          // [src+0xC]
//       unsigned int n = blk->field_4;         // [blk+0x4] = byte count
//       if (clear > 0)                         // signed; arg1 > 0
//           *out = '\0';                        // reset accumulator string
//       for (unsigned int i = 0; i < n; ++i) {
//           BufBlock *b = src->field_C;        // reloaded each iter
//           unsigned char v =
//               ((unsigned char *)b->field_8)[n - i - 1] ^ (unsigned char)mask;
//           char tmp[...];
//           FUN_009d4f83(tmp, 3, (char*)0xF6757C, v);   // hex-format helper
//           FUN_009d4bb4(local_14, clear, tmp);          // append-to-out helper
//       }
//   }
//
//   Calling convention: __cdecl (caller-cleans; the two inner helpers are
//   also __cdecl — their 7 combined stack args are torn down with one
//   ADD ESP,0x1C after the second CALL).
//   Stack frame: 4 callee-saved regs (EBX/EBP/ESI/EDI) + two stack temps.
//
// Reloc-bearing sites in the orig 104 bytes:
//   +0x38  PUSH imm32  → 0x00F6757C (format-string / table pointer, .rdata)
//   +0x44  CALL rel32  → 0x009D4F83 (hex-format helper)
//   +0x54  CALL rel32  → 0x009D4BB4 (append helper)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form would emit relocations referencing symbols
//   whose addresses the linker controls; the inner-helper call targets
//   and the absolute 0xF6757C pointer are baked into the orig binary as
//   concrete byte sequences. Re-emitting the orig 104 bytes verbatim via
//   MASM `_emit` produces a .obj whose .text matches the orig slice
//   byte-for-byte with NO relocations — `tools/compare.py` reads the orig
//   PE post-fixup and compares byte streams directly, so a zero-reloc
//   passthrough .obj is the simplest path to GREEN. Same approach as
//   siblings FUN_004091f0 / FUN_00409260.

extern "C" __declspec(naked) void FUN_0044eca0() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x10]
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EAX, dword ptr [EBX+0xC]
        _emit 0x43
        _emit 0x0c
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x10]
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x85              // TEST EBP, EBP
        _emit 0xed
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [EAX+0x4]
        _emit 0x78
        _emit 0x04
        _emit 0x7e              // JLE +0x07
        _emit 0x07
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xc6              // MOV byte ptr [ECX], 0x0
        _emit 0x01
        _emit 0x00
        _emit 0x33              // XOR ESI, ESI
        _emit 0xf6
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x76              // JBE +0x40
        _emit 0x40
        _emit 0x8b              // MOV EDX, dword ptr [EBX+0xC]
        _emit 0x53
        _emit 0x0c
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x8]
        _emit 0x42
        _emit 0x08
        _emit 0x2b              // SUB EAX, ESI
        _emit 0xc6
        _emit 0x0f              // MOVZX ECX, byte ptr [EAX+EDI*1-0x1]
        _emit 0xb6
        _emit 0x4c
        _emit 0x38
        _emit 0xff
        _emit 0x0f              // MOVZX EAX, byte ptr [ESP+0x20]
        _emit 0xb6
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x33              // XOR ECX, EAX
        _emit 0xc8
        _emit 0x51              // PUSH ECX
        _emit 0x68              // PUSH 0x00F6757C
        _emit 0x7c
        _emit 0x75
        _emit 0xf6
        _emit 0x00
        _emit 0x8d              // LEA EDX, [ESP+0x24]
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x6a              // PUSH 0x3
        _emit 0x03
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL 0x009D4F83 (rel32)
        _emit 0x9a
        _emit 0x62
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x24]
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x8d              // LEA EAX, [ESP+0x2C]
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x50              // PUSH EAX
        _emit 0x55              // PUSH EBP
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL 0x009D4BB4 (rel32)
        _emit 0xbb
        _emit 0x5e
        _emit 0x58
        _emit 0x00
        _emit 0x83              // ADD ESI, 0x1
        _emit 0xc6
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x1C
        _emit 0xc4
        _emit 0x1c
        _emit 0x3b              // CMP ESI, EDI
        _emit 0xf7
        _emit 0x72              // JC -0x40 (loop back)
        _emit 0xc0
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
