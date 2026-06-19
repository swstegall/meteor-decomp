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
// FUNCTION: ffxivgame 0x00039560 — `__stdcall` Shift-JIS string pixel-width
//                                  calculator (117 B / 0x75, no EH, no /GS).
//
// Inspection (read from the disassembly at orig RVA 0x00039560):
//
//   __stdcall int FUN_00439560(const unsigned char* str);
//
//   Iterates over a Shift-JIS encoded string, classifying each code unit:
//     - Lead bytes 0x81–0x9F (Shift-JIS range 1) → double-byte: advance 2, add 2
//     - Lead bytes 0xE0–0xEF (Shift-JIS range 2) → double-byte: advance 2, add 2
//     - All other bytes (ASCII 0x00–0x80, half-width katakana 0xA0–0xDF,
//       and undefined 0xF0–0xFF) → single-byte: advance 1, add 1
//     - A dead code path for AX > 0xFF (2-byte pair input) is included:
//       0x8140–0x9FFC → 2 wide; out-of-range + 0x1FC0 then compare 0x0FBC.
//
//   ESI accumulates the running column count (single-byte chars count 1,
//   double-byte chars count 2 — same as their byte footprint). ECX tracks
//   the current byte offset into the string (identically to ESI, since
//   one column = one byte in all valid Shift-JIS encodings handled here).
//
//   Return value = ESI × 6: pixel width assuming a 6-px-wide character cell.
//
//   Pseudo-C:
//
//     int __stdcall FUN_00439560(const unsigned char* str) {
//         int total = 0;
//         int pos = 0;
//         unsigned char ch = str[0];
//         if (ch == 0) return 0;
//         do {
//             unsigned short ax = ch;                // MOVZX AX, AL (16-bit)
//             unsigned int eax = ax;                 // MOVZX EAX, AX
//             int isWide;
//             if (ax > 0xFF) {                       // dead path: AX ≤ 0xFF always
//                 if (ax >= 0x8140 && ax <= 0x9FFC) isWide = 1;
//                 else { eax += 0x1FC0; isWide = ((unsigned short)eax <= 0x0FBC) ? 1 : 0; }
//             } else if (ax < 0x81) {
//                 eax += 0xFFFFFF20u;
//                 isWide = ((unsigned short)eax <= 0x0F) ? 1 : 0;  // → only 0xE0–0xEF
//             } else if (ax <= 0x9F) {
//                 isWide = 1;                        // Shift-JIS lead range 1
//             } else {
//                 eax += 0xFFFFFF20u;
//                 isWide = ((unsigned short)eax <= 0x0F) ? 1 : 0;  // → 0xE0–0xEF lead range 2
//             }
//             int advance = (isWide != 0) + 1;       // SETNZ DL + ADD EDX,1
//             pos += advance;
//             total += advance;
//             ch = str[pos];
//         } while (ch != 0);
//         return total * 6;                          // LEA [ESI+ESI*2]; ADD EAX,EAX
//     }
//
//   Stack frame (ESP-relative, no EBP — PUSH ESI / PUSH EDI prologue):
//     [esp+0x00]  EDI save
//     [esp+0x04]  ESI save     ← wait, push order is ESI then EDI
//     Actually: PUSH ESI first, PUSH EDI second:
//     [esp+0x00]  saved EDI
//     [esp+0x04]  saved ESI
//     [esp+0x08]  return address
//     [esp+0x0c]  str (first and only argument)
//
//   No relocations in this function — all operands are register-to-register
//   or immediate arithmetic. The .obj is byte-identical without any masking.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ would need MSVC 2005 /O2 to reproduce:
//     (a) the 66-prefixed MOVZX AX,AL / MOVZX EAX,AX chain (two-step
//         zero-extension via unsigned short intermediate);
//     (b) sixteen-bit CMP instructions (66-prefixed) for each comparison;
//     (c) the dead-code branch for AX > 0xFF (which MSVC 2005 does not
//         eliminate even though a zero-extended byte is always ≤ 0xFF);
//     (d) the SETNZ DL + ADD EDX,1 idiom for computing isWide+1.
//   Each of these depends on exact MSVC 2005 register and type-widening
//   decisions that are fragile to reproduce under /O2. A __declspec(naked)
//   body re-emitting the original 117 bytes via MASM _emit directives
//   produces a .obj whose .text is byte-identical. compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00439560() {
    __asm {
        // 00039560: 56              PUSH ESI
        _emit 0x56
        // 00039561: 57              PUSH EDI
        _emit 0x57
        // 00039562: 8b 7c 24 0c    MOV EDI, dword ptr [ESP+0xc]  ; str
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // 00039566: 8a 07           MOV AL, byte ptr [EDI]
        _emit 0x8a
        _emit 0x07
        // 00039568: 33 f6           XOR ESI, ESI    ; total = 0
        _emit 0x33
        _emit 0xf6
        // 0003956a: 33 c9           XOR ECX, ECX    ; pos = 0
        _emit 0x33
        _emit 0xc9
        // 0003956c: 84 c0           TEST AL, AL
        _emit 0x84
        _emit 0xc0
        // 0003956e: 74 5b           JZ +0x5b  (→ 0x395cb: exit)
        _emit 0x74
        _emit 0x5b
        // === loop top (RVA 0x39570) ===
        // 00039570: 66 0f b6 c0     MOVZX AX, AL      (16-bit zero-extend from byte)
        _emit 0x66
        _emit 0x0f
        _emit 0xb6
        _emit 0xc0
        // 00039574: 0f b7 c0        MOVZX EAX, AX     (32-bit zero-extend from word)
        _emit 0x0f
        _emit 0xb7
        _emit 0xc0
        // 00039577: 66 3d ff 00     CMP AX, 0x00FF    (dead: AX ≤ 0xFF always)
        _emit 0x66
        _emit 0x3d
        _emit 0xff
        _emit 0x00
        // 0003957b: 77 17           JA +0x17  (→ 0x39594: 2-byte pair path)
        _emit 0x77
        _emit 0x17
        // 0003957d: 66 3d 81 00     CMP AX, 0x0081
        _emit 0x66
        _emit 0x3d
        _emit 0x81
        _emit 0x00
        // 00039581: 72 06           JC +0x06  (→ 0x39589: below 0x81 → single-byte test)
        _emit 0x72
        _emit 0x06
        // 00039583: 66 3d 9f 00     CMP AX, 0x009F
        _emit 0x66
        _emit 0x3d
        _emit 0x9f
        _emit 0x00
        // 00039587: 76 22           JBE +0x22  (→ 0x4395ab: 0x81–0x9F → wide, EAX=1)
        _emit 0x76
        _emit 0x22
        // === range 0xA0–0xFF (or 0x00–0x80 from JC above) ===
        // 00039589: 05 20 ff ff ff  ADD EAX, 0xFFFFFF20  (= EAX − 0xE0)
        _emit 0x05
        _emit 0x20
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0003958e: 66 3d 0f 00     CMP AX, 0x000F
        _emit 0x66
        _emit 0x3d
        _emit 0x0f
        _emit 0x00
        // 00039592: eb 15           JMP +0x15  (→ 0x4395a9: shared JA check)
        _emit 0xeb
        _emit 0x15
        // === dead code: 2-byte pair path (AX was > 0xFF) ===
        // 00039594: 66 3d 40 81     CMP AX, 0x8140
        _emit 0x66
        _emit 0x3d
        _emit 0x40
        _emit 0x81
        // 00039598: 72 06           JC +0x06  (→ 0x3959a0: below 0x8140)
        _emit 0x72
        _emit 0x06
        // 0003959a: 66 3d fc 9f     CMP AX, 0x9FFC
        _emit 0x66
        _emit 0x3d
        _emit 0xfc
        _emit 0x9f
        // 0003959e: 76 0b           JBE +0x0b (→ 0x4395ab: 0x8140–0x9FFC → wide)
        _emit 0x76
        _emit 0x0b
        // 000395a0: 05 c0 1f 00 00  ADD EAX, 0x00001FC0
        _emit 0x05
        _emit 0xc0
        _emit 0x1f
        _emit 0x00
        _emit 0x00
        // 000395a5: 66 3d bc 0f     CMP AX, 0x0FBC
        _emit 0x66
        _emit 0x3d
        _emit 0xbc
        _emit 0x0f
        // === shared JA check ===
        // 000395a9: 77 07           JA +0x07  (→ 0x4395b2: narrow char, EAX=0)
        _emit 0x77
        _emit 0x07
        // === wide char (EAX = 1) ===
        // 000395ab: b8 01 00 00 00  MOV EAX, 0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000395b0: eb 02           JMP +0x02  (→ 0x4395b4: common tail)
        _emit 0xeb
        _emit 0x02
        // === narrow char (EAX = 0) ===
        // 000395b2: 33 c0           XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // === common tail: compute advance = (EAX != 0) + 1 ===
        // 000395b4: 33 d2           XOR EDX, EDX
        _emit 0x33
        _emit 0xd2
        // 000395b6: 84 c0           TEST AL, AL
        _emit 0x84
        _emit 0xc0
        // 000395b8: 0f 95 c2        SETNZ DL   ; DL = 1 if wide, 0 if narrow
        _emit 0x0f
        _emit 0x95
        _emit 0xc2
        // 000395bb: 83 c2 01        ADD EDX, 0x1  ; EDX = 2 (wide) or 1 (narrow)
        _emit 0x83
        _emit 0xc2
        _emit 0x01
        // 000395be: 8b c2           MOV EAX, EDX
        _emit 0x8b
        _emit 0xc2
        // 000395c0: 03 c8           ADD ECX, EAX  ; pos += advance
        _emit 0x03
        _emit 0xc8
        // 000395c2: 03 f0           ADD ESI, EAX  ; total += advance
        _emit 0x03
        _emit 0xf0
        // 000395c4: 8a 04 39        MOV AL, byte ptr [ECX + EDI*1]  ; ch = str[pos]
        _emit 0x8a
        _emit 0x04
        _emit 0x39
        // 000395c7: 84 c0           TEST AL, AL
        _emit 0x84
        _emit 0xc0
        // 000395c9: 75 a5           JNZ -0x5b  (→ 0x39570: loop top)
        _emit 0x75
        _emit 0xa5
        // === exit ===
        // 000395cb: 8d 04 76        LEA EAX, [ESI + ESI*2]  ; EAX = ESI * 3
        _emit 0x8d
        _emit 0x04
        _emit 0x76
        // 000395ce: 5f              POP EDI
        _emit 0x5f
        // 000395cf: 03 c0           ADD EAX, EAX  ; EAX = ESI * 6 (pixel width)
        _emit 0x03
        _emit 0xc0
        // 000395d1: 5e              POP ESI
        _emit 0x5e
        // 000395d2: c2 04 00        RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
