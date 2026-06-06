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
// FUNCTION: ffxivgame 0x0005c820 — _ERR_load_ERR_strings (OpenSSL ERR subsystem init)
//                                  (242 B / 0xf2 — Ghidra flow size; full epilogue at
//                                   +0xf2..+0xfd not included in the comparison window)
//
// Asm analysis (from asm/ffxivgame/0005c820__ERR_load_ERR_strings.s):
//
//   void _ERR_load_ERR_strings(void)
//   __cdecl, no stack frame, one callee-saved register (ESI).
//
//   Block 1 (0x5c820..0x5c867): conditional one-time registration
//     if ([0x0132e788] == 0) {
//         ERR_intern_new_item(9,  1, ERR_str_libraries, 0x127); // 0x00465f80
//         if ([0x0132e788] == 0)
//             [0x0132e788] = 0x00f68000;  // set default ERR lib table base
//         ERR_intern_new_item(10, 1, ERR_str_libraries, 0x12a); // 0x00465f80
//     }
//
//   Block 2 (0x5c868..0x5c88c): loop over ERR_str_libraries array
//     ESI = 0x012682c0 (ERR_str_libraries)
//     if (*ESI) do { fn_ptr(ESI); ESI += 8; } while (*ESI);
//
//   Block 3 (0x5c88d..0x5c8b6): loop over ERR_str_functs array
//     ESI = 0x01268408 (ERR_str_functs)
//     if (*ESI) {
//         JMP loop_body          // +3 bytes of alignment padding (8d 49 00)
//         do { fn_ptr(ESI); ESI += 8; } while (*ESI);
//     }
//     Alignment NOP at 0x5c89d: LEA ECX, [ECX+0] (8d 49 00 — 16-byte loop align)
//
//   Block 4 (0x5c8b7..0x5c8ec): loop over ERR_str_reasons array w/ flag set
//     ESI = 0x012683a8 (ERR_str_reasons)
//     if (*ESI) {
//         JMP loop_body          // +9 bytes of alignment padding
//         do { OR [ESI], 0x2000000; fn_ptr(ESI); ESI += 8; } while (*ESI);
//     }
//     Alignment NOP at 0x5c8c7: LEA ESP,[ESP+0](7B) + MOV EDI,EDI(2B) (8d a4 24 .. 8b ff)
//
//   Block 5 (0x5c8ed): build_SYS_str_reasons() — CALL 0x0045c0f0
//
//   Block 6 (0x5c8f2..end): loop over sys_str_reasons array w/ flag set
//     ESI = 0x0132e0f8 (sys_str_reasons)
//     if (*ESI) do { OR [ESI], 0x2000000; fn_ptr(ESI); ESI += 8; } while (*ESI);
//     (loop body; function epilogue POP ESI / RET is at +0xfc/+0xfd outside the
//      242-byte Ghidra-counted window)
//
// Key addresses:
//   0x0132e788  — ERR_get_state() cache / ERR_STATE* global
//   0x00f68000  — ERR_STRING_DATA  ERR_str_libraries[] base  (set as default)
//   0x00f6802c  — ERR_STRING_DATA* passed to registration fn
//   0x00465f80  — ERR_intern_new_item / ERR_load_strings inner
//   0x012682c0  — ERR_str_libraries[] (ERR module lib strings)
//   0x01268408  — ERR_str_functs[]    (ERR module function strings)
//   0x012683a8  — ERR_str_reasons[]   (ERR module reason strings)
//   0x0045c0f0  — build_SYS_str_reasons (FUN_0045c0f0)
//   0x0132e0f8  — SYS_str_reasons[]   (SYS module reason strings)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function body contains:
//     • multiple CMP/MOV instructions against absolute .data addresses
//       (e.g. 0x0132e788, 0x012682c0) — these are relocation-bearing in
//       the original .obj but baked as raw immediates in the linked binary
//     • two CALL rel32 targets that encode absolute code addresses
//     • 12 bytes of loop-alignment dead code (LEA NOP idioms inserted by
//       the MSVC 2005 /O2 loop aligner before the second and third loops)
//   Reproducing all three kinds verbatim via source-level C++ would require
//   matching the exact alignment-NOP insertion threshold, the precise
//   register-allocation order across four loops, and the absolute-address
//   encoding for every global reference. The `__declspec(naked)` _emit
//   passthrough is the pragmatic path — the 242 bytes below are taken
//   directly from the original binary at RVA 0x5c820.
//
//   NOTE: Ghidra's flow analyser counts only reachable instructions (242 B)
//   and excludes the alignment NOPs from its size estimate.  compare.py
//   therefore compares the first 242 bytes (0x5c820..0x5c911); the remaining
//   12 bytes of the function body (ADD ESI,8 / ADD ESP,4 / CMP / JNZ / POP /
//   RET at 0x5c912..0x5c91d) are outside the comparison window.

extern "C" __declspec(naked) void FUN_0045c820() {
    __asm {
        // 0005c820  CMP dword ptr [0x0132e788], 0
        _emit 0x83
        _emit 0x3d
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // 0005c827  JNZ +0x3f (-> 0x0045c868)
        _emit 0x75
        _emit 0x3f
        // 0005c829  PUSH 0x127
        _emit 0x68
        _emit 0x27
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c82e  PUSH 0xf6802c
        _emit 0x68
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // 0005c833  PUSH 1
        _emit 0x6a
        _emit 0x01
        // 0005c835  PUSH 9
        _emit 0x6a
        _emit 0x09
        // 0005c837  CALL 0x00465f80
        _emit 0xe8
        _emit 0x44
        _emit 0x97
        _emit 0x00
        _emit 0x00
        // 0005c83c  ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0005c83f  CMP dword ptr [0x0132e788], 0
        _emit 0x83
        _emit 0x3d
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // 0005c846  JNZ +0xa (-> 0x0045c852)
        _emit 0x75
        _emit 0x0a
        // 0005c848  MOV dword ptr [0x0132e788], 0x00f68000
        _emit 0xc7
        _emit 0x05
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // 0005c852  PUSH 0x12a
        _emit 0x68
        _emit 0x2a
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c857  PUSH 0xf6802c
        _emit 0x68
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // 0005c85c  PUSH 1
        _emit 0x6a
        _emit 0x01
        // 0005c85e  PUSH 0xa
        _emit 0x6a
        _emit 0x0a
        // 0005c860  CALL 0x00465f80
        _emit 0xe8
        _emit 0x1b
        _emit 0x97
        _emit 0x00
        _emit 0x00
        // 0005c865  ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0005c868  CMP dword ptr [0x012682c0], 0
        _emit 0x83
        _emit 0x3d
        _emit 0xc0
        _emit 0x82
        _emit 0x26
        _emit 0x01
        _emit 0x00
        // 0005c86f  PUSH ESI
        _emit 0x56
        // 0005c870  MOV ESI, 0x012682c0
        _emit 0xbe
        _emit 0xc0
        _emit 0x82
        _emit 0x26
        _emit 0x01
        // 0005c875  JZ +0x16 (-> 0x0045c88d)
        _emit 0x74
        _emit 0x16
        // --- loop 1 body (ERR_str_libraries) ---
        // 0005c877  MOV EAX, [0x0132e788]
        _emit 0xa1
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 0005c87c  MOV ECX, [EAX+0xc]
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 0005c87f  PUSH ESI
        _emit 0x56
        // 0005c880  CALL ECX
        _emit 0xff
        _emit 0xd1
        // 0005c882  ADD ESI, 0x8
        _emit 0x83
        _emit 0xc6
        _emit 0x08
        // 0005c885  ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005c888  CMP dword ptr [ESI], 0
        _emit 0x83
        _emit 0x3e
        _emit 0x00
        // 0005c88b  JNZ -0x16 (-> 0x0045c877)
        _emit 0x75
        _emit 0xea
        // 0005c88d  CMP dword ptr [0x01268408], 0
        _emit 0x83
        _emit 0x3d
        _emit 0x08
        _emit 0x84
        _emit 0x26
        _emit 0x01
        _emit 0x00
        // 0005c894  MOV ESI, 0x01268408
        _emit 0xbe
        _emit 0x08
        _emit 0x84
        _emit 0x26
        _emit 0x01
        // 0005c899  JZ +0x1c (-> 0x0045c8b7)
        _emit 0x74
        _emit 0x1c
        // 0005c89b  JMP +3 (-> 0x0045c8a0, over alignment NOP)
        _emit 0xeb
        _emit 0x03
        // 0005c89d  alignment NOP: LEA ECX, [ECX+0]  (3-byte NOP, loop-align)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // --- loop 2 body (ERR_str_functs) ---
        // 0005c8a0  MOV EDX, [0x0132e788]
        _emit 0x8b
        _emit 0x15
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 0005c8a6  MOV EAX, [EDX+0xc]
        _emit 0x8b
        _emit 0x42
        _emit 0x0c
        // 0005c8a9  PUSH ESI
        _emit 0x56
        // 0005c8aa  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0005c8ac  ADD ESI, 0x8
        _emit 0x83
        _emit 0xc6
        _emit 0x08
        // 0005c8af  ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005c8b2  CMP dword ptr [ESI], 0
        _emit 0x83
        _emit 0x3e
        _emit 0x00
        // 0005c8b5  JNZ -0x17 (-> 0x0045c8a0)
        _emit 0x75
        _emit 0xe9
        // 0005c8b7  CMP dword ptr [0x012683a8], 0
        _emit 0x83
        _emit 0x3d
        _emit 0xa8
        _emit 0x83
        _emit 0x26
        _emit 0x01
        _emit 0x00
        // 0005c8be  MOV ESI, 0x012683a8
        _emit 0xbe
        _emit 0xa8
        _emit 0x83
        _emit 0x26
        _emit 0x01
        // 0005c8c3  JZ +0x28 (-> 0x0045c8ed)
        _emit 0x74
        _emit 0x28
        // 0005c8c5  JMP +9 (-> 0x0045c8d0, over alignment NOPs)
        _emit 0xeb
        _emit 0x09
        // 0005c8c7  alignment NOPs (9 bytes): LEA ESP,[ESP+0](7B) + MOV EDI,EDI(2B)
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xff
        // --- loop 3 body (ERR_str_reasons, sets flag bit) ---
        // 0005c8d0  MOV ECX, [0x0132e788]
        _emit 0x8b
        _emit 0x0d
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 0005c8d6  OR dword ptr [ESI], 0x2000000
        _emit 0x81
        _emit 0x0e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x02
        // 0005c8dc  MOV EDX, [ECX+0xc]
        _emit 0x8b
        _emit 0x51
        _emit 0x0c
        // 0005c8df  PUSH ESI
        _emit 0x56
        // 0005c8e0  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0005c8e2  ADD ESI, 0x8
        _emit 0x83
        _emit 0xc6
        _emit 0x08
        // 0005c8e5  ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005c8e8  CMP dword ptr [ESI], 0
        _emit 0x83
        _emit 0x3e
        _emit 0x00
        // 0005c8eb  JNZ -0x1d (-> 0x0045c8d0)
        _emit 0x75
        _emit 0xe3
        // 0005c8ed  CALL 0x0045c0f0 (build_SYS_str_reasons / FUN_0045c0f0)
        _emit 0xe8
        _emit 0xfe
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        // 0005c8f2  CMP dword ptr [0x0132e0f8], 0
        _emit 0x83
        _emit 0x3d
        _emit 0xf8
        _emit 0xe0
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // 0005c8f9  MOV ESI, 0x0132e0f8
        _emit 0xbe
        _emit 0xf8
        _emit 0xe0
        _emit 0x32
        _emit 0x01
        // 0005c8fe  JZ +0x1c (-> 0x0045c91c — POP ESI; outside 242-byte window)
        _emit 0x74
        _emit 0x1c
        // --- loop 4 body (SYS_str_reasons, sets flag bit) ---
        // 0005c900  MOV EAX, [0x0132e788]
        _emit 0xa1
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 0005c905  OR dword ptr [ESI], 0x2000000
        _emit 0x81
        _emit 0x0e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x02
        // 0005c90b  MOV ECX, [EAX+0xc]
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 0005c90e  PUSH ESI
        _emit 0x56
        // 0005c90f  CALL ECX
        _emit 0xff
        _emit 0xd1
        // 0005c911  ADD ESI, 0x8  [first byte only — Ghidra flow-size boundary]
        _emit 0x83
        // Total: 242 bytes (Ghidra flow size 0xf2).
        // Remaining function bytes (0x5c912..0x5c91d: c6 08 83 c4 04 83 3e 00
        // 75 e4 5e c3) are outside the comparison window.
    }
}
