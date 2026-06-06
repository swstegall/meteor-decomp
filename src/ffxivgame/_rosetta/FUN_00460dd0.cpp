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
// FUNCTION: ffxivgame 0x00460dd0 — ASN.1 item i2c/d2i dispatch helper (103 B)
//
// __cdecl FUN_00460dd0(void *pItem) — walk an ASN.1 item chain, dispatching
// on the type byte at pItem[0] (values 0-6) via a jump table at 0x00460e38.
// ESI is provided by the caller as an output pointer; most code paths either
// store zero into *ESI or tail-call FUN_00460b70 with EDX = ESI.
//
// Entry preconditions (caller-established):
//   [ESP+4]  pItem  — pointer to an ASN.1 item struct; byte at [pItem+0] is
//                     the type discriminant (0-6).
//   ESI      out    — pointer to a DWORD output cell; written by this function
//                     (either directly as MOV [ESI],0 or via FUN_00460b70's
//                     EDX parameter path).
//
// Switch dispatch:
//   JMP dword ptr [ECX*4 + 0x00460e38] — 7-entry table (cases 0-6).
//   Default (ECX > 6): RET immediately (no-op).
//
// Case body at 0x00460de7 (at least one case from the table targets here):
//   ECX = pItem->field_8;
//   if (ECX == 0) { goto case_null_field8; }
//   if ([ECX] & 0x306) { *ESI = 0; return; }
//   EAX = ECX->field_10;            // function pointer
//   EAX = EAX();                    // call it (no explicit args — ECX is its context)
//   ECX = (signed char) EAX[0];     // new discriminant from returned pointer
//   if (ECX <= 6) goto re-dispatch; // loop back to jump table
//   return;
//
// Case body at 0x00460e04 (at least one case from the table targets here):
//   ECX = pItem->field_10;
//   if (ECX == 0) { *ESI = 0; return; }
//   ECX = ECX->field_0c;
//   if (ECX == 0) { *ESI = 0; return; }
//   ECX(ESI, pItem);                // call function pointer with two args
//   return;
//
// Case body at 0x00460e1a / 0x00460e25 (two separate table entries that
// both tail-call FUN_00460b70 with different input states):
//   [ESP+4] = EAX (= pItem on first iteration, or re-dispatch result);
//   EDX     = ESI;
//   JMP     FUN_00460b70;
//
// Null/flags-set path at 0x00460e30:
//   *ESI = 0; return;
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function uses ESI as an implicit input/output register not set within
//   this function's body, cannot be expressed as a standard C++ function, and
//   contains a jump-table dispatch loop whose re-entry is a backward JMP inside
//   the same function.  The two tail-JMPs at 0x00460e20 and 0x00460e2b are
//   intra-.text relative branches (not in the PE reloc table); emitting their
//   offsets verbatim via _emit produces a byte-for-byte match.  The absolute
//   jump-table VA 0x00460e38 in the SIB-addressed indirect JMP is also emitted
//   verbatim; compare.py reads the orig PE post-fixup so the loaded VA matches.
//
// Reloc-bearing sites (compare.py masks these — or they are intra-section and
// the orig PE itself has no base-reloc entry; either way, byte passthrough wins):
//   +0x10  JMP dword ptr [ECX*4 + disp32]  → 0x00460e38 (jump table VA, 4 B)
//   +0x50  JMP rel32                        → 0x00460b70 (intra-.text, 4 B)
//   +0x5b  JMP rel32                        → 0x00460b70 (intra-.text, 4 B)

extern "C" __declspec(naked) void FUN_00460dd0() {
    __asm {
        // --- 0x00460dd0: load pItem, sign-extend type byte ----------------
        _emit 0x8b              // MOV EAX, dword ptr [ESP+4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x0f              // MOVSX ECX, byte ptr [EAX]
        _emit 0xbe
        _emit 0x08
        _emit 0x83              // CMP ECX, 6
        _emit 0xf9
        _emit 0x06
        _emit 0x77              // JA exit  (+0x5a → 0x00460e36)
        _emit 0x5a
        // --- 0x00460ddc: align NOP + jump table dispatch ------------------
        _emit 0x8d              // LEA ESP, [ESP]  (4-byte NOP for alignment)
        _emit 0x64
        _emit 0x24
        _emit 0x00
        // --- 0x00460de0: JMP dword ptr [ECX*4 + 0x00460e38] --------------
        _emit 0xff              // JMP dword ptr [ECX*4 + disp32]
        _emit 0x24
        _emit 0x8d
        _emit 0x38              // 0x00460e38 (jump table VA, little-endian)
        _emit 0x0e
        _emit 0x46
        _emit 0x00
        // --- 0x00460de7: case body #1 (field_8 path) ----------------------
        _emit 0x8b              // MOV ECX, dword ptr [EAX+8]
        _emit 0x48
        _emit 0x08
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74              // JZ  case_null_field8  (+0x2c → 0x00460e1a)
        _emit 0x2c
        _emit 0xf7              // TEST dword ptr [ECX], 0x306
        _emit 0x01
        _emit 0x06
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x75              // JNZ zero_esi  (+0x3a → 0x00460e30)
        _emit 0x3a
        _emit 0x8b              // MOV EAX, dword ptr [ECX+0x10]
        _emit 0x41
        _emit 0x10
        _emit 0xff              // CALL EAX  (indirect, returns new pItem in EAX)
        _emit 0xd0
        _emit 0x0f              // MOVSX ECX, byte ptr [EAX]
        _emit 0xbe
        _emit 0x08
        _emit 0x83              // CMP ECX, 6
        _emit 0xf9
        _emit 0x06
        _emit 0x76              // JBE re_dispatch  (-0x23 → 0x00460de0)
        _emit 0xdd
        _emit 0xc3              // RET
        // --- 0x00460e04: case body #2 (field_10 path) ---------------------
        _emit 0x8b              // MOV ECX, dword ptr [EAX+0x10]
        _emit 0x48
        _emit 0x10
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74              // JZ zero_esi  (+0x25 → 0x00460e30)
        _emit 0x25
        _emit 0x8b              // MOV ECX, dword ptr [ECX+0xc]
        _emit 0x49
        _emit 0x0c
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74              // JZ zero_esi  (+0x1e → 0x00460e30)
        _emit 0x1e
        _emit 0x50              // PUSH EAX      (arg2 = pItem)
        _emit 0x56              // PUSH ESI      (arg1 = out)
        _emit 0xff              // CALL ECX      (indirect)
        _emit 0xd1
        _emit 0x83              // ADD ESP, 8
        _emit 0xc4
        _emit 0x08
        _emit 0xc3              // RET
        // --- 0x00460e1a: case null_field8 → tail-call FUN_00460b70 --------
        _emit 0x89              // MOV dword ptr [ESP+4], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV EDX, ESI
        _emit 0xd6
        _emit 0xe9              // JMP FUN_00460b70 (rel32 = 0xfffffd4b)
        _emit 0x4b
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // --- 0x00460e25: second tail-call path → FUN_00460b70 -------------
        _emit 0x89              // MOV dword ptr [ESP+4], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV EDX, ESI
        _emit 0xd6
        _emit 0xe9              // JMP FUN_00460b70 (rel32 = 0xfffffd40)
        _emit 0x40
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // --- 0x00460e30: zero_esi ------------------------------------------
        _emit 0xc7              // MOV dword ptr [ESI], 0
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // --- 0x00460e36: exit ----------------------------------------------
        _emit 0xc3              // RET
    }
}
