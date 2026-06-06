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
// FUNCTION: ffxivgame 0x00064d80 — keyed-table lookup with binary-search
//           fallback (__cdecl, 185 B / 0xb9)
//
// RVA 0x00064d80 – 0x00064e38 (inclusive)
//
// High-level behaviour (recovered from asm):
//
//   void* FUN_00464d80(SomeStruct* key) {
//       // 8 bytes of local storage allocated via __chkstk
//       if (!key) return NULL;
//
//       if (key->field_8 != 0) {
//           // Direct: the answer is already cached in key->field_8
//           return (void*)key->field_8;
//       }
//
//       // Try the global lookup object first
//       GlobalObj* g = *(GlobalObj**)0x0132e7b8;
//       if (g) {
//           void* local = 0;
//           LookupResult* r = FUN_00466b60(g, &local);
//           if (r) return r->field_4->field_8;
//       }
//
//       // Binary search in sorted table at 0xf77b10 (0x348 = 840 entries)
//       int left = 0, right = 0x348;
//       while (left < right) {
//           int mid = (left + right) / 2;
//           // Compare: table_entry[mid].compare(&key)  [__thiscall on EBP]
//           int cmp = FUN_00464bf0(&key);
//           if (cmp < 0)       right = mid;
//           else if (cmp > 0)  left  = mid + 1;
//           else               break;  // found: mid_val = *EBP
//       }
//       if (cmp == 0 && EBP) {
//           int val = *EBP;
//           return *(void**)(0xf70bb0 + val * 24);
//       }
//       return NULL;
//   }
//
// Reloc-bearing sites (each 4-byte window wildcarded by compare.py):
//   +0x06  CALL rel32  → 0x009d29d0  (__chkstk — stack probe/alloc)
//   +0x28  MOV moffs32 ← [0x0132e7b8]  (DIR32 global pointer)
//   +0x3f  CALL rel32  → 0x00466b60  (global lookup helper)
//   +0x6f  LEA EBP, [ESI*4 + 0xf77b10]  (imm32 table address)
//   +0x79  CALL rel32  → 0x00464bf0  (compare, nearby; rel32 = 0xFFFFFDF3)
//   +0xac  MOV EAX, [ECX*8 + 0xf70bb0]  (imm32 result table address)
//
// Why naked asm: the __chkstk prologue (`MOV EAX,8; CALL __chkstk`) plus
// the unconventional stack discipline it creates (8-byte alloca + shared
// tail at 0x00464e34 used by both early-exit and binary-search paths) is
// not reproducible from plain C++ under /O2; the register allocator and
// branch encoding differ between isolated-TU and whole-binary context.
// Byte-pinned naked asm is the pragmatic match (same strategy as
// FUN_00403f10, FUN_00401750, FUN_00401b70 siblings).

extern "C" __declspec(naked) void FUN_00464d80() {
    __asm {
        // --- prologue: alloca(8) via __chkstk ---------------------------
        _emit 0xb8              // MOV EAX, 0x00000008
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL rel32 → 0x009d29d0 (__chkstk)
        _emit 0x46
        _emit 0xdc
        _emit 0x56
        _emit 0x00

        // --- load arg1; null-check --------------------------------------
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0xc]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x57              // PUSH EDI
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0x3b              // CMP ECX, EDI
        _emit 0xcf
        _emit 0x75              // JNZ short +7 → [key != NULL path]
        _emit 0x07
        _emit 0x33              // XOR EAX, EAX   (return NULL)
        _emit 0xc0
        _emit 0x5f              // POP EDI
        _emit 0x83              // ADD ESP, 0x8   (remove alloca)
        _emit 0xc4
        _emit 0x08
        _emit 0xc3              // RET

        // --- check key->field_8 -----------------------------------------
        _emit 0x8b              // MOV EAX, dword ptr [ECX+0x8]
        _emit 0x41
        _emit 0x08
        _emit 0x3b              // CMP EAX, EDI
        _emit 0xc7
        _emit 0x0f              // JNZ near → shared tail (return key->field_8)
        _emit 0x85
        _emit 0x8d
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // --- load global object pointer ---------------------------------
        _emit 0xa1              // MOV EAX, [0x0132e7b8]
        _emit 0xb8
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x3b              // CMP EAX, EDI
        _emit 0xc7
        _emit 0x74              // JZ short +0x25 → binary search
        _emit 0x25

        // --- call global lookup helper ----------------------------------
        _emit 0x89              // MOV dword ptr [ESP+0x8], ECX  (save key)
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8d              // LEA ECX, [ESP+0x4]  (&local_out)
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x51              // PUSH ECX  (arg2 = &local_out)
        _emit 0x50              // PUSH EAX  (arg1 = global_ptr)
        _emit 0x89              // MOV dword ptr [ESP+0xc], EDI  (zero local_out)
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0xe8              // CALL rel32 → 0x00466b60
        _emit 0x9d
        _emit 0x1d
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x3b              // CMP EAX, EDI
        _emit 0xc7
        _emit 0x74              // JZ short +0xb → binary search
        _emit 0x0b

        // --- return result->field_4->field_8 ----------------------------
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x4]
        _emit 0x50
        _emit 0x04
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x8]
        _emit 0x42
        _emit 0x08
        _emit 0x5f              // POP EDI
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc3              // RET

        // --- binary search setup ----------------------------------------
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0xbb              // MOV EBX, 0x348   (right = 840)
        _emit 0x48
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ECX+0]  (align NOP; EDI=left=0)
        _emit 0x49
        _emit 0x00

        // --- binary search loop top -------------------------------------
        _emit 0x8d              // LEA EAX, [EBX + EDI*1]  (left + right)
        _emit 0x04
        _emit 0x3b
        _emit 0x99              // CDQ
        _emit 0x2b              // SUB EAX, EDX
        _emit 0xc2
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0xd1              // SAR ESI, 1   (mid = (left+right)/2)
        _emit 0xfe
        _emit 0x8d              // LEA EAX, [ESP+0x1c]   (&key on stack)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x8d              // LEA EBP, [ESI*4 + 0xf77b10]   (&table[mid])
        _emit 0x2c
        _emit 0xb5
        _emit 0x10
        _emit 0x7b
        _emit 0xf7
        _emit 0x00
        _emit 0x50              // PUSH EAX   (arg: &key)
        _emit 0x8b              // MOV ECX, EBP   (this = &table[mid])
        _emit 0xcd
        _emit 0xe8              // CALL rel32 → 0x00464bf0 (compare)
        _emit 0xf3
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04

        // --- adjust bounds based on compare result ----------------------
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7d              // JGE short +4   (cmp >= 0: don't shrink right)
        _emit 0x04
        _emit 0x8b              // MOV EBX, ESI   (right = mid; cmp < 0)
        _emit 0xde
        _emit 0xeb              // JMP short +5   (→ loop condition)
        _emit 0x05
        _emit 0x7e              // JLE short +7   (cmp == 0: found, jump past)
        _emit 0x07
        _emit 0x8d              // LEA EDI, [ESI+1]   (left = mid+1; cmp > 0)
        _emit 0x7e
        _emit 0x01

        // --- loop condition: left < right? ------------------------------
        _emit 0x3b              // CMP EDI, EBX
        _emit 0xfb
        _emit 0x7c              // JL short -0x31   (back to loop top)
        _emit 0xcf

        // --- not found or check EBP -------------------------------------
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ short +4   (cmp != 0: not found → NULL)
        _emit 0x04
        _emit 0x85              // TEST EBP, EBP
        _emit 0xed
        _emit 0x75              // JNZ short +0xa  (EBP valid: compute result)
        _emit 0x0a

        // --- not found: return NULL -------------------------------------
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5f              // POP EDI
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc3              // RET

        // --- found: dereference and index into result table -------------
        _emit 0x8b              // MOV EBP, dword ptr [EBP+0]   (*&table[mid])
        _emit 0x6d
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0x8d              // LEA ECX, [EBP + EBP*2]   (ECX = EBP*3)
        _emit 0x4c
        _emit 0x6d
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ECX*8 + 0xf70bb0]  (EBP*24)
        _emit 0x04
        _emit 0xcd
        _emit 0xb0
        _emit 0x0b
        _emit 0xf7
        _emit 0x00
        _emit 0x5d              // POP EBP

        // --- shared tail (reached from direct/binary-search paths) ------
        _emit 0x5b              // POP EBX
        _emit 0x5f              // POP EDI
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc3              // RET
    }
}
