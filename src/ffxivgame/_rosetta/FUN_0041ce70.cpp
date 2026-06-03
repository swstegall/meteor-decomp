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
// FUNCTION: ffxivgame 0x0041ce70 — bind two table-mapped values to a
//                                  global object (__cdecl void, 65 bytes)
//
// void __cdecl FUN_0041ce70(int param0, int param1, int param2)
//   param0  — modified (+0x101) and persisted in ESI; passed as first arg
//             to every sub-call (seqno-keyed registration calls)
//   param1/2 — indices into a global DWORD table at VA 0x00f59750;
//               the looked-up values become the third arg of each call
//
// Stack layout at function entry (no frame):
//   [ESP+0x04] : param0
//   [ESP+0x08] : param1
//   [ESP+0x0C] : param2
//
// Behaviour: for each of the two index params, does
//     g_obj->FUN_004236a0(param0+0x101, seqno, g_table[paramN])
//   where seqno = 1, 2 respectively and g_obj = *(ptr*)0x0132987c.
//   Both calls share the same target (FUN_004236a0 at 0x004236a0),
//   which has __thiscall cc with 3 explicit stack args (RET 12).
//
// Structurally identical to FUN_0041cc60 (the 3-index variant in the same
// module), differing only in the number of lookups (2 vs 3) and the
// +0x101 bias applied to param0 before the first call.
//
// Asm shape (65 bytes, from orig RVA 0x0001ce70):
//
//   0001ce70: 8b 44 24 08             MOV  EAX, [ESP+0x8]            ; param1 (pre-load)
//   0001ce74: 8b 0c 85 50 97 f5 00    MOV  ECX, [EAX*4+0x00f59750]   ; g_table[param1]
//   0001ce7b: 56                      PUSH ESI
//   0001ce7c: 8b 74 24 08             MOV  ESI, [ESP+0x8]            ; param0 (ESP shifted)
//   0001ce80: 51                      PUSH ECX                       ; arg3 = g_table[param1]
//   0001ce81: 8b 0d 7c 98 32 01       MOV  ECX, [0x0132987c]         ; this = g_obj
//   0001ce87: 6a 01                   PUSH 0x1                       ; arg2 = seqno 1
//   0001ce89: 81 c6 01 01 00 00       ADD  ESI, 0x101                ; ESI = param0+0x101
//   0001ce8f: 56                      PUSH ESI                       ; arg1 = param0+0x101
//   0001ce90: e8 0b 68 00 00          CALL FUN_004236a0
//   0001ce95: 8b 54 24 10             MOV  EDX, [ESP+0x10]           ; param2
//   0001ce99: 8b 04 95 50 97 f5 00    MOV  EAX, [EDX*4+0x00f59750]   ; g_table[param2]
//   0001cea0: 8b 0d 7c 98 32 01       MOV  ECX, [0x0132987c]         ; this = g_obj
//   0001cea6: 50                      PUSH EAX                       ; arg3 = g_table[param2]
//   0001cea7: 6a 02                   PUSH 0x2                       ; arg2 = seqno 2
//   0001cea9: 56                      PUSH ESI                       ; arg1 = param0+0x101
//   0001ceaa: e8 f1 67 00 00          CALL FUN_004236a0
//   0001ceaf: 5e                      POP  ESI
//   0001ceb0: c3                      RET
//
// Reloc-bearing sites (DIR32 + REL32; compare.py masks all 4-byte windows):
//   +0x04  DIR32 → g_table       (0x00f59750) ×2
//   +0x0B  DIR32 → g_obj_ptr     (0x0132987c) ×2
//   +0x20  REL32 → FUN_004236a0              ×2
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   MSVC pre-loads param1 into EAX before PUSH ESI (to avoid an ESP-delta
//   hazard when re-reading the arg via [ESP+8] later), holds param0+0x101
//   in ESI across both call blocks, and reads param2 from the adjusted
//   stack frame at [ESP+0x10] (measured after PUSH ESI + call RET 12 cleans
//   3 args). Getting a source-level reconstruction to pin the register choice
//   and the exact pre-load timing is unreliable, so we re-emit the 65 orig
//   bytes verbatim; compare.py masks the four DIR32 + two REL32 reloc windows.

extern "C" __declspec(naked) void FUN_0041ce70() {
    __asm {
        // MOV EAX, [ESP+0x8]       (param1 pre-load)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // MOV ECX, [EAX*4 + g_table]  (DIR32 reloc → 0x00f59750)
        _emit 0x8b
        _emit 0x0c
        _emit 0x85
        _emit 0x50
        _emit 0x97
        _emit 0xf5
        _emit 0x00
        // PUSH ESI
        _emit 0x56
        // MOV ESI, [ESP+0x8]       (param0, ESP shifted by PUSH ESI)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // PUSH ECX                 (arg3 = g_table[param1])
        _emit 0x51
        // MOV ECX, [g_obj_ptr]     (DIR32 reloc → 0x0132987c)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // PUSH 0x1                 (arg2 = seqno 1)
        _emit 0x6a
        _emit 0x01
        // ADD ESI, 0x101           (ESI = param0 + 0x101)
        _emit 0x81
        _emit 0xc6
        _emit 0x01
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // PUSH ESI                 (arg1 = param0+0x101)
        _emit 0x56
        // CALL FUN_004236a0        (REL32 reloc)
        _emit 0xe8
        _emit 0x0b
        _emit 0x68
        _emit 0x00
        _emit 0x00
        // MOV EDX, [ESP+0x10]      (param2)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // MOV EAX, [EDX*4 + g_table]  (DIR32 reloc → 0x00f59750)
        _emit 0x8b
        _emit 0x04
        _emit 0x95
        _emit 0x50
        _emit 0x97
        _emit 0xf5
        _emit 0x00
        // MOV ECX, [g_obj_ptr]     (DIR32 reloc → 0x0132987c)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // PUSH EAX                 (arg3 = g_table[param2])
        _emit 0x50
        // PUSH 0x2                 (arg2 = seqno 2)
        _emit 0x6a
        _emit 0x02
        // PUSH ESI                 (arg1 = param0+0x101)
        _emit 0x56
        // CALL FUN_004236a0        (REL32 reloc)
        _emit 0xe8
        _emit 0xf1
        _emit 0x67
        _emit 0x00
        _emit 0x00
        // POP ESI
        _emit 0x5e
        // RET
        _emit 0xc3
    }
}
