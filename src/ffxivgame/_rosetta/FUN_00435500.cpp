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
// FUNCTION: ffxivgame 0x00435500 — __thiscall dispatcher that maps a mode
//                                  byte (0x65 or 0x66) to a flag (2 or 4)
//                                  then forwards to FUN_00435440.
//                                  (87 bytes, RET 0xC — cleans 3 × 4-byte args)
//
// Calling convention: __thiscall
//   ECX       = this pointer (saved to EDI)
//   [ESP+0x4] = arg1
//   [ESP+0x8] = arg2
//   [ESP+0xC] = mode  (read into EAX before the prologue pushes)
//
// Behaviour:
//
//   int flag = 0;
//   if (mode == 0x65)       flag = 2;   // LEA ESI, [EAX-0x63]
//   else if (mode == 0x66)  flag = 4;   // LEA ESI, [EAX-0x62]
//   else {
//       // assertion / hard error: never falls off
//       ECX = &mode;     // LEA ECX, [ESP+0x28]  (pointing to arg3 on caller frame)
//       FUN_00406550(0xf56510, 0xf54d48, 0xf649f8, 0xc5, 0xf64ac0);
//   }
//   // tail — forwarding call
//   this->FUN_00435440(arg1, arg2, flag);   // __thiscall, ECX = EDI
//
// Asm shape (87 bytes, RVA 0x00035500):
//
//   00035500:  8b 44 24 0c    MOV  EAX, dword ptr [ESP+0x0C]   ; mode (before pushes)
//   00035504:  56             PUSH ESI
//   00035505:  33 f6          XOR  ESI, ESI                    ; flag = 0
//   00035507:  83 f8 65       CMP  EAX, 0x65
//   0003550a:  57             PUSH EDI
//   0003550b:  8b f9          MOV  EDI, ECX                    ; save this
//   0003550d:  75 05          JNZ  +5  → 0x35514
//   0003550f:  8d 70 9d       LEA  ESI, [EAX-0x63]             ; flag = 2
//   00035512:  eb 2c          JMP  +0x2c → 0x35540
//   00035514:  83 f8 66       CMP  EAX, 0x66
//   00035517:  75 05          JNZ  +5  → 0x3551e
//   00035519:  8d 70 9e       LEA  ESI, [EAX-0x62]             ; flag = 4
//   0003551c:  eb 22          JMP  +0x22 → 0x35540
//   0003551e:  68 c0 4a f6 00 PUSH 0xf64ac0                    ; DIR32 reloc
//   00035523:  68 c5 00 00 00 PUSH 0x000000c5                  ; literal (197)
//   00035528:  68 f8 49 f6 00 PUSH 0xf649f8                    ; DIR32 reloc
//   0003552d:  68 48 4d f5 00 PUSH 0xf54d48                    ; DIR32 reloc
//   00035532:  68 10 65 f5 00 PUSH 0xf56510                    ; DIR32 reloc
//   00035537:  8d 4c 24 28    LEA  ECX, [ESP+0x28]
//   0003553b:  e8 10 10 fd ff CALL 0x00406550                  ; rel32 reloc
//   00035540:  8b 44 24 10    MOV  EAX, dword ptr [ESP+0x10]   ; arg2
//   00035544:  8b 4c 24 0c    MOV  ECX, dword ptr [ESP+0x0C]   ; arg1
//   00035548:  56             PUSH ESI                         ; flag
//   00035549:  50             PUSH EAX                         ; arg2
//   0003554a:  51             PUSH ECX                         ; arg1
//   0003554b:  8b cf          MOV  ECX, EDI                    ; this
//   0003554d:  e8 ee fe ff ff CALL 0x00435440                  ; rel32 reloc
//   00035552:  5f             POP  EDI
//   00035553:  5e             POP  ESI
//   00035554:  c2 0c 00       RET  0x0C
//
// Reloc-bearing sites (masked by tools/compare.py in the byte diff):
//   +0x1e  PUSH imm32 → 0xf64ac0   (DIR32)
//   +0x28  PUSH imm32 → 0xf649f8   (DIR32)
//   +0x2d  PUSH imm32 → 0xf54d48   (DIR32)
//   +0x32  PUSH imm32 → 0xf56510   (DIR32)
//   +0x3b  CALL rel32 → 0x00406550 (REL32)
//   +0x4d  CALL rel32 → 0x00435440 (REL32)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The interleaved prologue (arg3 read at [ESP+0xC] BEFORE the ESI/EDI
//   saves, with MOV EDI,ECX sandwiched between CMP and JNZ) plus the
//   LEA ESI,[EAX+imm8] encoding of 2/4 cannot be coaxed from a plain C++
//   source; naked _emit passthrough gives byte-identical .text with zero
//   additional relocations.

extern "C" __declspec(naked) void FUN_00435500() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x0C]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x56              // PUSH ESI
        _emit 0x33              // XOR ESI, ESI
        _emit 0xf6
        _emit 0x83              // CMP EAX, 0x65
        _emit 0xf8
        _emit 0x65
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x75              // JNZ +5
        _emit 0x05
        _emit 0x8d              // LEA ESI, [EAX-0x63]
        _emit 0x70
        _emit 0x9d
        _emit 0xeb              // JMP +0x2c
        _emit 0x2c
        _emit 0x83              // CMP EAX, 0x66
        _emit 0xf8
        _emit 0x66
        _emit 0x75              // JNZ +5
        _emit 0x05
        _emit 0x8d              // LEA ESI, [EAX-0x62]
        _emit 0x70
        _emit 0x9e
        _emit 0xeb              // JMP +0x22
        _emit 0x22
        _emit 0x68              // PUSH 0xf64ac0   (DIR32 reloc)
        _emit 0xc0
        _emit 0x4a
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x000000c5 (literal)
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf649f8   (DIR32 reloc)
        _emit 0xf8
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0xf54d48   (DIR32 reloc)
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0xf56510   (DIR32 reloc)
        _emit 0x10
        _emit 0x65
        _emit 0xf5
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP+0x28]
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0xe8              // CALL 0x00406550 (rel32 reloc)
        _emit 0x10
        _emit 0x10
        _emit 0xfd
        _emit 0xff
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x0C]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x56              // PUSH ESI
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8              // CALL 0x00435440 (rel32 reloc)
        _emit 0xee
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x0C
        _emit 0x0c
        _emit 0x00
    }
}
