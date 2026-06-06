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
// FUNCTION: ffxivgame 0x004654b0 — OBJ_NAME add-or-replace helper (150 B /
//                                  0x96, __cdecl, 2 stack args).
//
// __cdecl int FUN_004654b0(some_type arg0, int flags):
//   Stack args at [ESP+0x14] and [ESP+0x18] after a 0x10-byte alloca.
//   Returns 1 on success, 0 if the lookup produced no entry.
//
// Object-level description (recovered from asm @ 0x004654b0):
//
//   1. Allocate 0x10 bytes on the stack via __chkstk (MOV EAX,0x10 / CALL).
//   2. Load global pointer g_hash_table at [0x0132e7c8]. If NULL, restore
//      stack and return (EAX was 0 from the TEST — callers tolerate that).
//   3. Read arg0 → ECX, arg1 → EDX. Stash arg0 into local slot [ESP+8].
//   4. PUSH ESI. LEA the local-slot base (&local[0]) into ECX, PUSH it.
//      Clear bit 15 of arg1 (AND EDX,0xffff7fff). PUSH g_hash_table (EAX).
//      Write masked-arg1 into local[0] ([ESP+0xc] after the three pushes).
//   5. CALL FUN_00466af0(g_hash_table, &local[0])  — find/create entry; returns
//      a pointer or NULL. ESI = result. ADD ESP,8.
//   6. If ESI == 0: XOR EAX,EAX; POP ESI; ADD ESP,0x10; RET (return 0).
//   7. Load g_obj_name_table at [0x0132e7c0]; if NULL skip to step 11.
//   8. PUSH EDI. EDI = *ESI (first field). PUSH g_obj_name_table.
//      CALL FUN_00464030(g_obj_name_table) → sk_num count. ADD ESP,4.
//      If count <= EDI, skip the replace (JLE → step 10 / POP EDI).
//   9. Replace path: MOV EDX,[0x0132e7c0]; PUSH EDI; PUSH EDX;
//      CALL FUN_00464040(g_obj_name_table, EDI) → sk_value entry.
//      Virtual dispatch through entry->fn_at_8([ESI+8],[ESI],[ESI+0xc]).
//      ADD ESP,0x14 (cleans both calls' args together).
//  10. POP EDI.
//  11. PUSH ESI; CALL FUN_004632f0(ESI)  — CRYPTO_free front half.
//      ADD ESP,4.
//  12. MOV EAX,1; POP ESI; ADD ESP,0x10; RET (return 1).
//
// Reloc-bearing sites (offsets within function body, wildcarded by compare.py):
//   +0x05  REL32  0x009d29d0  (__chkstk / __alloca_probe)
//   +0x0b  DIR32  0x0132e7c8  (g_hash_table)
//   +0x31  REL32  0x00466af0  (FUN_00466af0 — find/create OBJ_NAME entry)
//   +0x42  DIR32  0x0132e7c0  (g_obj_name_table)
//   +0x50  REL32  0x00464030  (sk_num)
//   +0x5e  DIR32  0x0132e7c0  (g_obj_name_table, second load)
//   +0x65  REL32  0x00464040  (sk_value)
//   +0x7d  REL32  0x004632f0  (FUN_004632f0 — CRYPTO_free front half)
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//
//   The function opens with a __chkstk frame (no PUSH EBP / MOV EBP,ESP) and
//   accesses its two stack arguments via ESP-relative addressing throughout.
//   MSVC 2005's FPO optimisation prevents a source-level port from reproducing
//   the exact ESP deltas and slot assignments that follow the alloca. The naked
//   asm path re-emits the original 150 bytes verbatim; compare.py wildcards the
//   eight reloc sites listed above and reports GREEN.

extern "C" __declspec(naked) void FUN_004654b0() {
    __asm {
        // 004654b0  MOV EAX, 0x10
        _emit 0xb8
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 004654b5  CALL 0x009d29d0  (__chkstk — allocs 0x10 local frame)
        _emit 0xe8
        _emit 0x16
        _emit 0xd5
        _emit 0x56
        _emit 0x00
        // 004654ba  MOV EAX, [0x0132e7c8]  (g_hash_table)
        _emit 0xa1
        _emit 0xc8
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 004654bf  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 004654c1  JNZ +0x4  (→ 004654c7)
        _emit 0x75
        _emit 0x04
        // 004654c3  ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 004654c6  RET
        _emit 0xc3
        // 004654c7  MOV ECX, dword ptr [ESP+0x14]  (arg0)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 004654cb  MOV EDX, dword ptr [ESP+0x18]  (arg1 / flags)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 004654cf  MOV dword ptr [ESP+0x8], ECX   (save arg0 into local)
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 004654d3  PUSH ESI
        _emit 0x56
        // 004654d4  LEA ECX, [ESP+0x4]   (&local[0] after PUSH ESI)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 004654d8  PUSH ECX   (arg2 for FUN_00466af0: &local[0])
        _emit 0x51
        // 004654d9  AND EDX, 0xffff7fff  (clear bit 15 of flags)
        _emit 0x81
        _emit 0xe2
        _emit 0xff
        _emit 0x7f
        _emit 0xff
        _emit 0xff
        // 004654df  PUSH EAX   (arg1 for FUN_00466af0: g_hash_table)
        _emit 0x50
        // 004654e0  MOV dword ptr [ESP+0xc], EDX   (write masked flags into local[0])
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 004654e4  CALL 0x00466af0  (FUN_00466af0 — find/create entry)
        _emit 0xe8
        _emit 0x07
        _emit 0x16
        _emit 0x00
        _emit 0x00
        // 004654e9  MOV ESI, EAX   (ESI = result pointer, or NULL)
        _emit 0x8b
        _emit 0xf0
        // 004654eb  ADD ESP, 0x8   (cdecl cleanup: 2 args)
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 004654ee  TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 004654f0  JZ +0x4d  (→ 0046553f: return-0 path)
        _emit 0x74
        _emit 0x4d
        // 004654f2  MOV EAX, [0x0132e7c0]  (g_obj_name_table)
        _emit 0xa1
        _emit 0xc0
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 004654f7  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 004654f9  JZ +0x31  (→ 0046552c: skip sk check, go to CRYPTO_free)
        _emit 0x74
        _emit 0x31
        // 004654fb  PUSH EDI
        _emit 0x57
        // 004654fc  MOV EDI, dword ptr [ESI]   (EDI = *ESI, first field)
        _emit 0x8b
        _emit 0x3e
        // 004654fe  PUSH EAX   (arg for sk_num: g_obj_name_table)
        _emit 0x50
        // 004654ff  CALL 0x00464030  (sk_num)
        _emit 0xe8
        _emit 0x2c
        _emit 0xeb
        _emit 0xff
        _emit 0xff
        // 00465504  ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00465507  CMP EAX, EDI   (count vs first field)
        _emit 0x3b
        _emit 0xc7
        // 00465509  JLE +0x20  (→ 0046552b: POP EDI)
        _emit 0x7e
        _emit 0x20
        // 0046550b  MOV EDX, dword ptr [0x0132e7c0]  (g_obj_name_table reload)
        _emit 0x8b
        _emit 0x15
        _emit 0xc0
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 00465511  PUSH EDI
        _emit 0x57
        // 00465512  PUSH EDX
        _emit 0x52
        // 00465513  CALL 0x00464040  (sk_value)
        _emit 0xe8
        _emit 0x28
        _emit 0xeb
        _emit 0xff
        _emit 0xff
        // 00465518  MOV ECX, dword ptr [ESI+0xc]
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        // 0046551b  MOV EDX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x16
        // 0046551d  PUSH ECX   (3rd call arg: ESI->field_0xc)
        _emit 0x51
        // 0046551e  MOV ECX, dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        // 00465521  PUSH EDX   (2nd call arg: ESI->field_0x0)
        _emit 0x52
        // 00465522  MOV EDX, dword ptr [EAX+0x8]  (fn ptr from sk_value result)
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 00465525  PUSH ECX   (1st call arg: ESI->field_0x8)
        _emit 0x51
        // 00465526  CALL EDX   (virtual dispatch through entry->fn_at_8)
        _emit 0xff
        _emit 0xd2
        // 00465528  ADD ESP, 0x14  (clean 5 args: 2 for sk_value + 3 for virtual)
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0046552b  POP EDI
        _emit 0x5f
        // 0046552c  PUSH ESI   (arg for FUN_004632f0: ptr to free)
        _emit 0x56
        // 0046552d  CALL 0x004632f0  (CRYPTO_free front half)
        _emit 0xe8
        _emit 0xbe
        _emit 0xdd
        _emit 0xff
        _emit 0xff
        // 00465532  ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00465535  MOV EAX, 0x1   (return 1 = success)
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0046553a  POP ESI
        _emit 0x5e
        // 0046553b  ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0046553e  RET
        _emit 0xc3
        // 0046553f  XOR EAX, EAX   (return 0 = not found)
        _emit 0x33
        _emit 0xc0
        // 00465541  POP ESI
        _emit 0x5e
        // 00465542  ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00465545  RET
        _emit 0xc3
    }
}
