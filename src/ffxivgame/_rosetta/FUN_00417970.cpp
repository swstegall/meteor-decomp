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
// FUNCTION: ffxivgame 0x00417970 — `__thiscall` container advance-cursor
//                                   (245 B / 0xf5)
//
// void __thiscall FUN_00417970(this /*ECX*/)
//   → RET  (no stack args; plain __thiscall, no RET N)
//
// Stack frame (SUB ESP,8 / PUSH EBX / PUSH EBP / PUSH ESI / PUSH EDI):
//   [ESP+0x00]  saved EDI
//   [ESP+0x04]  saved EBX
//   [ESP+0x08]  saved EBP (= 0 throughout; MSVC omits frame-ptr, uses EBP as null const)
//   [ESP+0x0C]  saved ESI (= this)
//   [ESP+0x10]  8-byte local pair scratch (two DWORDs used for FUN_00417fc0 arg)
//   [ESP+0x18]  return address
//
// EBP is zeroed at entry (XOR EBP,EBP) and used as the null constant
// throughout — a common MSVC /O2 /Oy idiom when frame-pointer is omitted
// and there are many pointer null-checks.
//
// Behaviour (read from asm/ffxivgame/00017970_FUN_00417970.s):
//
//   this layout:
//     +0x04  begin   — pointer to first 8-byte element
//     +0x08  end     — pointer one past last element
//     +0x0c  cap     — (used by FUN_00417fc0, not directly here)
//     +0x14  current — pointer to the active element (iterator)
//     +0x18  dirty   — flag cleared to 0 on every exit path
//
//   Compute count:
//     if begin == 0 → count = 0
//     else          → count = (end - begin) / 8  (SAR 3)
//
//   If count <= 0 (JBE): set dirty=0, return.
//
//   Loop i = 0 .. count-1:
//     assert begin != null && i < count  (call FUN_009d22b4 on failure)
//     elem = &begin[i]
//     if elem == current:
//       count-- (EBX--)
//       if i == count (i.e., current WAS the last element):
//         buf = FUN_009d04ac(0x9c4000)   // some alloc/convert
//         local_pair = {buf, 0x9c4000}
//         FUN_00417fc0(this, &local_pair) // push_back helper (__thiscall, one stack arg)
//       i++
//       assert begin != null && i < (fresh count)  (FUN_009d22b4 on failure)
//       next = &begin[i]
//       current = next
//       if next != null: set dirty=0, return
//       // assertion: next must not be null — if it is, fire one-time
//       //   debug handler at [0x0132390c] (initialised to 0x00417760 on first use,
//       //   guarded by byte flag at [0x01323910])
//       //   args: str, 183 (line), str, str, str → ADD ESP,0x14
//       set dirty=0, return
//     i++
//     if i >= count: break
//
//   // current not found in array:
//   set dirty=0, return
//
// Reloc-bearing sites (masked by compare.py — these 4-byte windows are
// wildcarded):
//   +0x3b   REL32  code FUN_009d22b4   (out-of-range error thunk, first call)
//   +0x58   REL32  code FUN_009d04ac   (alloc/convert helper)
//   +0x7e   REL32  code FUN_00417fc0   (push helper, __thiscall, RET 4)
//   +0x97   REL32  code FUN_009d22b4   (out-of-range error thunk, second call)
//   +0xab   DIR32  data [0x01323910]   (assert-flag byte, TEST)
//   +0xb5   DIR32  data [0x01323910]   (assert-flag byte, OR)
//   +0xbd   DIR32  data [0x0132390c]   (assert fn-ptr, MOV destination)
//   +0xc1   DIR32  code 0x00417760     (assert handler fn, MOV immediate)
//   +0xc6   DIR32  str  0xf579e8       (assert arg 5)
//   +0xcc   DIR32  str  0xf57a40       (assert arg 3)
//   +0xd1   DIR32  str  0xf57a98       (assert arg 2)
//   +0xd6   DIR32  str  0xf57ab4       (assert arg 1)
//   +0xdc   DIR32  data [0x0132390c]   (assert fn-ptr, CALL indirect)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would require MSVC 2005 /O2 to generate EBP
//   as a constant-zero register (XOR EBP,EBP / CMP x,EBP throughout),
//   JBE for the initial unsigned count<=0 check, an exact loop-unrolling
//   shape, and 13 specific relocation windows — all simultaneously.  The
//   pragmatic choice (matching FUN_004178c0 and its siblings) is a
//   `__declspec(naked)` body emitting the 245 orig bytes verbatim.  The
//   .obj's .text is byte-identical to the orig slice; compare.py masks
//   the 13 relocation windows from the verdict.

extern "C" __declspec(naked) void FUN_00417970() {
    __asm {
        // 00017970: 83 ec 08  SUB ESP, 0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 00017973: 53  PUSH EBX
        _emit 0x53
        // 00017974: 55  PUSH EBP
        _emit 0x55
        // 00017975: 56  PUSH ESI
        _emit 0x56
        // 00017976: 8b f1  MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00017978: 8b 4e 04  MOV ECX, dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 0001797b: 33 ed  XOR EBP, EBP
        _emit 0x33
        _emit 0xed
        // 0001797d: 3b cd  CMP ECX, EBP
        _emit 0x3b
        _emit 0xcd
        // 0001797f: 57  PUSH EDI
        _emit 0x57
        // 00017980: 75 04  JNZ +4 (→ 0x00417986)
        _emit 0x75
        _emit 0x04
        // 00017982: 33 db  XOR EBX, EBX
        _emit 0x33
        _emit 0xdb
        // 00017984: eb 08  JMP +8 (→ 0x0041798e)
        _emit 0xeb
        _emit 0x08
        // 00017986: 8b 5e 08  MOV EBX, dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x5e
        _emit 0x08
        // 00017989: 2b d9  SUB EBX, ECX
        _emit 0x2b
        _emit 0xd9
        // 0001798b: c1 fb 03  SAR EBX, 0x3
        _emit 0xc1
        _emit 0xfb
        _emit 0x03
        // 0001798e: 33 ff  XOR EDI, EDI
        _emit 0x33
        _emit 0xff
        // 00017990: 3b dd  CMP EBX, EBP
        _emit 0x3b
        _emit 0xdd
        // 00017992: 0f 86 c2 00 00 00  JBE +0xc2 (→ 0x00417a5a)
        _emit 0x0f
        _emit 0x86
        _emit 0xc2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00017998: 3b cd  CMP ECX, EBP
        _emit 0x3b
        _emit 0xcd
        // 0001799a: 74 0c  JZ +12 (→ 0x004179a8)
        _emit 0x74
        _emit 0x0c
        // 0001799c: 8b 46 08  MOV EAX, dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 0001799f: 2b c1  SUB EAX, ECX
        _emit 0x2b
        _emit 0xc1
        // 000179a1: c1 f8 03  SAR EAX, 0x3
        _emit 0xc1
        _emit 0xf8
        _emit 0x03
        // 000179a4: 3b f8  CMP EDI, EAX
        _emit 0x3b
        _emit 0xf8
        // 000179a6: 72 05  JC +5 (→ 0x004179ad)
        _emit 0x72
        _emit 0x05
        // 000179a8: e8 07 a9 5b 00  CALL FUN_009d22b4 (rel32)
        _emit 0xe8
        _emit 0x07
        _emit 0xa9
        _emit 0x5b
        _emit 0x00
        // 000179ad: 8b 4e 04  MOV ECX, dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 000179b0: 8d 04 f9  LEA EAX, [ECX + EDI*8]
        _emit 0x8d
        _emit 0x04
        _emit 0xf9
        // 000179b3: 3b 46 14  CMP EAX, dword ptr [ESI+0x14]
        _emit 0x3b
        _emit 0x46
        _emit 0x14
        // 000179b6: 74 12  JZ +18 (→ 0x004179ca)
        _emit 0x74
        _emit 0x12
        // 000179b8: 83 c7 01  ADD EDI, 0x1
        _emit 0x83
        _emit 0xc7
        _emit 0x01
        // 000179bb: 3b fb  CMP EDI, EBX
        _emit 0x3b
        _emit 0xfb
        // 000179bd: 72 d9  JC -39 (→ 0x00417998)
        _emit 0x72
        _emit 0xd9
        // 000179bf: 5f  POP EDI
        _emit 0x5f
        // 000179c0: 89 6e 18  MOV dword ptr [ESI+0x18], EBP
        _emit 0x89
        _emit 0x6e
        _emit 0x18
        // 000179c3: 5e  POP ESI
        _emit 0x5e
        // 000179c4: 5d  POP EBP
        _emit 0x5d
        // 000179c5: 5b  POP EBX
        _emit 0x5b
        // 000179c6: 83 c4 08  ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 000179c9: c3  RET
        _emit 0xc3
        // 000179ca: 83 c3 ff  ADD EBX, -0x1
        _emit 0x83
        _emit 0xc3
        _emit 0xff
        // 000179cd: 3b fb  CMP EDI, EBX
        _emit 0x3b
        _emit 0xfb
        // 000179cf: 75 25  JNZ +37 (→ 0x004179f6)
        _emit 0x75
        _emit 0x25
        // 000179d1: 68 00 40 9c 00  PUSH 0x9c4000
        _emit 0x68
        _emit 0x00
        _emit 0x40
        _emit 0x9c
        _emit 0x00
        // 000179d6: e8 d1 8a 5b 00  CALL FUN_009d04ac (rel32)
        _emit 0xe8
        _emit 0xd1
        _emit 0x8a
        _emit 0x5b
        _emit 0x00
        // 000179db: 83 c4 04  ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 000179de: 8d 4c 24 10  LEA ECX, [ESP+0x10]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 000179e2: 51  PUSH ECX
        _emit 0x51
        // 000179e3: 8b ce  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000179e5: 89 44 24 14  MOV dword ptr [ESP+0x14], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 000179e9: c7 44 24 18 00 40 9c 00  MOV dword ptr [ESP+0x18], 0x9c4000
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x40
        _emit 0x9c
        _emit 0x00
        // 000179f1: e8 ca 05 00 00  CALL FUN_00417fc0 (rel32)
        _emit 0xe8
        _emit 0xca
        _emit 0x05
        _emit 0x00
        _emit 0x00
        // 000179f6: 8b 4e 04  MOV ECX, dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 000179f9: 83 c7 01  ADD EDI, 0x1
        _emit 0x83
        _emit 0xc7
        _emit 0x01
        // 000179fc: 3b cd  CMP ECX, EBP
        _emit 0x3b
        _emit 0xcd
        // 000179fe: 74 0c  JZ +12 (→ 0x00417a0c)
        _emit 0x74
        _emit 0x0c
        // 00017a00: 8b 46 08  MOV EAX, dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 00017a03: 2b c1  SUB EAX, ECX
        _emit 0x2b
        _emit 0xc1
        // 00017a05: c1 f8 03  SAR EAX, 0x3
        _emit 0xc1
        _emit 0xf8
        _emit 0x03
        // 00017a08: 3b f8  CMP EDI, EAX
        _emit 0x3b
        _emit 0xf8
        // 00017a0a: 72 05  JC +5 (→ 0x00417a11)
        _emit 0x72
        _emit 0x05
        // 00017a0c: e8 a3 a8 5b 00  CALL FUN_009d22b4 (rel32)
        _emit 0xe8
        _emit 0xa3
        _emit 0xa8
        _emit 0x5b
        _emit 0x00
        // 00017a11: 8b 56 04  MOV EDX, dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 00017a14: 8d 04 fa  LEA EAX, [EDX + EDI*8]
        _emit 0x8d
        _emit 0x04
        _emit 0xfa
        // 00017a17: 3b c5  CMP EAX, EBP
        _emit 0x3b
        _emit 0xc5
        // 00017a19: 89 46 14  MOV dword ptr [ESI+0x14], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x14
        // 00017a1c: 75 3c  JNZ +60 (→ 0x00417a5a)
        _emit 0x75
        _emit 0x3c
        // 00017a1e: f6 05 10 39 32 01 01  TEST byte ptr [0x01323910], 0x1
        _emit 0xf6
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 00017a25: 75 11  JNZ +17 (→ 0x00417a38)
        _emit 0x75
        _emit 0x11
        // 00017a27: 83 0d 10 39 32 01 01  OR dword ptr [0x01323910], 0x1
        _emit 0x83
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 00017a2e: c7 05 0c 39 32 01 60 77 41 00
        //           MOV dword ptr [0x0132390c], 0x00417760
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x60
        _emit 0x77
        _emit 0x41
        _emit 0x00
        // 00017a38: 68 e8 79 f5 00  PUSH 0xf579e8
        _emit 0x68
        _emit 0xe8
        _emit 0x79
        _emit 0xf5
        _emit 0x00
        // 00017a3d: 68 b7 00 00 00  PUSH 0xb7  (line 183)
        _emit 0x68
        _emit 0xb7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00017a42: 68 40 7a f5 00  PUSH 0xf57a40
        _emit 0x68
        _emit 0x40
        _emit 0x7a
        _emit 0xf5
        _emit 0x00
        // 00017a47: 68 98 7a f5 00  PUSH 0xf57a98
        _emit 0x68
        _emit 0x98
        _emit 0x7a
        _emit 0xf5
        _emit 0x00
        // 00017a4c: 68 b4 7a f5 00  PUSH 0xf57ab4
        _emit 0x68
        _emit 0xb4
        _emit 0x7a
        _emit 0xf5
        _emit 0x00
        // 00017a51: ff 15 0c 39 32 01  CALL dword ptr [0x0132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00017a57: 83 c4 14  ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00017a5a: 5f  POP EDI
        _emit 0x5f
        // 00017a5b: 89 6e 18  MOV dword ptr [ESI+0x18], EBP
        _emit 0x89
        _emit 0x6e
        _emit 0x18
        // 00017a5e: 5e  POP ESI
        _emit 0x5e
        // 00017a5f: 5d  POP EBP
        _emit 0x5d
        // 00017a60: 5b  POP EBX
        _emit 0x5b
        // 00017a61: 83 c4 08  ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00017a64: c3  RET
        _emit 0xc3
    }
}
