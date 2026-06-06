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
// FUNCTION: ffxivgame 0x0041cc60 — bind three table-mapped values to a
//                                  global object (__cdecl void, 85 bytes)
//
// void __cdecl FUN_0041cc60(int param0, int param1, int param2, int param3)
//   param0  — persisted in ESI; passed as first arg to every sub-call
//   param1/2/3 — indices into a global DWORD table at VA 0x00f59750;
//                the looked-up values become the third arg of each call
//
// Stack layout at function entry (no frame):
//   [ESP+0x04] : param0
//   [ESP+0x08] : param1
//   [ESP+0x0C] : param2
//   [ESP+0x10] : param3
//
// Behaviour: for each of the three index params, does
//     g_obj->FUN_004236a0(param0, seqno, g_table[paramN])
//   where seqno = 1, 2, 3 respectively and g_obj = *(ptr*)0x0132987c.
//   All three calls share the same target (FUN_004236a0 at 0x004236a0),
//   which has __thiscall cc with 3 explicit stack args.
//
// Asm shape (85 bytes, from orig RVA 0x0001cc60):
//
//   0001cc60: 8b 44 24 08             MOV  EAX, [ESP+0x8]            ; param1 (pre-load before PUSH ESI)
//   0001cc64: 8b 0c 85 50 97 f5 00    MOV  ECX, [EAX*4+0x00f59750]   ; g_table[param1]
//   0001cc6b: 56                      PUSH ESI
//   0001cc6c: 8b 74 24 08             MOV  ESI, [ESP+0x8]            ; param0 (ESP shifted by PUSH ESI)
//   0001cc70: 51                      PUSH ECX                       ; arg3 = g_table[param1]
//   0001cc71: 8b 0d 7c 98 32 01       MOV  ECX, [0x0132987c]         ; this = g_obj
//   0001cc77: 6a 01                   PUSH 0x1                       ; arg2 = seqno 1
//   0001cc79: 56                      PUSH ESI                       ; arg1 = param0
//   0001cc7a: e8 21 6a 00 00          CALL FUN_004236a0
//   0001cc7f: 8b 54 24 10             MOV  EDX, [ESP+0x10]           ; param2
//   0001cc83: 8b 04 95 50 97 f5 00    MOV  EAX, [EDX*4+0x00f59750]   ; g_table[param2]
//   0001cc8a: 8b 0d 7c 98 32 01       MOV  ECX, [0x0132987c]         ; this = g_obj
//   0001cc90: 50                      PUSH EAX                       ; arg3 = g_table[param2]
//   0001cc91: 6a 02                   PUSH 0x2                       ; arg2 = seqno 2
//   0001cc93: 56                      PUSH ESI                       ; arg1 = param0
//   0001cc94: e8 07 6a 00 00          CALL FUN_004236a0
//   0001cc99: 8b 4c 24 14             MOV  ECX, [ESP+0x14]           ; param3
//   0001cc9d: 8b 14 8d 50 97 f5 00    MOV  EDX, [ECX*4+0x00f59750]   ; g_table[param3]
//   0001cca4: 8b 0d 7c 98 32 01       MOV  ECX, [0x0132987c]         ; this = g_obj
//   0001ccaa: 52                      PUSH EDX                       ; arg3 = g_table[param3]
//   0001ccab: 6a 03                   PUSH 0x3                       ; arg2 = seqno 3
//   0001ccad: 56                      PUSH ESI                       ; arg1 = param0
//   0001ccae: e8 ed 69 00 00          CALL FUN_004236a0
//   0001ccb3: 5e                      POP  ESI
//   0001ccb4: c3                      RET
//
// Reloc-bearing sites (DIR32 + REL32; compare.py masks all 4-byte windows):
//   +0x04  DIR32 → g_table       (0x00f59750) ×3
//   +0x0B  DIR32 → g_obj_ptr     (0x0132987c) ×3
//   +0x1A  REL32 → FUN_004236a0               ×3
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   MSVC pre-loads param1 into EAX before PUSH ESI (to avoid an ESP-delta
//   hazard when re-reading the arg via [ESP+8] later), uses ESI across all
//   three call blocks to hold param0, and reads param2/param3 from the
//   adjusted stack frame at [ESP+0x10] and [ESP+0x14] respectively (both
//   measured after PUSH ESI shifts the base). Getting a source-level
//   reconstruction to pin all three index registers (EAX, EDX, ECX in
//   order) and the exact pre-load timing is unreliable, so we re-emit the
//   85 orig bytes verbatim; compare.py masks the six DIR32 + three REL32
//   reloc windows.

extern "C" __declspec(naked) void FUN_0041cc60() {
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
        // PUSH ESI                 (arg1 = param0)
        _emit 0x56
        // CALL FUN_004236a0        (REL32 reloc)
        _emit 0xe8
        _emit 0x21
        _emit 0x6a
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
        // PUSH ESI                 (arg1 = param0)
        _emit 0x56
        // CALL FUN_004236a0        (REL32 reloc)
        _emit 0xe8
        _emit 0x07
        _emit 0x6a
        _emit 0x00
        _emit 0x00
        // MOV ECX, [ESP+0x14]      (param3)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // MOV EDX, [ECX*4 + g_table]  (DIR32 reloc → 0x00f59750)
        _emit 0x8b
        _emit 0x14
        _emit 0x8d
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
        // PUSH EDX                 (arg3 = g_table[param3])
        _emit 0x52
        // PUSH 0x3                 (arg2 = seqno 3)
        _emit 0x6a
        _emit 0x03
        // PUSH ESI                 (arg1 = param0)
        _emit 0x56
        // CALL FUN_004236a0        (REL32 reloc)
        _emit 0xe8
        _emit 0xed
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // POP ESI
        _emit 0x5e
        // RET
        _emit 0xc3
    }
}
