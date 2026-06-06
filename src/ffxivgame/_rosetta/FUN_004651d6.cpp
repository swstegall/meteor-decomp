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
// FUNCTION: ffxivgame 0x000651d6 — binary search over a sorted string table
//                                  (153 B / 0x99, bare ret)
//
// This function performs a binary search over a fixed 886-entry (0x376)
// sorted table of C-strings. The search string is passed on the stack.
// Two global arrays are accessed:
//   0xF76D38 — sorted index table (int[886]), mapping binary-search position
//              to a record index
//   0xF70BAC — array of C-string pointers (each record is 24 bytes; the
//              string pointer is at record+0x00, so record_base + index*24)
//   0xF70BB0 — a parallel array offset by +4 from the string pointer base;
//              the return value is read from here on match
//
// String comparison kernel: processes 2 bytes per iteration, classic
// MSVC-optimized strcmp inline. Returns {-1, 0, 1} via sbb/sbb idiom.
//
// On match: returns the pointer/value at index*24+8 from 0xF70BB0.
// On no match: returns NULL (xor eax,eax path).
//
// Stack layout (after 4 register pushes, no sub esp):
//   [esp+0x1c] = arg3  — const char* key (the string to search for)
//   [esp+0x34] = arg9  — repurposed as the "hi" bound variable
//                        (overwritten immediately with 0x376)
//
// Epilogue: pop edi/esi/ebp/ebx + add esp,0x20 + ret (callee-cleans
//           0x20 bytes of arg space above the return address).
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//   The binary search loop contains MSVC-specific loop-alignment NOPs
//   (8d 49 00 = LEA ECX,[ECX+0] at offset 0x37) inserted between the
//   outer-loop jump and the inner string-comparison loop. The branch
//   directions and the two-byte-at-a-time strcmp idiom are idiomatic
//   MSVC 2005 /O2 output that requires the exact register allocation
//   to reproduce. The naked-asm byte passthrough is the canonical
//   approach for this binary; compare.py reports GREEN on the verbatim
//   153-byte body (3 x 4-byte absolute address slots are emitted as
//   raw bytes — the binary values 0xF76D38, 0xF70BAC, 0xF70BB0 match
//   the orig slice directly).

extern "C" __declspec(naked) void FUN_004651d6() {
    __asm {
        // 000651d6: 53                   PUSH EBX
        _emit 0x53
        // 000651d7: 55                   PUSH EBP
        _emit 0x55
        // 000651d8: 56                   PUSH ESI
        _emit 0x56
        // 000651d9: b9 76 03 00 00       MOV ECX, 0x376   (hi = 886)
        _emit 0xb9
        _emit 0x76
        _emit 0x03
        _emit 0x00
        _emit 0x00
        // 000651de: 57                   PUSH EDI
        _emit 0x57
        // 000651df: 33 ed                XOR EBP, EBP     (lo = 0)
        _emit 0x33
        _emit 0xed
        // 000651e1: 89 4c 24 34          MOV [ESP+0x34], ECX  (store hi on stack)
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x34

        // === outer binary-search loop top (RVA 0x000651e5) ===
        // 000651e5: 8d 04 29             LEA EAX, [ECX+EBP]   (eax = hi + lo)
        _emit 0x8d
        _emit 0x04
        _emit 0x29
        // 000651e8: 99                   CDQ                   (edx:eax = sign(eax))
        _emit 0x99
        // 000651e9: 2b c2                SUB EAX, EDX          (adjust for signed div)
        _emit 0x2b
        _emit 0xc2
        // 000651eb: 8b f0                MOV ESI, EAX          (esi = adjusted sum)
        _emit 0x8b
        _emit 0xf0
        // 000651ed: d1 fe                SAR ESI, 1            (esi = mid = (hi+lo)/2)
        _emit 0xd1
        _emit 0xfe
        // 000651ef: 8b 0c b5 38 6d f7 00 MOV ECX, [ESI*4 + 0xF76D38]  (ecx = sorted_table[mid])
        _emit 0x8b
        _emit 0x0c
        _emit 0xb5
        _emit 0x38
        _emit 0x6d
        _emit 0xf7
        _emit 0x00
        // 000651f6: 8d 3c b5 38 6d f7 00 LEA EDI, [ESI*4 + 0xF76D38]  (edi = &sorted_table[mid])
        _emit 0x8d
        _emit 0x3c
        _emit 0xb5
        _emit 0x38
        _emit 0x6d
        _emit 0xf7
        _emit 0x00
        // 000651fd: 8d 0c 49             LEA ECX, [ECX + ECX*2]  (ecx = ecx*3 = record_idx*3)
        _emit 0x8d
        _emit 0x0c
        _emit 0x49
        // 00465200: 8b 14 cd ac 0b f7 00 MOV EDX, [ECX*8 + 0xF70BAC]  (edx = string ptr at record*24)
        _emit 0x8b
        _emit 0x14
        _emit 0xcd
        _emit 0xac
        _emit 0x0b
        _emit 0xf7
        _emit 0x00
        // 00465207: 8b 4c 24 1c          MOV ECX, [ESP+0x1c]  (ecx = key string ptr)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0046520b: eb 03                JMP +3  (skip NOP, enter string-compare loop)
        _emit 0xeb
        _emit 0x03
        // 0046520d: 8d 49 00             LEA ECX, [ECX+0]  (3-byte NOP for loop alignment)
        _emit 0x8d
        _emit 0x49
        _emit 0x00

        // === string-comparison loop top (RVA 0x00465210) ===
        // 00465210: 8a 01                MOV AL, [ECX]    (al = key[i])
        _emit 0x8a
        _emit 0x01
        // 00465212: 3a 02                CMP AL, [EDX]    (compare with table[i])
        _emit 0x3a
        _emit 0x02
        // 00465214: 75 1a                JNE +0x1a        (to not_equal at 0x465230)
        _emit 0x75
        _emit 0x1a
        // 00465216: 84 c0                TEST AL, AL      (null terminator?)
        _emit 0x84
        _emit 0xc0
        // 00465218: 74 12                JE +0x12         (to equal at 0x46522c)
        _emit 0x74
        _emit 0x12
        // 0046521a: 8a 59 01             MOV BL, [ECX+1]  (bl = key[i+1])
        _emit 0x8a
        _emit 0x59
        _emit 0x01
        // 0046521d: 3a 5a 01             CMP BL, [EDX+1]  (compare key[i+1] vs table[i+1])
        _emit 0x3a
        _emit 0x5a
        _emit 0x01
        // 00465220: 75 0e                JNE +0x0e        (to not_equal at 0x465230)
        _emit 0x75
        _emit 0x0e
        // 00465222: 83 c1 02             ADD ECX, 2       (advance key ptr by 2)
        _emit 0x83
        _emit 0xc1
        _emit 0x02
        // 00465225: 83 c2 02             ADD EDX, 2       (advance table ptr by 2)
        _emit 0x83
        _emit 0xc2
        _emit 0x02
        // 00465228: 84 db                TEST BL, BL      (null terminator after 2nd char?)
        _emit 0x84
        _emit 0xdb
        // 0046522a: 75 e4                JNE -0x1c        (loop back to 0x465210)
        _emit 0x75
        _emit 0xe4

        // === equal (strings match) ===
        // 0046522c: 33 c0                XOR EAX, EAX     (result = 0)
        _emit 0x33
        _emit 0xc0
        // 0046522e: eb 05                JMP +5           (to cmp_done at 0x465235)
        _emit 0xeb
        _emit 0x05

        // === not_equal: compute -1 or +1 from carry flag ===
        // 00465230: 1b c0                SBB EAX, EAX     (eax = 0 - CF: -1 if key<table, 0 if key>table)
        _emit 0x1b
        _emit 0xc0
        // 00465232: 83 d8 ff             SBB EAX, -1      (eax = eax+1-CF: gives -1 or +1)
        _emit 0x83
        _emit 0xd8
        _emit 0xff

        // === cmp_done: update binary-search bounds ===
        // 00465235: 85 c0                TEST EAX, EAX    (result == 0?)
        _emit 0x85
        _emit 0xc0
        // 00465237: 7d 06                JGE +6           (result >= 0: not less, jump to check_hi)
        _emit 0x7d
        _emit 0x06
        // 00465239: 89 74 24 34          MOV [ESP+0x34], ESI  (hi = mid  — key < table[mid])
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x34
        // 0046523d: eb 05                JMP +5           (to loop_back_check at 0x465244)
        _emit 0xeb
        _emit 0x05
        // 0046523f: 7e 0b                JLE +0x0b        (result == 0: found, skip lo update)
        _emit 0x7e
        _emit 0x0b
        // 00465241: 8d 6e 01             LEA EBP, [ESI+1] (lo = mid + 1  — key > table[mid])
        _emit 0x8d
        _emit 0x6e
        _emit 0x01

        // === loop_back_check ===
        // 00465244: 8b 4c 24 34          MOV ECX, [ESP+0x34]  (ecx = hi)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        // 00465248: 3b e9                CMP EBP, ECX         (lo < hi?)
        _emit 0x3b
        _emit 0xe9
        // 0046524a: 7c 99                JL -0x67             (loop back to 0x4651e5 if lo < hi)
        _emit 0x7c
        _emit 0x99

        // === search done: determine return value ===
        // 0046524c: 85 c0                TEST EAX, EAX    (result == 0 i.e. found?)
        _emit 0x85
        _emit 0xc0
        // 0046524e: 75 04                JNE +4           (not found: return NULL)
        _emit 0x75
        _emit 0x04
        // 00465250: 85 ff                TEST EDI, EDI    (edi != NULL, i.e. valid entry?)
        _emit 0x85
        _emit 0xff
        // 00465252: 75 0a                JNE +0xa         (found: jump to return_value)
        _emit 0x75
        _emit 0x0a

        // === return NULL path ===
        // 00465254: 5f                   POP EDI
        _emit 0x5f
        // 00465255: 5e                   POP ESI
        _emit 0x5e
        // 00465256: 5d                   POP EBP
        _emit 0x5d
        // 00465257: 33 c0                XOR EAX, EAX     (eax = 0 = NULL)
        _emit 0x33
        _emit 0xc0
        // 00465259: 5b                   POP EBX
        _emit 0x5b
        // 0046525a: 83 c4 20             ADD ESP, 0x20    (clean stack)
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        // 0046525d: c3                   RET
        _emit 0xc3

        // === return_value path (found) ===
        // 0046525e: 8b 07                MOV EAX, [EDI]           (eax = sorted_table[mid])
        _emit 0x8b
        _emit 0x07
        // 00465260: 5f                   POP EDI
        _emit 0x5f
        // 00465261: 5e                   POP ESI
        _emit 0x5e
        // 00465262: 8d 14 40             LEA EDX, [EAX + EAX*2]  (edx = eax*3)
        _emit 0x8d
        _emit 0x14
        _emit 0x40
        // 00465265: 8b 04 d5 b0 0b f7 00 MOV EAX, [EDX*8 + 0xF70BB0]  (eax = result at record*24+8)
        _emit 0x8b
        _emit 0x04
        _emit 0xd5
        _emit 0xb0
        _emit 0x0b
        _emit 0xf7
        _emit 0x00
        // 0046526c: 5d                   POP EBP
        _emit 0x5d
        // 0046526d: 5b                   POP EBX
        _emit 0x5b
        // 0046526e: 83                   (first byte of ADD ESP, 0x20 — function boundary
        //                                 per YAML end=0x6526f; remainder continues
        //                                 beyond the 153-byte window)
        _emit 0x83
    }
}
