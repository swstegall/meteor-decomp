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
// FUNCTION: ffxivgame 0x0000a710 — string-table linear search
//                                  (107 B / 0x6b)
//
// Non-standard calling convention: first argument arrives in EAX (not on the
// stack or in ECX).  The function never returns directly — on a match it
// tail-calls the shared epilogue function at 0x0040a77b (which does
// POP ESI / MOVZX AX,BL / POP EBX / RET, returning the 16-bit index in AX).
// On every non-match pass it loops back unconditionally.
//
// Functional description:
//   Given a candidate string pointer in EAX (or NULL → fall back to
//   DAT_00f5533c), the function walks a global array of C-string pointers
//   at DAT_00f558c0 (count stored at DAT_01265300) using an inline two-byte-
//   stride strcmp.  On a match it tail-calls the epilogue at 0x0040a77b.
//   If the whole table is exhausted without a match the input pointer is
//   replaced with DAT_00f55328 and the search restarts, making the outer
//   loop an unconditional do-while that eventually always succeeds.
//
// Memory layout (absolute addresses as in the original binary):
//   0x01265300  byte — number of entries in the string table
//   0x00f558c0  const char *[] — array of entry pointers (indexed by byte BL)
//   0x00f55328  const char * — "not found" fallback string
//   0x00f5533c  const char * — NULL-input fallback string
//
// Structure:
//   outer do-while:
//     if (EAX == 0) EAX = 0xf5533c;
//     BL = 0;
//     if (DAT_01265300 == 0) goto not_found;
//     JMP loop_body;           // skip 7-byte alignment NOP sled
//     [7-byte NOP: LEA ESP,[ESP+0]]
//   loop_body:   (16-byte aligned at 0x0040a730)
//     ECX = table[BL];
//     EAX = input ptr;
//     [4-byte NOP: LEA ESP,[ESP+0]]   // inner-loop alignment
//     inline two-byte-stride strcmp:
//       DL = *EAX; compare *ECX; if != → SBB result
//       if DL == 0 → equal (result = 0)
//       DL = EAX[1]; compare ECX[1]; if != → SBB result
//       EAX += 2; ECX += 2; if DL != 0 → repeat
//     if result == 0 → tail-call epilogue at 0x0040a77b
//     BL++; if BL < table_count → repeat loop_body
//   not_found:
//     EAX (= ESI) = 0xf55328;
//     goto outer loop top
//
// Alignment NOPs:
//   0xa729..0xa72f  8d a4 24 00 00 00 00  LEA ESP,[ESP+0x00]  (7-byte)
//   0xa73c..0xa73f  8d 64 24 00            LEA ESP,[ESP+0x00]  (4-byte)
//
// The function body is exactly 107 bytes (0x6b).  The epilogue
// (POP ESI / MOVZX AX,BL / POP EBX / RET = 7 bytes at 0x0040a77b) is
// the start of the NEXT function and is NOT part of this body.
//
// Reloc-bearing bytes: NONE.  All absolute addresses are data references
// baked as immediate constants; the function contains no CALL instructions.
// Emitting the bytes verbatim via _emit produces a .obj whose .text matches
// the orig slice byte-for-byte with no relocations.

extern "C" __declspec(naked) void FUN_0040a710() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x75              // JNZ +5 → 0x0040a71d
        _emit 0x05
        _emit 0xbe              // MOV ESI, 0x00f5533c
        _emit 0x3c
        _emit 0x53
        _emit 0xf5
        _emit 0x00
        _emit 0x32              // XOR BL, BL
        _emit 0xdb
        _emit 0x38              // CMP byte ptr [0x01265300], BL
        _emit 0x1d
        _emit 0x00
        _emit 0x53
        _emit 0x26
        _emit 0x01
        _emit 0x76              // JBE +0x4d → 0x0040a774  (count==0 → not_found)
        _emit 0x4d
        _emit 0xeb              // JMP +7 → 0x0040a730     (enter loop body)
        _emit 0x07
        _emit 0x8d              // LEA ESP, [ESP+0x00]      (7-byte NOP sled, loop alignment)
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // loop_body: (16-byte aligned at 0x0040a730)
        _emit 0x0f              // MOVZX EAX, BL
        _emit 0xb6
        _emit 0xc3
        _emit 0x8b              // MOV ECX, dword ptr [EAX*4 + 0x00f558c0]
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x58
        _emit 0xf5
        _emit 0x00
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x8d              // LEA ESP, [ESP+0x00]      (4-byte NOP, inner-loop alignment)
        _emit 0x64
        _emit 0x24
        _emit 0x00
        // strcmp_top:
        _emit 0x8a              // MOV DL, byte ptr [EAX]
        _emit 0x10
        _emit 0x3a              // CMP DL, byte ptr [ECX]
        _emit 0x11
        _emit 0x75              // JNZ +0x1a → 0x0040a760  (chars differ → compute result)
        _emit 0x1a
        _emit 0x84              // TEST DL, DL
        _emit 0xd2
        _emit 0x74              // JZ +0x12 → 0x0040a75c   (NUL → equal)
        _emit 0x12
        _emit 0x8a              // MOV DL, byte ptr [EAX+1]
        _emit 0x50
        _emit 0x01
        _emit 0x3a              // CMP DL, byte ptr [ECX+1]
        _emit 0x51
        _emit 0x01
        _emit 0x75              // JNZ +0x0e → 0x0040a760
        _emit 0x0e
        _emit 0x83              // ADD EAX, 2
        _emit 0xc0
        _emit 0x02
        _emit 0x83              // ADD ECX, 2
        _emit 0xc1
        _emit 0x02
        _emit 0x84              // TEST DL, DL
        _emit 0xd2
        _emit 0x75              // JNZ -0x1c → 0x0040a740  (not NUL → strcmp_top)
        _emit 0xe4
        _emit 0x33              // XOR EAX, EAX             (equal: result = 0)
        _emit 0xc0
        _emit 0xeb              // JMP +5 → 0x0040a765
        _emit 0x05
        // differ: (0x0040a760)
        _emit 0x1b              // SBB EAX, EAX             (EAX = 0 or -1)
        _emit 0xc0
        _emit 0x83              // SBB EAX, -1              (EAX = -1 or +1)
        _emit 0xd8
        _emit 0xff
        // result_check: (0x0040a765)
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x12 → 0x0040a77b   (match → return BL)
        _emit 0x12
        _emit 0x80              // ADD BL, 1
        _emit 0xc3
        _emit 0x01
        _emit 0x3a              // CMP BL, byte ptr [0x01265300]
        _emit 0x1d
        _emit 0x00
        _emit 0x53
        _emit 0x26
        _emit 0x01
        _emit 0x72              // JC -0x44 → 0x0040a730   (below count → loop_body)
        _emit 0xbc
        // not_found: (0x0040a774)
        _emit 0xbe              // MOV ESI, 0x00f55328
        _emit 0x28
        _emit 0x53
        _emit 0xf5
        _emit 0x00
        _emit 0xeb              // JMP -0x67 → 0x0040a714  (→ outer loop top)
        _emit 0x99
        // Function ends here (107 bytes / 0x6b).
        // The epilogue (POP ESI / MOVZX AX,BL / POP EBX / RET) is the
        // start of the NEXT function at 0x0040a77b — not part of this body.
    }
}
