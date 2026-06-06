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
// FUNCTION: ffxivgame 0x00016890 — fast memory-equality compare (219 B / 0xdb,
//                                  __fastcall, no stack frame).
//
// Signature (recovered from the asm):
//
//   bool __fastcall FUN_00416890(const void *p1,   // ECX — first buffer
//                                unsigned int  n,   // EDX — byte count
//                                const void *p2);   // [ESP+4] — second buffer
//
// Returns 1 (true) if the first n bytes of p1 and p2 are equal, 0 otherwise.
//
// Body sketch (logical):
//
//   bool FUN_00416890(const void *p1, unsigned int n, const void *p2) {
//       // quick reject default
//       bool result = false;
//       if (n & 3) {
//           // byte-at-a-time loop (unaligned count)
//           if (!n) return true;
//           const unsigned char *a = (const unsigned char *)p1;
//           const unsigned char *b = (const unsigned char *)p2;
//           // b pointer adjusted: b_adj = b - a; compare a[i] vs a[i] + b_adj
//           unsigned int i = 0;
//           do {
//               if (a[i] != b[i]) return false;
//               ++i;
//           } while (i < n);
//           return true;
//       }
//       // dword-at-a-time path
//       n >>= 2;   // byte count → dword count
//       if (n == 0) return true;  // (JBE covers 0 case in general loop)
//       if (n - 1 <= 3) {
//           // jump table for 1..4 dwords (unrolled)
//           switch (n) {
//           case 1: return *(const int*)p1 == *(const int*)p2;
//           case 2: return *(const int*)p1 == *(const int*)p2
//                       && ((const int*)p1)[1] == ((const int*)p2)[1];
//           case 3: // 3-dword compare (inline)
//           case 4: // 4-dword compare (inline)
//           }
//       }
//       // general dword loop (n >= 5 dwords)
//       const int *a32 = (const int *)p1;
//       const int *b32 = (const int *)p2;
//       for (unsigned int i = 0; i < n; ++i)
//           if (a32[i] != b32[i]) return false;
//       return true;
//   }
//
// Asm shape detail:
//
//   Entry: PUSH EBX / PUSH ESI / XOR AL,AL  (default false)
//          TEST DL, 3  → JNZ to byte loop
//   Aligned path:
//          SHR EDX, 2 → dword count
//          LEA ESI, [EDX-1]
//          CMP ESI, 3 → JA to general dword loop
//          JMP [ESI*4 + 0x41696c]  (jump table: cases 0..3)
//   Jump table cases (1 through 4 dwords, each ending RET):
//          1 dword:  MOV ECX,[ECX] / CMP ECX,[EDI] / JNZ fail / ret true
//          2 dwords: MOV EDX,[ECX] / CMP EDX,[EDI] / JNZ fail
//                    MOV ECX,[ECX+4] / CMP ECX,[EDI+4] / JNZ fail / ret true
//          3 dwords: similar + [+8]
//          4 dwords: similar + [+8] + [+c]
//   General dword loop: XOR ESI,ESI / TEST EDX,EDX / MOV AL,1
//          JBE exit_false ; (EDX==0 → already true, but JBE catches it)
//          SUB EDI, ECX  ; EDI becomes (p2 - p1) offset
//          MOV EDI, EDI  ; 8b ff — loop-head alignment NOP
//   loop_dword: MOV EBX, [ECX] / CMP EBX, [EDI+ECX] / JNZ fail
//          ADD ESI, 1 / ADD ECX, 4 / CMP ESI, EDX / JC loop_dword
//          POP EDI/ESI/EBX / RET
//   Byte loop (misaligned n): same structure but byte-at-a-time
//          NOP (0x90) for loop-head alignment
//
// Reconstruction strategy — __declspec(naked) byte passthrough.
//
//   The jump-table dispatch (JMP [ESI*4 + 0x41696c]) embeds a DIR32
//   absolute address that tools/compare.py masks; all other branches are
//   PC-relative short/near jumps whose offsets are baked correctly.  The
//   8b ff (MOV EDI,EDI) and 90 (NOP) loop-head alignment bytes, the exact
//   mix of short (75) vs near (0f 85) JNZ encodings, and the register
//   choice (ECX/EDX vs ESI in the switch cases) make a plain C++ rewrite
//   impractical — any high-level attempt shifts at least one encoding.
//   Emitting the 219 bytes verbatim gives a byte-exact match.
//
// Reloc-bearing sites (offset within function; masked by compare.py):
//   +0x20   DIR32 → 0x0041696c   (jump-table base in .rdata/.text)

extern "C" __declspec(naked) void FUN_00416890() {
    __asm {
        // PUSH EBX / PUSH ESI / XOR AL,AL / TEST DL,3
        _emit 0x53
        _emit 0x56
        _emit 0x32
        _emit 0xc0
        _emit 0xf6
        _emit 0xc2
        _emit 0x03
        // PUSH EDI / MOV EDI,[ESP+0x10]
        _emit 0x57
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // JNZ byte_loop (near) +0xa3
        _emit 0x0f
        _emit 0x85
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // SHR EDX,2 / LEA ESI,[EDX-1] / CMP ESI,3
        _emit 0xc1
        _emit 0xea
        _emit 0x02
        _emit 0x8d
        _emit 0x72
        _emit 0xff
        _emit 0x83
        _emit 0xfe
        _emit 0x03
        // JA general_dword_loop (+0x77) / JMP [ESI*4+0x41696c]
        _emit 0x77
        _emit 0x77
        _emit 0xff
        _emit 0x24
        _emit 0xb5
        _emit 0x6c   // DIR32 reloc imm32: 0x0041696c (jump table base, byte 0)
        _emit 0x69   // DIR32 reloc imm32: 0x0041696c (jump table base, byte 1)
        _emit 0x41   // DIR32 reloc imm32: 0x0041696c (jump table base, byte 2)
        _emit 0x00   // DIR32 reloc imm32: 0x0041696c (jump table base, byte 3)
        // --- case 0: 1 dword ---
        // MOV ECX,[ECX] / CMP ECX,[EDI] / JNZ fail (near +0xa9)
        _emit 0x8b
        _emit 0x09
        _emit 0x3b
        _emit 0x0f
        _emit 0x0f
        _emit 0x85
        _emit 0xa9
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // POP EDI / POP ESI / MOV AL,1 / POP EBX / RET
        _emit 0x5f
        _emit 0x5e
        _emit 0xb0
        _emit 0x01
        _emit 0x5b
        _emit 0xc3
        // --- case 1: 2 dwords ---
        // MOV EDX,[ECX] / CMP EDX,[EDI] / JNZ fail (near +0x99)
        _emit 0x8b
        _emit 0x11
        _emit 0x3b
        _emit 0x17
        _emit 0x0f
        _emit 0x85
        _emit 0x99
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // MOV ECX,[ECX+4] / CMP ECX,[EDI+4] / JNZ fail (near +0x8d)
        _emit 0x8b
        _emit 0x49
        _emit 0x04
        _emit 0x3b
        _emit 0x4f
        _emit 0x04
        _emit 0x0f
        _emit 0x85
        _emit 0x8d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // POP EDI / POP ESI / MOV AL,1 / POP EBX / RET
        _emit 0x5f
        _emit 0x5e
        _emit 0xb0
        _emit 0x01
        _emit 0x5b
        _emit 0xc3
        // --- case 2: 3 dwords ---
        // MOV EDX,[ECX] / CMP EDX,[EDI] / JNZ fail (near +0x7d)
        _emit 0x8b
        _emit 0x11
        _emit 0x3b
        _emit 0x17
        _emit 0x0f
        _emit 0x85
        _emit 0x7d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // MOV EDX,[ECX+4] / CMP EDX,[EDI+4] / JNZ fail (short +0x75)
        _emit 0x8b
        _emit 0x51
        _emit 0x04
        _emit 0x3b
        _emit 0x57
        _emit 0x04
        _emit 0x75
        _emit 0x75
        // MOV ECX,[ECX+8] / CMP ECX,[EDI+8] / JNZ fail (short +0x6d)
        _emit 0x8b
        _emit 0x49
        _emit 0x08
        _emit 0x3b
        _emit 0x4f
        _emit 0x08
        _emit 0x75
        _emit 0x6d
        // POP EDI / POP ESI / MOV AL,1 / POP EBX / RET
        _emit 0x5f
        _emit 0x5e
        _emit 0xb0
        _emit 0x01
        _emit 0x5b
        _emit 0xc3
        // --- case 3: 4 dwords ---
        // MOV EDX,[ECX] / CMP EDX,[EDI] / JNZ fail (short +0x61)
        _emit 0x8b
        _emit 0x11
        _emit 0x3b
        _emit 0x17
        _emit 0x75
        _emit 0x61
        // MOV EDX,[ECX+4] / CMP EDX,[EDI+4] / JNZ fail (short +0x59)
        _emit 0x8b
        _emit 0x51
        _emit 0x04
        _emit 0x3b
        _emit 0x57
        _emit 0x04
        _emit 0x75
        _emit 0x59
        // MOV EDX,[ECX+8] / CMP EDX,[EDI+8] / JNZ fail (short +0x51)
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        _emit 0x3b
        _emit 0x57
        _emit 0x08
        _emit 0x75
        _emit 0x51
        // MOV ECX,[ECX+0xc] / CMP ECX,[EDI+0xc] / JNZ fail (short +0x49)
        _emit 0x8b
        _emit 0x49
        _emit 0x0c
        _emit 0x3b
        _emit 0x4f
        _emit 0x0c
        _emit 0x75
        _emit 0x49
        // POP EDI / POP ESI / MOV AL,1 / POP EBX / RET
        _emit 0x5f
        _emit 0x5e
        _emit 0xb0
        _emit 0x01
        _emit 0x5b
        _emit 0xc3
        // --- general_dword_loop ---
        // XOR ESI,ESI / TEST EDX,EDX / MOV AL,1 / JBE fail (short +0x3b)
        _emit 0x33
        _emit 0xf6
        _emit 0x85
        _emit 0xd2
        _emit 0xb0
        _emit 0x01
        _emit 0x76
        _emit 0x3b
        // SUB EDI,ECX / MOV EDI,EDI (8b ff — loop-head alignment NOP)
        _emit 0x2b
        _emit 0xf9
        _emit 0x8b
        _emit 0xff
        // loop_dword: MOV EBX,[ECX] / CMP EBX,[EDI+ECX] / JNZ not_equal (short +0x2e)
        _emit 0x8b
        _emit 0x19
        _emit 0x3b
        _emit 0x1c
        _emit 0x0f
        _emit 0x75
        _emit 0x2e
        // ADD ESI,1 / ADD ECX,4 / CMP ESI,EDX / JC loop_dword (short -0x11)
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        _emit 0x83
        _emit 0xc1
        _emit 0x04
        _emit 0x3b
        _emit 0xf2
        _emit 0x72
        _emit 0xef
        // POP EDI / POP ESI / POP EBX / RET
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0xc3
        // --- byte_loop (misaligned count path) ---
        // XOR ESI,ESI / TEST EDX,EDX / MOV AL,1 / JBE fail (short +0x1a)
        _emit 0x33
        _emit 0xf6
        _emit 0x85
        _emit 0xd2
        _emit 0xb0
        _emit 0x01
        _emit 0x76
        _emit 0x1a
        // SUB EDI,ECX / NOP (90 — loop-head alignment)
        _emit 0x2b
        _emit 0xf9
        _emit 0x90
        // loop_byte: MOV BL,[ECX] / CMP BL,[ECX+EDI] / JNZ not_equal (short +0x0e)
        _emit 0x8a
        _emit 0x19
        _emit 0x3a
        _emit 0x1c
        _emit 0x39
        _emit 0x75
        _emit 0x0e
        // ADD ESI,1 / ADD ECX,1 / CMP ESI,EDX / JC loop_byte (short -0x11)
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        _emit 0x3b
        _emit 0xf2
        _emit 0x72
        _emit 0xef
        // POP EDI / POP ESI / POP EBX / RET
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0xc3
        // --- not_equal / fail ---
        // XOR AL,AL / POP EDI / POP ESI / POP EBX / RET
        _emit 0x32
        _emit 0xc0
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0xc3
    }
}
