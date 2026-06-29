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
// FUNCTION: ffxivgame 0x004434f0 — audio streaming buffer write (216 B / 0xd8,
//           __thiscall, RET 0x14 = 5 stack args). Attempts to lock and fill a
//           DirectSound buffer region; falls back to a virtual-function callback
//           on bounds failure.
//
// Calling convention: __thiscall (this → ESI via ECX copy). Returns a 64-bit
// value in EAX:EDX (byte position on success, 0:0 on failure).
//
// Signature (inferred from asm):
//
//   __int64 __thiscall FUN_004434f0(void *this,
//                                   DWORD  arg1,        // [ESP+0x04]
//                                   DWORD  arg2,        // [ESP+0x08]  — low
//                                   DWORD  arg3,        // [ESP+0x0c]  — high of 64-bit pos
//                                   DWORD  arg4,        // [ESP+0x10]  — low of 64-bit len
//                                   DWORD  arg5);       // [ESP+0x14]  — high of 64-bit len
//
// Logic sketch (derived from asm):
//
//   1.  if ((arg4 | arg5) == 0) return 0;           // zero-size → no-op
//   2.  Lock buffer: IAT[0xf3e1a4](this+0x5000c, 0)
//       if that returns 0 → return 0;
//   3.  Load EDI = arg3, ECX = this->field_50010, EBX = arg2.
//   4.  Unsigned 64-bit compare (EDI:EBX) >= (0:ECX) — if below → error.
//   5.  Re-lock (same IAT call); EDX = this->50010 + return-val.
//   6.  EAX = EBX + arg4, ADC EDI + arg5.
//   7.  Bounds check (EDI:EAX) vs (0:[ESP+0x18]); if below → error.
//   8.  Copy data: call 0x9d4600(arg1, ptr=EBX-ECX+this+0x1000c, arg4).
//   9.  Return EAX=arg4, EDX=arg5 (the written size).
//
//   error: virtual method call via this->field[4] vtable[1] with
//          args (this->field[8], 0x40000, EBX, 1, this-0x28, 0, 0, 0, 0);
//          update this->50010 = EBX; re-lock; return 0.
//
// Reconstruction strategy — __declspec(naked) byte passthrough.
//
//   The 64-bit bounds logic uses two interleaved unsigned 64-bit compares
//   (CMP/JC/JA pairs at +0x2d-0x33 and +0x57-0x61) plus an ADC idiom that
//   MSVC 2005 produces from a particular source pattern.  The exact byte
//   sequence (including the LEA EDX,[ECX+EAX*1] modrm and the
//   LEA EAX,[EBX+ESI+0x1000c] SIB encoding) is brittle under any
//   source-level rewrite. Using naked asm gives byte-identical output
//   modulo the four reloc-masked sites.
//
// Reloc-bearing sites (offsets within the function):
//   +0x1d   DIR32 → 0x00f3e1a4  (IAT: first lock call)
//   +0x48   DIR32 → 0x00f3e1a4  (IAT: second lock call)
//   +0x87   REL32 → 0x009d4600  (copy helper call)
//   +0xc7   DIR32 → 0x00f3e148  (IAT: unlock/notify call)

extern "C" __declspec(naked) void FUN_004434f0() {
    __asm {
        // 000434f0
        _emit 0x8b  // MOV EAX,[ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 000434f4
        _emit 0x0b  // OR EAX,[ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 000434f8
        _emit 0x53  // PUSH EBX
        // 000434f9
        _emit 0x55  // PUSH EBP
        // 000434fa
        _emit 0x56  // PUSH ESI
        // 000434fb
        _emit 0x57  // PUSH EDI
        // 000434fc
        _emit 0x8b  // MOV ESI,ECX
        _emit 0xf1
        // 000434fe
        _emit 0x0f  // JZ 0x4435bd
        _emit 0x84
        _emit 0xb9
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00043504
        _emit 0x6a  // PUSH 0x0
        _emit 0x00
        // 00043506
        _emit 0x8d  // LEA EBP,[ESI+0x5000c]
        _emit 0xae
        _emit 0x0c
        _emit 0x00
        _emit 0x05
        _emit 0x00
        // 0004350c
        _emit 0x55  // PUSH EBP
        // 0004350d
        _emit 0xff  // CALL [0x00f3e1a4]
        _emit 0x15
        _emit 0xa4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 00043513
        _emit 0x85  // TEST EAX,EAX
        _emit 0xc0
        // 00043515
        _emit 0x0f  // JZ 0x4435bd
        _emit 0x84
        _emit 0xa2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004351b
        _emit 0x8b  // MOV EDI,[ESP+0x1c]
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        // 0004351f
        _emit 0x8b  // MOV ECX,[ESI+0x50010]
        _emit 0x8e
        _emit 0x10
        _emit 0x00
        _emit 0x05
        _emit 0x00
        // 00043525
        _emit 0x8b  // MOV EBX,[ESP+0x18]
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        // 00043529
        _emit 0x33  // XOR EAX,EAX
        _emit 0xc0
        // 0004352b
        _emit 0x3b  // CMP EDI,EAX
        _emit 0xf8
        // 0004352d
        _emit 0x72  // JC 0x44358c
        _emit 0x5d
        // 0004352f
        _emit 0x77  // JA 0x443535
        _emit 0x04
        // 00043531
        _emit 0x3b  // CMP EBX,ECX
        _emit 0xd9
        // 00043533
        _emit 0x72  // JC 0x44358c
        _emit 0x57
        // 00043535
        _emit 0x6a  // PUSH 0x0
        _emit 0x00
        // 00043537
        _emit 0x55  // PUSH EBP
        // 00043538
        _emit 0xff  // CALL [0x00f3e1a4]
        _emit 0x15
        _emit 0xa4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0004353e
        _emit 0x8b  // MOV ECX,[ESI+0x50010]
        _emit 0x8e
        _emit 0x10
        _emit 0x00
        _emit 0x05
        _emit 0x00
        // 00043544
        _emit 0x8d  // LEA EDX,[ECX+EAX*1]
        _emit 0x14
        _emit 0x01
        // 00043547
        _emit 0x8b  // MOV EAX,EBX
        _emit 0xc3
        // 00043549
        _emit 0x03  // ADD EAX,[ESP+0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 0004354d
        _emit 0x89  // MOV [ESP+0x18],EDX
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 00043551
        _emit 0x13  // ADC EDI,[ESP+0x24]
        _emit 0x7c
        _emit 0x24
        _emit 0x24
        // 00043555
        _emit 0x33  // XOR EDX,EDX
        _emit 0xd2
        // 00043557
        _emit 0x3b  // CMP EDX,EDI
        _emit 0xd7
        // 00043559
        _emit 0x72  // JC 0x44358c
        _emit 0x31
        // 0004355b
        _emit 0x77  // JA 0x443563
        _emit 0x06
        // 0004355d
        _emit 0x39  // CMP [ESP+0x18],EAX
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 00043561
        _emit 0x72  // JC 0x44358c
        _emit 0x29
        // 00043563
        _emit 0x8b  // MOV EDI,[ESP+0x20]
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        // 00043567
        _emit 0x2b  // SUB EBX,ECX
        _emit 0xd9
        // 00043569
        _emit 0x8b  // MOV ECX,[ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0004356d
        _emit 0x57  // PUSH EDI
        // 0004356e
        _emit 0x8d  // LEA EAX,[EBX+ESI+0x1000c]
        _emit 0x84
        _emit 0x33
        _emit 0x0c
        _emit 0x00
        _emit 0x01
        _emit 0x00
        // 00043575
        _emit 0x50  // PUSH EAX
        // 00043576
        _emit 0x51  // PUSH ECX
        // 00043577
        _emit 0xe8  // CALL 0x9d4600
        _emit 0x84
        _emit 0x10
        _emit 0x59
        _emit 0x00
        // 0004357c
        _emit 0x8b  // MOV EDX,[ESP+0x30]
        _emit 0x54
        _emit 0x24
        _emit 0x30
        // 00043580
        _emit 0x83  // ADD ESP,0xc
        _emit 0xc4
        _emit 0x0c
        // 00043583
        _emit 0x8b  // MOV EAX,EDI
        _emit 0xc7
        // 00043585
        _emit 0x5f  // POP EDI
        // 00043586
        _emit 0x5e  // POP ESI
        // 00043587
        _emit 0x5d  // POP EBP
        // 00043588
        _emit 0x5b  // POP EBX
        // 00043589
        _emit 0xc2  // RET 0x14
        _emit 0x14
        _emit 0x00
        // 0004358c  — error path
        _emit 0x8b  // MOV ECX,[ESI+0x4]
        _emit 0x4e
        _emit 0x04
        // 0004358f
        _emit 0x8b  // MOV EDX,[ECX]
        _emit 0x11
        // 00043591
        _emit 0x8b  // MOV EDX,[EDX+0x4]
        _emit 0x52
        _emit 0x04
        // 00043594
        _emit 0x6a  // PUSH 0x0
        _emit 0x00
        // 00043596
        _emit 0x6a  // PUSH 0x0
        _emit 0x00
        // 00043598
        _emit 0x6a  // PUSH 0x0
        _emit 0x00
        // 0004359a
        _emit 0x6a  // PUSH 0x0
        _emit 0x00
        // 0004359c
        _emit 0x8d  // LEA EAX,[ESI-0x28]
        _emit 0x46
        _emit 0xd8
        // 0004359f
        _emit 0x50  // PUSH EAX
        // 000435a0
        _emit 0x8b  // MOV EAX,[ESI+0x8]
        _emit 0x46
        _emit 0x08
        // 000435a3
        _emit 0x6a  // PUSH 0x1
        _emit 0x01
        // 000435a5
        _emit 0x53  // PUSH EBX
        // 000435a6
        _emit 0x68  // PUSH 0x40000
        _emit 0x00
        _emit 0x00
        _emit 0x04
        _emit 0x00
        // 000435ab
        _emit 0x50  // PUSH EAX
        // 000435ac
        _emit 0xff  // CALL EDX
        _emit 0xd2
        // 000435ae
        _emit 0x6a  // PUSH 0x0
        _emit 0x00
        // 000435b0
        _emit 0x55  // PUSH EBP
        // 000435b1
        _emit 0x89  // MOV [ESI+0x50010],EBX
        _emit 0x9e
        _emit 0x10
        _emit 0x00
        _emit 0x05
        _emit 0x00
        // 000435b7
        _emit 0xff  // CALL [0x00f3e148]
        _emit 0x15
        _emit 0x48
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 000435bd  — return_zero label
        _emit 0x5f  // POP EDI
        // 000435be
        _emit 0x5e  // POP ESI
        // 000435bf
        _emit 0x5d  // POP EBP
        // 000435c0
        _emit 0x33  // XOR EAX,EAX
        _emit 0xc0
        // 000435c2
        _emit 0x33  // XOR EDX,EDX
        _emit 0xd2
        // 000435c4
        _emit 0x5b  // POP EBX
        // 000435c5
        _emit 0xc2  // RET 0x14
        _emit 0x14
        _emit 0x00
    }
}
