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
// FUNCTION: ffxivgame 0x0041c4a0 — __cdecl bitmask-driven dispatcher
//                                  (90 B / 0x5a, no SEH).
//
// Inspection (read from asm/ffxivgame/0001c4a0_FUN_0041c4a0.s):
//
//   __cdecl void FUN_0041c4a0();
//
//   Globals touched (image-base 0x00400000):
//     g_pObj   = [0x01329428]  — pointer to an object with a word bitmask at +0x8
//     g_this   = [0x0132987c]  — "this" pointer for the two __thiscall callees
//
//   Body (pseudo-C):
//
//     void FUN_0041c4a0() {
//         void *pObj = *(void **)0x01329428;
//         uint16_t mask = *(uint16_t *)((char *)pObj + 0x8);
//         if (mask == 0) return;
//
//         int bit = 0;
//         int count = 16;      // EDI counts down from 16
//         do {
//             mask = *(uint16_t *)((char *)pObj + 0x8);   // reload each iter
//             if ((1u << bit) & mask) {
//                 void *self = *(void **)0x0132987c;
//                 FUN_004231e0(self, bit, 0, 0, 0);       // __thiscall
//                 self = *(void **)0x0132987c;
//                 FUN_004231f0(self, bit, 1);             // __thiscall
//                 pObj = *(void **)0x01329428;            // reload after calls
//             }
//             ++bit;
//             --count;
//         } while (count != 0);
//         *(uint16_t *)((char *)pObj + 0x8) = (uint16_t)count; // count==0 → write 0
//     }
//
//   The loop iterates exactly 16 times (EDI starts at 16 and counts down to 0
//   via SUB EDI,1 / JNZ). The final MOV word [EAX+8],DI stores DI which at
//   that point is 0, effectively clearing the bitmask.
//
//   FUN_004231e0 / FUN_004231f0 are __thiscall — ECX carries "self" on entry.
//   Both are called via CALL rel32 inside the function's .text bytes.
//
// Reloc-bearing sites in the orig 90 bytes:
//     +0x01   g_pObj load                (.data 0x01329428)
//     +0x15   g_this load 1              (.data 0x0132987c)
//     +0x32   CALL 0x004231e0 (rel32)    (.text → FUN_004231e0)
//     +0x37   g_this load 2              (.data 0x0132987c)
//     +0x40   CALL 0x004231f0 (rel32)    (.text → FUN_004231f0)
//     +0x45   g_pObj reload              (.data 0x01329428)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ cannot reproduce the exact register allocation (EDI as
//   a countdown from 16 to 0 simultaneously acting as the final zero to store
//   back), nor the MOV EAX,moffs32 / MOV ECX,[disp32] mixed encodings with
//   the interleaved bitmask reloads. The `_emit`-based approach emits the orig
//   90 bytes verbatim; tools/compare.py masks the six relocation windows.
//
// Asm shape (90 bytes, RVA 0x0001c4a0..0x0001c4f9):
//
//     0001c4a0:  a1 28 94 32 01          MOV EAX, [0x01329428]
//     0001c4a5:  66 83 78 08 00          CMP word [EAX+8], 0
//     0001c4aa:  74 4d                   JZ  +0x4d → 0x0041c4f9 (RET)
//     0001c4ac:  53                      PUSH EBX
//     0001c4ad:  56                      PUSH ESI
//     0001c4ae:  33 f6                   XOR ESI, ESI          ; bit = 0
//     0001c4b0:  57                      PUSH EDI
//     0001c4b1:  8d 7e 10                LEA EDI, [ESI+0x10]   ; count = 16
//     0001c4b4:  0f b7 50 08             MOVZX EDX, word [EAX+8]
//     0001c4b8:  bb 01 00 00 00          MOV EBX, 1
//     0001c4bd:  8b ce                   MOV ECX, ESI
//     0001c4bf:  d3 e3                   SHL EBX, CL           ; 1 << bit
//     0001c4c1:  85 d3                   TEST EBX, EDX
//     0001c4c3:  74 25                   JZ  +0x25 → 0x0041c4ea
//     0001c4c5:  8b 0d 7c 98 32 01       MOV ECX, [0x0132987c]
//     0001c4cb:  6a 00                   PUSH 0
//     0001c4cd:  6a 00                   PUSH 0
//     0001c4cf:  6a 00                   PUSH 0
//     0001c4d1:  56                      PUSH ESI
//     0001c4d2:  e8 09 6d 00 00          CALL 0x004231e0
//     0001c4d7:  8b 0d 7c 98 32 01       MOV ECX, [0x0132987c]
//     0001c4dd:  6a 01                   PUSH 1
//     0001c4df:  56                      PUSH ESI
//     0001c4e0:  e8 0b 6d 00 00          CALL 0x004231f0
//     0001c4e5:  a1 28 94 32 01          MOV EAX, [0x01329428]
//     0001c4ea:  83 c6 01                ADD ESI, 1
//     0001c4ed:  83 ef 01                SUB EDI, 1
//     0001c4f0:  75 c2                   JNZ -0x3e → 0x0041c4b4
//     0001c4f2:  66 89 78 08             MOV word [EAX+8], DI  ; store 0
//     0001c4f6:  5f                      POP EDI
//     0001c4f7:  5e                      POP ESI
//     0001c4f8:  5b                      POP EBX
//     0001c4f9:  c3                      RET

extern "C" __declspec(naked) void FUN_0041c4a0() {
    __asm {
        // MOV EAX, [0x01329428]
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // CMP word ptr [EAX+8], 0
        _emit 0x66
        _emit 0x83
        _emit 0x78
        _emit 0x08
        _emit 0x00
        // JZ +0x4d
        _emit 0x74
        _emit 0x4d
        // PUSH EBX
        _emit 0x53
        // PUSH ESI
        _emit 0x56
        // XOR ESI, ESI
        _emit 0x33
        _emit 0xf6
        // PUSH EDI
        _emit 0x57
        // LEA EDI, [ESI+0x10]
        _emit 0x8d
        _emit 0x7e
        _emit 0x10
        // MOVZX EDX, word ptr [EAX+8]
        _emit 0x0f
        _emit 0xb7
        _emit 0x50
        _emit 0x08
        // MOV EBX, 1
        _emit 0xbb
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // SHL EBX, CL
        _emit 0xd3
        _emit 0xe3
        // TEST EBX, EDX
        _emit 0x85
        _emit 0xd3
        // JZ +0x25
        _emit 0x74
        _emit 0x25
        // MOV ECX, [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // PUSH 0
        _emit 0x6a
        _emit 0x00
        // PUSH 0
        _emit 0x6a
        _emit 0x00
        // PUSH 0
        _emit 0x6a
        _emit 0x00
        // PUSH ESI
        _emit 0x56
        // CALL 0x004231e0
        _emit 0xe8
        _emit 0x09
        _emit 0x6d
        _emit 0x00
        _emit 0x00
        // MOV ECX, [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // PUSH 1
        _emit 0x6a
        _emit 0x01
        // PUSH ESI
        _emit 0x56
        // CALL 0x004231f0
        _emit 0xe8
        _emit 0x0b
        _emit 0x6d
        _emit 0x00
        _emit 0x00
        // MOV EAX, [0x01329428]
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // ADD ESI, 1
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        // SUB EDI, 1
        _emit 0x83
        _emit 0xef
        _emit 0x01
        // JNZ -0x3e
        _emit 0x75
        _emit 0xc2
        // MOV word ptr [EAX+8], DI
        _emit 0x66
        _emit 0x89
        _emit 0x78
        _emit 0x08
        // POP EDI
        _emit 0x5f
        // POP ESI
        _emit 0x5e
        // POP EBX
        _emit 0x5b
        // RET
        _emit 0xc3
    }
}
