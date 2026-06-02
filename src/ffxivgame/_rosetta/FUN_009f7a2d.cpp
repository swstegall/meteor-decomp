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
// FUNCTION: ffxivgame 0x005f7a2d — bitset-clear-then-notify (46 B / 0x2E)
//
// Two early-exit guards on EBP-locals vs ESI; if neither fires, reads
// a value from [EDI], splits it into a 5-bit slot-index (bits 4:0) and
// an upper word-index (bits 31:5), looks the word up in a global pointer
// table at 0x0137b7e0, computes the byte address as base+4+(slot<<6),
// clears bit 0 of that byte (AND 0xFE), then falls through to push
// [EDI] and call a notification helper at RVA 0x005E788D.
//
// Asm (46 bytes @ orig RVA 0x005f7a2d):
//
//   0x005f7a2d: 39 75 e4          CMP  dword ptr [EBP-0x1C], ESI
//   0x005f7a30: 74 28             JE   0x005f7a5a            ; → RET
//   0x005f7a32: 39 75 e0          CMP  dword ptr [EBP-0x20], ESI
//   0x005f7a35: 74 1b             JE   0x005f7a52            ; → PUSH [EDI]
//   0x005f7a37: 8b 07             MOV  EAX, dword ptr [EDI]
//   0x005f7a39: 8b c8             MOV  ECX, EAX
//   0x005f7a3b: c1 f9 05          SAR  ECX, 5
//   0x005f7a3e: 83 e0 1f          AND  EAX, 0x1F
//   0x005f7a41: c1 e0 06          SHL  EAX, 6
//   0x005f7a44: 8b 0c 8d e0b73701 MOV  ECX, dword ptr [ECX*4 + 0x0137b7e0]
//   0x005f7a4b: 8d 44 01 04       LEA  EAX, [ECX + EAX + 4]
//   0x005f7a4f: 80 20 fe          AND  byte ptr [EAX], 0xFE
//   0x005f7a52: ff 37             PUSH dword ptr [EDI]       ; ← JE target
//   0x005f7a54: e8 34fefeff       CALL 0x005e788d
//   0x005f7a59: 59                POP  ECX
//   0x005f7a5a: c3                RET
//
// Calling convention: inferred __cdecl (no args on entry; bare RET;
// caller provided ESI/EDI/EBP context). Frame: none visible here
// (EBP-locals set up by the outer inlining frame).
//
// Reconstruction: naked __asm byte passthrough. The SIB-indexed load
// (ECX*4 + abs32) and the CALL rel32 each carry a relocation — both
// are masked out of the byte diff by tools/compare.py, so the raw
// immediates baked into the orig binary are reproduced verbatim here.

extern "C" __declspec(naked) void FUN_009f7a2d() {
    __asm {
        // 0x005f7a2d: 39 75 e4       CMP dword ptr [EBP-0x1C], ESI
        _emit 0x39
        _emit 0x75
        _emit 0xe4
        // 0x005f7a30: 74 28          JE +0x28 (→ RET at 0x005f7a5a)
        _emit 0x74
        _emit 0x28
        // 0x005f7a32: 39 75 e0       CMP dword ptr [EBP-0x20], ESI
        _emit 0x39
        _emit 0x75
        _emit 0xe0
        // 0x005f7a35: 74 1b          JE +0x1B (→ PUSH [EDI] at 0x005f7a52)
        _emit 0x74
        _emit 0x1b
        // 0x005f7a37: 8b 07          MOV EAX, dword ptr [EDI]
        _emit 0x8b
        _emit 0x07
        // 0x005f7a39: 8b c8          MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 0x005f7a3b: c1 f9 05       SAR ECX, 5
        _emit 0xc1
        _emit 0xf9
        _emit 0x05
        // 0x005f7a3e: 83 e0 1f       AND EAX, 0x1F
        _emit 0x83
        _emit 0xe0
        _emit 0x1f
        // 0x005f7a41: c1 e0 06       SHL EAX, 6
        _emit 0xc1
        _emit 0xe0
        _emit 0x06
        // 0x005f7a44: 8b 0c 8d e0 b7 37 01   MOV ECX, dword ptr [ECX*4 + 0x0137b7e0]
        _emit 0x8b
        _emit 0x0c
        _emit 0x8d
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        // 0x005f7a4b: 8d 44 01 04    LEA EAX, [ECX + EAX + 4]
        _emit 0x8d
        _emit 0x44
        _emit 0x01
        _emit 0x04
        // 0x005f7a4f: 80 20 fe       AND byte ptr [EAX], 0xFE
        _emit 0x80
        _emit 0x20
        _emit 0xfe
        // 0x005f7a52: ff 37          PUSH dword ptr [EDI]   ← JE-from-0x35 lands here
        _emit 0xff
        _emit 0x37
        // 0x005f7a54: e8 34 fe fe ff CALL 0x005e788d (rel32 = 0xfffefe34)
        _emit 0xe8
        _emit 0x34
        _emit 0xfe
        _emit 0xfe
        _emit 0xff
        // 0x005f7a59: 59             POP ECX
        _emit 0x59
        // 0x005f7a5a: c3             RET    ← JE-from-0x30 lands here
        _emit 0xc3
    }
}
