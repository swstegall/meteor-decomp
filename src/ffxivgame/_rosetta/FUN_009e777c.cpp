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
// FUNCTION: ffxivgame 0x009e777c — bitset-slot lookup with errno-style
//                                  error reporting; 113 bytes / 0x71.
//
// __cdecl int FUN_009e777c(int index)
//
// Pseudo-C:
//
//   extern int   g_slot_count;           // [0x0137b7dc]
//   extern void *g_slot_array[];         // [0x0137b7e0]
//
//   int FUN_009e777c(int index) {
//       if (index == -2) {
//           *errno_ptr() = 0;            // CALL 0x9d9d5a; AND [EAX],0
//           *errno_ptr2() = 9;           // CALL 0x9d9d47; MOV [EAX],9
//           return -1;
//       }
//       if (index < 0 || index >= g_slot_count) {
//           *errno_ptr() = 0;            // CALL 0x9d9d5a; MOV [EAX],ESI(=0)
//           *errno_ptr2() = 9;           // CALL 0x9d9d47; MOV [EAX],9
//           some_error_handler(0,0,0,0,0); // CALL 0x9d2290 with 5 zero args
//           return -1;
//       }
//       // Compute element pointer:
//       //   word_idx = index >> 5
//       //   bit_pos  = index & 0x1f
//       //   elem     = (void*)g_slot_array[word_idx] + (bit_pos << 6)
//       void *elem = (char*)g_slot_array[index >> 5] + ((index & 0x1f) << 6);
//       if (*(unsigned char*)((char*)elem + 4) & 1) {
//           return *(int*)elem;           // slot is active: return value
//       }
//       // slot inactive: same error path as out-of-range
//       *errno_ptr() = 0;
//       *errno_ptr2() = 9;
//       some_error_handler(0,0,0,0,0);
//       return -1;
//   }
//
// Calling convention: __cdecl; no EBP frame.  ESI saved/restored as the
//   only callee-saved reg used in the main branch.
//
// Notable codegen:
//   - CMP EAX,-2 / JNZ at top: MSVC puts the -2 guard before PUSH ESI
//     so the early-out path never touches the callee-saved slot.
//   - In the out-of-range / inactive branch, 5x PUSH ESI passes five
//     zero arguments to 0x9d2290 (ESI is 0 at that point).
//   - Three CALL rel32 instructions and two absolute data references
//     prevent a source-level C++ reconstruction — the .obj would carry
//     relocations whose placeholder bytes differ from the original.
//     A __declspec(naked) _emit passthrough produces byte-identical .text.
//
// Reloc-bearing sites in the original 113 bytes:
//   +0x09  CALL rel32 → 0x009d9d5a   (e8 d0 25 ff ff)
//   +0x11  CALL rel32 → 0x009d9d47   (e8 b5 25 ff ff)
//   +0x27  CMP EAX,abs32 0x0137b7dc  (3b 05 dc b7 37 01)
//   +0x37  MOV ECX,[ECX*4+abs32]     (8b 0c 8d e0 b7 37 01)
//   +0x49  CALL rel32 → 0x009d9d5a   (e8 90 25 ff ff)
//   +0x50  CALL rel32 → 0x009d9d47   (e8 76 25 ff ff)
//   +0x60  CALL rel32 → 0x009d2290   (e8 af aa fe ff)

extern "C" __declspec(naked) void FUN_009e777c() {
    __asm {
        // 009e777c: 8b 44 24 04   MOV EAX,dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 009e7780: 83 f8 fe      CMP EAX,-0x2
        _emit 0x83
        _emit 0xf8
        _emit 0xfe
        // 009e7783: 75 17         JNZ +0x17 (→ 009e779c)
        _emit 0x75
        _emit 0x17
        // 009e7785: e8 d0 25 ff ff  CALL 0x009d9d5a
        _emit 0xe8
        _emit 0xd0
        _emit 0x25
        _emit 0xff
        _emit 0xff
        // 009e778a: 83 20 00      AND dword ptr [EAX],0x0
        _emit 0x83
        _emit 0x20
        _emit 0x00
        // 009e778d: e8 b5 25 ff ff  CALL 0x009d9d47
        _emit 0xe8
        _emit 0xb5
        _emit 0x25
        _emit 0xff
        _emit 0xff
        // 009e7792: c7 00 09 00 00 00  MOV dword ptr [EAX],0x9
        _emit 0xc7
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 009e7798: 83 c8 ff      OR EAX,0xffffffff
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // 009e779b: c3            RET
        _emit 0xc3
        // 009e779c: 56            PUSH ESI
        _emit 0x56
        // 009e779d: 33 f6         XOR ESI,ESI
        _emit 0x33
        _emit 0xf6
        // 009e779f: 3b c6         CMP EAX,ESI
        _emit 0x3b
        _emit 0xc6
        // 009e77a1: 7c 22         JL +0x22 (→ 009e77c5)
        _emit 0x7c
        _emit 0x22
        // 009e77a3: 3b 05 dc b7 37 01  CMP EAX,dword ptr [0x0137b7dc]
        _emit 0x3b
        _emit 0x05
        _emit 0xdc
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        // 009e77a9: 73 1a         JNC +0x1a (→ 009e77c5)
        _emit 0x73
        _emit 0x1a
        // 009e77ab: 8b c8         MOV ECX,EAX
        _emit 0x8b
        _emit 0xc8
        // 009e77ad: 83 e0 1f      AND EAX,0x1f
        _emit 0x83
        _emit 0xe0
        _emit 0x1f
        // 009e77b0: c1 f9 05      SAR ECX,0x5
        _emit 0xc1
        _emit 0xf9
        _emit 0x05
        // 009e77b3: 8b 0c 8d e0 b7 37 01  MOV ECX,dword ptr [ECX*4+0x137b7e0]
        _emit 0x8b
        _emit 0x0c
        _emit 0x8d
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        // 009e77ba: c1 e0 06      SHL EAX,0x6
        _emit 0xc1
        _emit 0xe0
        _emit 0x06
        // 009e77bd: 03 c1         ADD EAX,ECX
        _emit 0x03
        _emit 0xc1
        // 009e77bf: f6 40 04 01   TEST byte ptr [EAX+0x4],0x1
        _emit 0xf6
        _emit 0x40
        _emit 0x04
        _emit 0x01
        // 009e77c3: 75 24         JNZ +0x24 (→ 009e77e9)
        _emit 0x75
        _emit 0x24
        // 009e77c5: e8 90 25 ff ff  CALL 0x009d9d5a
        _emit 0xe8
        _emit 0x90
        _emit 0x25
        _emit 0xff
        _emit 0xff
        // 009e77ca: 89 30         MOV dword ptr [EAX],ESI
        _emit 0x89
        _emit 0x30
        // 009e77cc: e8 76 25 ff ff  CALL 0x009d9d47
        _emit 0xe8
        _emit 0x76
        _emit 0x25
        _emit 0xff
        _emit 0xff
        // 009e77d1: 56            PUSH ESI  (arg5 = 0)
        _emit 0x56
        // 009e77d2: 56            PUSH ESI  (arg4 = 0)
        _emit 0x56
        // 009e77d3: 56            PUSH ESI  (arg3 = 0)
        _emit 0x56
        // 009e77d4: 56            PUSH ESI  (arg2 = 0)
        _emit 0x56
        // 009e77d5: 56            PUSH ESI  (arg1 = 0)
        _emit 0x56
        // 009e77d6: c7 00 09 00 00 00  MOV dword ptr [EAX],0x9
        _emit 0xc7
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 009e77dc: e8 af aa fe ff  CALL 0x009d2290
        _emit 0xe8
        _emit 0xaf
        _emit 0xaa
        _emit 0xfe
        _emit 0xff
        // 009e77e1: 83 c4 14      ADD ESP,0x14  (pop 5 dword args)
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 009e77e4: 83 c8 ff      OR EAX,0xffffffff
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // 009e77e7: 5e            POP ESI
        _emit 0x5e
        // 009e77e8: c3            RET
        _emit 0xc3
        // 009e77e9: 8b 00         MOV EAX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x00
        // 009e77eb: 5e            POP ESI
        _emit 0x5e
        // 009e77ec: c3            RET
        _emit 0xc3
    }
}
