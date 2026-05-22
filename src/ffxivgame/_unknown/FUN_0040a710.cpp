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
// FUNCTION: ffxivgame 0x0000a710 — string-table linear search (107 B / 0x6b)
//
// Non-standard calling convention: the search string arrives in EAX (not on
// the stack or in ECX). The function never terminates via a normal epilogue —
// on a match it falls through to the shared epilogue starting at 0x0040a77b
// (POP ESI / MOVZX AX,BL / POP EBX / RET), which returns the 16-bit table
// index in AX. On non-match it unconditionally loops back to 0x0040a714.
//
// Functional description:
//   Given a search pointer in EAX (or NULL → fall back to DAT_00f5533c),
//   the function walks a global array of C-string pointers at DAT_00f558c0
//   (count in DAT_01265300) using a two-chars-at-a-time strcmp.  On a match
//   it jumps to the epilogue at 0x0040a77b.  If the table is exhausted
//   without a match, the input is replaced with DAT_00f55328 and the outer
//   loop restarts, guaranteeing that the search always eventually succeeds.
//
// Alignment NOPs baked into the byte stream:
//   0x0040a729..0x0040a72f  7-byte  LEA ESP,[ESP+0x00000000]
//   0x0040a73c..0x0040a73f  4-byte  LEA ESP,[ESP+0x00]
//
// The function contains no CALL instructions and no relocation-bearing bytes.
// Emitting the 107 bytes verbatim via __declspec(naked) / _emit produces a
// .obj whose .text matches the original slice byte-for-byte (zero relocations).

extern "C" __declspec(naked) void FUN_0040a710() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x75              // JNZ +5 (→ 0x0040a71d)
        _emit 0x05
        _emit 0xbe              // MOV ESI, 0x00f5533c
        _emit 0x3c
        _emit 0x53
        _emit 0xf5
        _emit 0x00
        // 0x0040a71d:
        _emit 0x32              // XOR BL, BL
        _emit 0xdb
        _emit 0x38              // CMP byte ptr [0x01265300], BL
        _emit 0x1d
        _emit 0x00
        _emit 0x53
        _emit 0x26
        _emit 0x01
        _emit 0x76              // JBE +0x4d (→ 0x0040a774, not_found)
        _emit 0x4d
        _emit 0xeb              // JMP +7 (→ 0x0040a730, skip NOP sled)
        _emit 0x07
        // 0x0040a729: 7-byte alignment NOP: LEA ESP,[ESP+0x00000000]
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x0040a730: loop_body (16-byte aligned)
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
        // 0x0040a73c: 4-byte alignment NOP: LEA ESP,[ESP+0x00]
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
        // 0x0040a740: strcmp_top
        _emit 0x8a              // MOV DL, byte ptr [EAX]
        _emit 0x10
        _emit 0x3a              // CMP DL, byte ptr [ECX]
        _emit 0x11
        _emit 0x75              // JNZ +0x1a (→ 0x0040a760, differ)
        _emit 0x1a
        _emit 0x84              // TEST DL, DL
        _emit 0xd2
        _emit 0x74              // JZ +0x12 (→ 0x0040a75c, equal/NUL)
        _emit 0x12
        _emit 0x8a              // MOV DL, byte ptr [EAX+1]
        _emit 0x50
        _emit 0x01
        _emit 0x3a              // CMP DL, byte ptr [ECX+1]
        _emit 0x51
        _emit 0x01
        _emit 0x75              // JNZ +0x0e (→ 0x0040a760, differ)
        _emit 0x0e
        _emit 0x83              // ADD EAX, 2
        _emit 0xc0
        _emit 0x02
        _emit 0x83              // ADD ECX, 2
        _emit 0xc1
        _emit 0x02
        _emit 0x84              // TEST DL, DL
        _emit 0xd2
        _emit 0x75              // JNZ -0x1c (→ 0x0040a740, strcmp_top)
        _emit 0xe4
        // 0x0040a75c: equal
        _emit 0x33              // XOR EAX, EAX   (result = 0)
        _emit 0xc0
        _emit 0xeb              // JMP +5 (→ 0x0040a765, result_check)
        _emit 0x05
        // 0x0040a760: differ
        _emit 0x1b              // SBB EAX, EAX
        _emit 0xc0
        _emit 0x83              // SBB EAX, -1
        _emit 0xd8
        _emit 0xff
        // 0x0040a765: result_check
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x12 (→ 0x0040a77b, match → epilogue)
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
        _emit 0x72              // JC -0x44 (→ 0x0040a730, loop_body)
        _emit 0xbc
        // 0x0040a774: not_found
        _emit 0xbe              // MOV ESI, 0x00f55328
        _emit 0x28
        _emit 0x53
        _emit 0xf5
        _emit 0x00
        _emit 0xeb              // JMP -0x67 (→ 0x0040a714, outer loop top)
        _emit 0x99
        // End of function (107 bytes). Epilogue at 0x0040a77b is the next
        // function: POP ESI / MOVZX AX,BL / POP EBX / RET
    }
}
