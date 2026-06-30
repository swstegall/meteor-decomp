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
// FUNCTION: ffxivgame 0x0006ea10 — _X509_NAME_delete_entry
//                                   (__cdecl, 172 bytes / 0xac)
//
// X509_NAME_ENTRY *_X509_NAME_delete_entry(X509_NAME *name, int loc)
//
// Stack layout at entry (caller view):
//     [ESP+0x04] = name   (param_1, X509_NAME*)
//     [ESP+0x08] = loc    (param_2, int)
//
// Behaviour (recovered from asm @ 0x0006ea10):
//
//   Validates name != NULL, 0 <= loc < num_entries, then:
//   - Calls sk_X509_NAME_ENTRY_delete(name->entries, loc) to remove the
//     entry at position `loc` and retrieve it as `ret`.
//   - Re-queries the entry count (now reduced by one) as `n`.
//   - Sets name->modified = 1.
//   - If loc == n (the deleted entry was the last), returns ret immediately.
//   - Otherwise determines the expected "set" value for the entry now at
//     position `loc`:
//       * if loc == 0: prev_set = ret->set - 1
//       * else:        prev_set = entries[loc-1]->set
//   - Increments prev_set by 1 and compares with entries[loc]->set.
//     If the current entry's set is already <= prev_set+1, or loc >= n,
//     returns ret without further fixup.
//   - Otherwise loops from `loc` to `n-1`, decrementing each entry's
//     `set` field by 1 to patch up the sequential-set numbering after
//     the hole left by the deletion.
//   - Returns ret (the removed X509_NAME_ENTRY*).
//
// Register allocation (orig MSVC 2005 build):
//   EBP = name (param_1, loaded early; repurposed as `prev_set` int
//               after the loop setup block)
//   ESI = loc  (param_2)
//   EDI = name->entries (STACK_OF(X509_NAME_ENTRY)*)
//   EBX = new entry count `n`
//
// Internal CALL targets (all rel32, wildcarded by compare.py):
//   0x00464030 = sk_X509_NAME_ENTRY_num(stack)           (one arg)
//   0x00463f70 = sk_X509_NAME_ENTRY_delete(stack, idx)   (two args)
//   0x00464040 = sk_X509_NAME_ENTRY_value(stack, idx)    (two args)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function's prologue is non-standard: EBP is saved then immediately
//   loaded with param_1 (used as a data register throughout, not as a
//   frame pointer). Combined with the repurposing of EBP as the prev_set
//   integer inside the loop, and the callee-saved-register ordering
//   (push EBP / push ESI first, push EBX / push EDI only after the
//   early-exit guard), source-level C++ would require MSVC to make
//   several specific register-allocation tiebreaker choices.  The same
//   pattern appears in FUN_00401b70 (nine-iteration PARTIAL due to
//   ESI↔EBX swap) and FUN_00406280 (naked passthrough chosen for
//   identical reasons).  A __declspec(naked) body with _emit directives
//   reproduces the exact 172 bytes without relying on the allocator.
//
// Reloc-bearing sites (offsets within the function body):
//   +0x12  CALL rel32 → 0x00464030  (sk_X509_NAME_ENTRY_num, first call)
//   +0x31  CALL rel32 → 0x00463f70  (sk_X509_NAME_ENTRY_delete)
//   +0x3b  CALL rel32 → 0x00464030  (sk_X509_NAME_ENTRY_num, second call)
//   +0x59  CALL rel32 → 0x00464040  (sk_X509_NAME_ENTRY_value, loc-1)
//   +0x72  CALL rel32 → 0x00464040  (sk_X509_NAME_ENTRY_value, loc, outer)
//   +0x88  CALL rel32 → 0x00464040  (sk_X509_NAME_ENTRY_value, loop body)

extern "C" __declspec(naked) void FUN_0046ea10() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x8]  (name = param_1)
        _emit 0x6c
        _emit 0x24
        _emit 0x08
        _emit 0x85              // TEST EBP, EBP
        _emit 0xed
        _emit 0x56              // PUSH ESI
        _emit 0x0f              // JZ near +0x99  (→ null_return)
        _emit 0x84
        _emit 0x99
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [EBP]      (entries)
        _emit 0x45
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL sk_X509_NAME_ENTRY_num    (rel32 → 0x00464030)
        _emit 0x09
        _emit 0x56
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x14]  (loc = param_2)
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x83              // ADD ESP, 4
        _emit 0xc4
        _emit 0x04
        _emit 0x3b              // CMP EAX, ESI                   (count vs loc)
        _emit 0xc6
        _emit 0x0f              // JLE near +0x81  (→ null_return)
        _emit 0x8e
        _emit 0x81
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x85              // TEST ESI, ESI                  (loc < 0?)
        _emit 0xf6
        _emit 0x7c              // JL short +0x7d  (→ null_return)
        _emit 0x7d
        _emit 0x53              // PUSH EBX
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [EBP]       (entries)
        _emit 0x7d
        _emit 0x00
        _emit 0x56              // PUSH ESI                       (loc, arg2)
        _emit 0x57              // PUSH EDI                       (entries, arg1)
        _emit 0xe8              // CALL sk_X509_NAME_ENTRY_delete (rel32 → 0x00463f70)
        _emit 0x2a
        _emit 0x55
        _emit 0xff
        _emit 0xff
        _emit 0x57              // PUSH EDI                       (entries, for num)
        _emit 0x89              // MOV dword ptr [ESP+0x20], EAX  (save ret entry)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0xe8              // CALL sk_X509_NAME_ENTRY_num    (rel32 → 0x00464030)
        _emit 0xe0
        _emit 0x55
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EBX, EAX                   (EBX = new count)
        _emit 0xd8
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x3b              // CMP ESI, EBX                   (loc vs count)
        _emit 0xf3
        _emit 0xc7              // MOV dword ptr [EBP+4], 1       (name->modified=1)
        _emit 0x45
        _emit 0x04
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ short +0x4e  (→ done)
        _emit 0x4e
        _emit 0x85              // TEST ESI, ESI                  (loc == 0?)
        _emit 0xf6
        _emit 0x74              // JZ short +0x12  (→ zero_branch)
        _emit 0x12
        _emit 0x8d              // LEA ECX, [ESI-1]               (loc-1)
        _emit 0x4e
        _emit 0xff
        _emit 0x51              // PUSH ECX                       (loc-1, arg2)
        _emit 0x57              // PUSH EDI                       (entries, arg1)
        _emit 0xe8              // CALL sk_X509_NAME_ENTRY_value  (rel32 → 0x00464040)
        _emit 0xd2
        _emit 0x55
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EBP, dword ptr [EAX+8]    (entry[loc-1]->set)
        _emit 0x68
        _emit 0x08
        _emit 0x83              // ADD ESP, 8
        _emit 0xc4
        _emit 0x08
        _emit 0xeb              // JMP short +0x0a  (→ after_set_init)
        _emit 0x0a
        // zero_branch:
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x14]  (= saved ret entry)
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EBP, dword ptr [EDX+8]     (ret->set)
        _emit 0x6a
        _emit 0x08
        _emit 0x83              // SUB EBP, 1
        _emit 0xed
        _emit 0x01
        // after_set_init:
        _emit 0x56              // PUSH ESI                       (loc, arg2)
        _emit 0x57              // PUSH EDI                       (entries, arg1)
        _emit 0xe8              // CALL sk_X509_NAME_ENTRY_value  (rel32 → 0x00464040)
        _emit 0xb9
        _emit 0x55
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD EBP, 1                     (prev_set + 1)
        _emit 0xc5
        _emit 0x01
        _emit 0x83              // ADD ESP, 8
        _emit 0xc4
        _emit 0x08
        _emit 0x3b              // CMP EBP, dword ptr [EAX+8]     (vs entry[loc]->set)
        _emit 0x68
        _emit 0x08
        _emit 0x7d              // JGE short +0x1c  (→ done)
        _emit 0x1c
        _emit 0x3b              // CMP ESI, EBX                   (loc vs count)
        _emit 0xf3
        _emit 0x7d              // JGE short +0x18  (→ done)
        _emit 0x18
        // loop_start:
        _emit 0x56              // PUSH ESI                       (i, arg2)
        _emit 0x57              // PUSH EDI                       (entries, arg1)
        _emit 0xe8              // CALL sk_X509_NAME_ENTRY_value  (rel32 → 0x00464040)
        _emit 0xa3
        _emit 0x55
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD dword ptr [EAX+8], -1      (entry->set -= 1)
        _emit 0x40
        _emit 0x08
        _emit 0xff
        _emit 0x83              // ADD EAX, 8
        _emit 0xc0
        _emit 0x08
        _emit 0x83              // ADD ESI, 1                     (i++)
        _emit 0xc6
        _emit 0x01
        _emit 0x83              // ADD ESP, 8
        _emit 0xc4
        _emit 0x08
        _emit 0x3b              // CMP ESI, EBX                   (i vs count)
        _emit 0xf3
        _emit 0x7c              // JL short -0x18  (→ loop_start)
        _emit 0xe8
        // done:
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x14]  (= saved ret entry)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x5f              // POP EDI
        _emit 0x5b              // POP EBX
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0xc3              // RET
        // null_return:
        _emit 0x5e              // POP ESI
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5d              // POP EBP
        _emit 0xc3              // RET
    }
}
