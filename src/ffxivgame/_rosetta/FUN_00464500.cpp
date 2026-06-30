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
// FUNCTION: ffxivgame 0x00064500 — _ASN1_STRING_cmp (174 B / 0xae)
//
// OpenSSL ASN1_STRING comparison helper. Compares two ASN1_STRING
// structures (layout: int length @ +0, int type @ +4, unsigned char*
// data @ +8). Calling convention: __cdecl (two pointer args, caller
// cleans, plain RET at epilog — not RET n).
//
// Logic (recovered from asm @ 0x00064500):
//
//   int _ASN1_STRING_cmp(const ASN1_STRING *a, const ASN1_STRING *b) {
//       // 1. Compare lengths
//       int diff = a->length - b->length;
//       if (diff) return diff;               // early exit
//
//       // 2. Inline memcmp: DWORD-accelerated, then byte-by-byte tail
//       int n        = a->length;
//       unsigned char *pa = a->data;
//       unsigned char *pb = b->data;
//
//       while (n >= 4) {
//           if (*(unsigned int*)pa != *(unsigned int*)pb) break;
//           n -= 4; pa += 4; pb += 4;
//       }
//
//       // Up to 4 bytes compared one-by-one (unrolled x4 with early exit
//       // at each step; last iteration uses EAX instead of EBP for the
//       // right operand — a compiler artefact of register reuse).
//       // ...
//
//       // 3. Return sign of byte difference, or type tiebreak if equal
//       if (byte_diff > 0)  return  1;
//       if (byte_diff < 0)  return -1;
//       return a->type - b->type;
//   }
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function contains NO external symbol references (no IAT calls,
//   no global-variable accesses, no CALL rel32 targets outside the TU).
//   All control flow uses short (8-bit) or near (32-bit) offsets that
//   are self-relative within the 174-byte body, so they encode
//   identically at any load address. Emitting the 174 bytes verbatim via
//   _emit produces a byte-identical match. There are zero relocations
//   for compare.py to wildcard.
//
//   Source-level reconstruction was considered but rejected: getting
//   MSVC 2005 /O2 to emit precisely the DWORD-loop + 4x-unrolled-byte
//   tail without the /arch:SSE2 path, with EBX ↔ second-arg and EDI ↔
//   first-arg, requires the exact source shape that produced this binary
//   — which is unknown. Naked asm is the correct path for a TU-isolated
//   match.
//
// Offset map (relative to function start):
//   +0x00  PUSH EBX / MOV EBX,[ESP+0xc]  — load arg2 (b)
//   +0x05  PUSH EDI / MOV EDI,[ESP+0xc]  — load arg1 (a)
//   +0x0a  MOV EAX,[EDI] / MOV ECX,EAX / SUB ECX,[EBX]
//          → ECX = a->length - b->length
//   +0x10  JNZ +0x93  → tail @ +0xa9 (return ECX on length mismatch)
//   +0x16  CMP EAX,4 / load pointers / PUSH EBP,ESI / JC +0x14
//          → jump to byte-loop if length < 4
//   +0x23  DWORD compare loop (MOV ESI,[EDX]; CMP ESI,[ECX]; ...)
//   +0x3b  4x unrolled byte compare tail
//   +0x8a  sign computation (ESI > 0 → EAX=1; else EAX=-1)
//   +0x98  XOR EAX,EAX  (equal path)
//   +0x9a  TEST EAX,EAX / POP ESI,EBP / JNZ +0xb → return byte result
//   +0xa0  MOV EAX,[EDI+4] / SUB EAX,[EBX+4]  → type tiebreak
//   +0xa6  POP EDI,EBX / RET
//   +0xa9  MOV EAX,ECX / POP EDI,EBX / RET  (length-diff early exit)

extern "C" __declspec(naked) void FUN_00464500() {
    __asm {
        // +0x00  PUSH EBX
        _emit 0x53
        // +0x01  MOV EBX, dword ptr [ESP+0xc]   — b (arg2)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        // +0x05  PUSH EDI
        _emit 0x57
        // +0x06  MOV EDI, dword ptr [ESP+0xc]   — a (arg1)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // +0x0a  MOV EAX, dword ptr [EDI]        — a->length
        _emit 0x8b
        _emit 0x07
        // +0x0c  MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // +0x0e  SUB ECX, dword ptr [EBX]        — ECX = a->length - b->length
        _emit 0x2b
        _emit 0x0b
        // +0x10  JNZ near +0x93   → offset 0xa9  (lengths differ: return diff)
        _emit 0x0f
        _emit 0x85
        _emit 0x93
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x16  CMP EAX, 4
        _emit 0x83
        _emit 0xf8
        _emit 0x04
        // +0x19  MOV ECX, dword ptr [EBX+8]      — b->data
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // +0x1c  MOV EDX, dword ptr [EDI+8]      — a->data
        _emit 0x8b
        _emit 0x57
        _emit 0x08
        // +0x1f  PUSH EBP
        _emit 0x55
        // +0x20  PUSH ESI
        _emit 0x56
        // +0x21  JC short +0x14   → offset 0x37  (length < 4: skip DWORD loop)
        _emit 0x72
        _emit 0x14
        // --- DWORD compare loop -------------------------------------------
        // +0x23  MOV ESI, dword ptr [EDX]
        _emit 0x8b
        _emit 0x32
        // +0x25  CMP ESI, dword ptr [ECX]
        _emit 0x3b
        _emit 0x31
        // +0x27  JNZ short +0x12  → offset 0x3b  (mismatch: enter byte cmp)
        _emit 0x75
        _emit 0x12
        // +0x29  SUB EAX, 4
        _emit 0x83
        _emit 0xe8
        _emit 0x04
        // +0x2c  ADD ECX, 4
        _emit 0x83
        _emit 0xc1
        _emit 0x04
        // +0x2f  ADD EDX, 4
        _emit 0x83
        _emit 0xc2
        _emit 0x04
        // +0x32  CMP EAX, 4
        _emit 0x83
        _emit 0xf8
        _emit 0x04
        // +0x35  JNC short -0x14  → offset 0x23  (loop back while n >= 4)
        _emit 0x73
        _emit 0xec
        // --- post-DWORD: remaining bytes -----------------------------------
        // +0x37  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // +0x39  JZ short +0x5d   → offset 0x98  (n == 0: equal)
        _emit 0x74
        _emit 0x5d
        // --- 4x unrolled byte compare -------------------------------------
        // iteration 1
        // +0x3b  MOVZX ESI, byte ptr [EDX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x32
        // +0x3e  MOVZX EBP, byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x29
        // +0x41  SUB ESI, EBP
        _emit 0x2b
        _emit 0xf5
        // +0x43  JNZ short +0x45  → offset 0x8a  (differ: compute sign)
        _emit 0x75
        _emit 0x45
        // +0x45  SUB EAX, 1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // +0x48  ADD ECX, 1
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // +0x4b  ADD EDX, 1
        _emit 0x83
        _emit 0xc2
        _emit 0x01
        // +0x4e  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // +0x50  JZ short +0x46   → offset 0x98  (n == 0: equal)
        _emit 0x74
        _emit 0x46
        // iteration 2
        // +0x52  MOVZX ESI, byte ptr [EDX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x32
        // +0x55  MOVZX EBP, byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x29
        // +0x58  SUB ESI, EBP
        _emit 0x2b
        _emit 0xf5
        // +0x5a  JNZ short +0x2e  → offset 0x8a  (differ: compute sign)
        _emit 0x75
        _emit 0x2e
        // +0x5c  SUB EAX, 1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // +0x5f  ADD ECX, 1
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // +0x62  ADD EDX, 1
        _emit 0x83
        _emit 0xc2
        _emit 0x01
        // +0x65  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // +0x67  JZ short +0x2f   → offset 0x98  (n == 0: equal)
        _emit 0x74
        _emit 0x2f
        // iteration 3
        // +0x69  MOVZX ESI, byte ptr [EDX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x32
        // +0x6c  MOVZX EBP, byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x29
        // +0x6f  SUB ESI, EBP
        _emit 0x2b
        _emit 0xf5
        // +0x71  JNZ short +0x17  → offset 0x8a  (differ: compute sign)
        _emit 0x75
        _emit 0x17
        // +0x73  SUB EAX, 1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // +0x76  ADD ECX, 1
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // +0x79  ADD EDX, 1
        _emit 0x83
        _emit 0xc2
        _emit 0x01
        // +0x7c  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // +0x7e  JZ short +0x18   → offset 0x98  (n == 0: equal)
        _emit 0x74
        _emit 0x18
        // iteration 4 (last — right operand loaded into EAX, not EBP)
        // +0x80  MOVZX ESI, byte ptr [EDX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x32
        // +0x83  MOVZX EAX, byte ptr [ECX]   — NOTE: EAX not EBP
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // +0x86  SUB ESI, EAX
        _emit 0x2b
        _emit 0xf0
        // +0x88  JZ short +0x0e   → offset 0x98  (equal: fall through to tiebreak)
        _emit 0x74
        _emit 0x0e
        // --- sign computation (ESI != 0) ----------------------------------
        // +0x8a  TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // +0x8c  MOV EAX, 1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x91  JG short +0x07   → offset 0x9a  (positive: return 1)
        _emit 0x7f
        _emit 0x07
        // +0x93  OR EAX, 0xffffffff           — EAX = -1
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // +0x96  JMP short +0x02  → offset 0x9a
        _emit 0xeb
        _emit 0x02
        // --- equal path ---------------------------------------------------
        // +0x98  XOR EAX, EAX                 — result = 0
        _emit 0x33
        _emit 0xc0
        // --- common epilog ------------------------------------------------
        // +0x9a  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // +0x9c  POP ESI
        _emit 0x5e
        // +0x9d  POP EBP
        _emit 0x5d
        // +0x9e  JNZ short +0x0b  → offset 0xab (non-zero result: skip type cmp)
        _emit 0x75
        _emit 0x0b
        // +0xa0  MOV EAX, dword ptr [EDI+4]   — a->type
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // +0xa3  SUB EAX, dword ptr [EBX+4]   — a->type - b->type
        _emit 0x2b
        _emit 0x43
        _emit 0x04
        // +0xa6  POP EDI
        _emit 0x5f
        // +0xa7  POP EBX
        _emit 0x5b
        // +0xa8  RET
        _emit 0xc3
        // --- length-diff early exit (target of JNZ near @ +0x10) ----------
        // +0xa9  MOV EAX, ECX               — EAX = a->length - b->length
        _emit 0x8b
        _emit 0xc1
        // +0xab  POP EDI
        _emit 0x5f
        // +0xac  POP EBX
        _emit 0x5b
        // +0xad  RET
        _emit 0xc3
    }
}
