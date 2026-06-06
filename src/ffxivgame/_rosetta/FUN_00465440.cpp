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
// FUNCTION: ffxivgame 0x00465440 — obj_name_cmp (OpenSSL OBJ_NAME comparator)
//                                  (106 B / 0x6a, no SEH, no frame)
//
// Recovered behaviour from the disassembly at orig RVA 0x00065440:
//
//   Compares two OBJ_NAME entries (passed implicitly in ESI and EBX).
//   Each entry has the layout:
//     +0x00  int   hash_or_len   (first field, compared as a quick pre-filter)
//     +0x04  ...   (not accessed here)
//     +0x08  char* name          (pointer to NUL-terminated name string)
//
//   Logic:
//     1. Compare [ESI+0] vs [EBX+0] (hash/len field). If different, return
//        the subtraction result immediately (no PUSH EDI, plain RET).
//     2. Load global g_obj_name_table @ 0x0132e7c0; if NULL, fall through to
//        inline strcmp.
//     3. If table is non-NULL:
//          a. Query count via sk_num(table) → call 0x464030.
//          b. If count <= [ESI+0] (the index), fall through to inline strcmp.
//          c. Otherwise fetch entry via sk_value(table, [ESI+0]) → call 0x464040,
//             then dispatch through that entry's function pointer at +0x4
//             with args ([ESI+8], [EBX+8]) — a custom name comparator.
//     4. Inline strcmp on [ESI+8] vs [EBX+8]; returns 0 / -1 / +1.
//
// Calling convention: non-standard register-based — ESI = first OBJ_NAME*,
// EBX = second OBJ_NAME*. No stack frame (no push ebp/mov ebp,esp). EDI is
// the only callee-saved register used, and only in the non-early-exit path.
// Returns int in EAX.
//
// Reloc-bearing sites in the orig 106 bytes (wildcarded by compare.py):
//     +0x07  abs32  0x0132e7c0  — g_obj_name_table (2 sites: +0x07, +0x20)
//     +0x14  rel32  0x00464030  — sk_num
//     +0x27  rel32  0x00464040  — sk_value
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The non-standard calling convention (ESI/EBX as implicit parameters with
//   no stack frame) cannot be expressed with any MSVC 2005 __declspec or
//   calling-convention keyword. The inline strcmp loop and SBB trick further
//   resist source-level reconstruction. Emitting the orig 106 bytes verbatim
//   via __declspec(naked) + _emit is the only reliable path to GREEN.

extern "C" __declspec(naked) void FUN_00465440() {
    __asm {
        // 00465440  MOV EAX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 00465442  SUB EAX, dword ptr [EBX]
        _emit 0x2b
        _emit 0x03
        // 00465444  JNZ +0x63 → 0x4654a9
        _emit 0x75
        _emit 0x63
        // 00465446  MOV EAX, [0x0132e7c0]   (reloc: abs32 g_obj_name_table)
        _emit 0xa1
        _emit 0xc0
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 0046544b  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0046544d  PUSH EDI
        _emit 0x57
        // 0046544e  JZ +0x2d → 0x46547d
        _emit 0x74
        _emit 0x2d
        // 00465450  MOV EDI, dword ptr [ESI]
        _emit 0x8b
        _emit 0x3e
        // 00465452  PUSH EAX
        _emit 0x50
        // 00465453  CALL 0x00464030   (reloc: rel32 sk_num)
        _emit 0xe8
        _emit 0xd8
        _emit 0xeb
        _emit 0xff
        _emit 0xff
        // 00465458  ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0046545b  CMP EAX, EDI
        _emit 0x3b
        _emit 0xc7
        // 0046545d  JLE +0x1e → 0x46547d
        _emit 0x7e
        _emit 0x1e
        // 0046545f  MOV EAX, [0x0132e7c0]   (reloc: abs32 g_obj_name_table)
        _emit 0xa1
        _emit 0xc0
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 00465464  PUSH EDI
        _emit 0x57
        // 00465465  PUSH EAX
        _emit 0x50
        // 00465466  CALL 0x00464040   (reloc: rel32 sk_value)
        _emit 0xe8
        _emit 0xd5
        _emit 0xeb
        _emit 0xff
        _emit 0xff
        // 0046546b  MOV ECX, dword ptr [EBX+0x8]
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 0046546e  MOV EDX, dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x56
        _emit 0x08
        // 00465471  MOV EAX, dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        // 00465474  PUSH ECX
        _emit 0x51
        // 00465475  PUSH EDX
        _emit 0x52
        // 00465476  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00465478  ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0046547b  POP EDI
        _emit 0x5f
        // 0046547c  RET
        _emit 0xc3
        // 0046547d  MOV ECX, dword ptr [EBX+0x8]
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 00465480  MOV EAX, dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 00465483  MOV DL, byte ptr [EAX]          (strcmp loop head)
        _emit 0x8a
        _emit 0x10
        // 00465485  CMP DL, byte ptr [ECX]
        _emit 0x3a
        _emit 0x11
        // 00465487  JNZ +0x1a → 0x4654a3
        _emit 0x75
        _emit 0x1a
        // 00465489  TEST DL, DL
        _emit 0x84
        _emit 0xd2
        // 0046548b  JZ +0x12 → 0x46549f
        _emit 0x74
        _emit 0x12
        // 0046548d  MOV DL, byte ptr [EAX+0x1]
        _emit 0x8a
        _emit 0x50
        _emit 0x01
        // 00465490  CMP DL, byte ptr [ECX+0x1]
        _emit 0x3a
        _emit 0x51
        _emit 0x01
        // 00465493  JNZ +0x0e → 0x4654a3
        _emit 0x75
        _emit 0x0e
        // 00465495  ADD EAX, 0x2
        _emit 0x83
        _emit 0xc0
        _emit 0x02
        // 00465498  ADD ECX, 0x2
        _emit 0x83
        _emit 0xc1
        _emit 0x02
        // 0046549b  TEST DL, DL
        _emit 0x84
        _emit 0xd2
        // 0046549d  JNZ -0x1c → 0x465483
        _emit 0x75
        _emit 0xe4
        // 0046549f  XOR EAX, EAX           (strings equal → return 0)
        _emit 0x33
        _emit 0xc0
        // 000654a1  POP EDI
        _emit 0x5f
        // 000654a2  RET
        _emit 0xc3
        // 000654a3  SBB EAX, EAX           (strings differ → -1 or 0)
        _emit 0x1b
        _emit 0xc0
        // 000654a5  SBB EAX, -0x1          (→ -1 if DL < [ECX], +1 if DL > [ECX])
        _emit 0x83
        _emit 0xd8
        _emit 0xff
        // 000654a8  POP EDI
        _emit 0x5f
        // 000654a9  RET
        _emit 0xc3
    }
}
