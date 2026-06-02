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
// FUNCTION: ffxivgame 0x005df952 — integer-code mapper (47 B / 0x2F)
//
// Maps an input integer (in EAX on entry) to a corresponding output code via
// a subtract-chain switch pattern. No frame, no callee-saves, no relocations.
// The argument is in EAX — this is a compiler-generated switch dispatch body
// called from a context where EAX already holds the key value.
//
// Mapping:
//   0x3a4 → 0x411
//   0x3a8 → 0x804
//   0x3b5 → 0x412
//   0x3b6 → 0x404
//   else  →  0x000
//
// Asm (47 bytes @ orig RVA 0x005df952):
//   2d a4 03 00 00  SUB  EAX, 0x3a4
//   74 22           JZ   +0x22  (→ case_3a4: return 0x411)
//   83 e8 04        SUB  EAX, 0x4
//   74 17           JZ   +0x17  (→ case_3a8: return 0x804)
//   83 e8 0d        SUB  EAX, 0xd
//   74 0c           JZ   +0x0c  (→ case_3b5: return 0x412)
//   48              DEC  EAX
//   74 03           JZ   +0x03  (→ case_3b6: return 0x404)
//   33 c0           XOR  EAX, EAX
//   c3              RET
//   b8 04 04 00 00  MOV  EAX, 0x404   (case_3b6)
//   c3              RET
//   b8 12 04 00 00  MOV  EAX, 0x412   (case_3b5)
//   c3              RET
//   b8 04 08 00 00  MOV  EAX, 0x804   (case_3a8)
//   c3              RET
//   b8 11 04 00 00  MOV  EAX, 0x411   (case_3a4)
//   c3              RET
//
// Calling convention: non-standard — argument already in EAX.
// Frame: none — no prologue, no callee-saves.
//
// Reconstruction: __declspec(naked) with real MASM mnemonics. No relocations
// are present (all jumps are short relative offsets within the function body;
// all immediates are plain integer constants). MASM selects the correct
// encodings: 2D-form SUB EAX,imm32 for 0x3a4, 83-form SUB EAX,imm8 for
// smaller values, 48 DEC EAX, 74-form short JZ, and B8-form MOV EAX,imm32.

extern "C" __declspec(naked) void FUN_009df952() {
    __asm {
        sub     eax, 0x3a4
        jz      case_3a4
        sub     eax, 0x4
        jz      case_3a8
        sub     eax, 0xd
        jz      case_3b5
        dec     eax
        jz      case_3b6
        xor     eax, eax
        ret
    case_3b6:
        mov     eax, 0x404
        ret
    case_3b5:
        mov     eax, 0x412
        ret
    case_3a8:
        mov     eax, 0x804
        ret
    case_3a4:
        mov     eax, 0x411
        ret
    }
}
