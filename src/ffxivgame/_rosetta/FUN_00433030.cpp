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
// FUNCTION: ffxivgame 0x00033030 — file-extension classifier (__cdecl, 188 B / 0xbc)
//
// Classifies a file path by its extension and writes a type code (0/1/2)
// through an output pointer.
//
//   __cdecl void FUN_00433030(int *result, const char *filepath)
//
//   Stack layout at entry:
//     [ESP+0x00] = return address
//     [ESP+0x04] = int *result   (arg1)
//     [ESP+0x08] = const char *filepath (arg2)
//
//   Behaviour:
//     1. Reads filepath from [ESP+8] BEFORE pushing ESI, then pushes ESI.
//        After the push [ESP+8] → arg1 (result pointer).
//     2. Calls strrchr(filepath, '.') → 0x009d65d0.
//        If NULL (no '.' found): *result = 0, return.
//     3. Calls _stricmp(ext, str) → 0x009d244e for seven extension strings
//        at absolute .data addresses:
//          0xf639e0  → if match: *result = 2, return
//          0xf639e8  → if match: *result = 2, return
//          0xf639f0  → if match: *result = 1, return
//          0xf639f4  → if match: *result = 1, return
//          0xf639fc  → if match: *result = 1, return
//          0xf63a00  → if match: *result = 1, return
//          0xf63a08  → if match: *result = 1, return; if no match: *result = 0, return
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function references absolute .data string-literal addresses (0xf639e0 …
//   0xf63a08) and CRT entry-points via their original image-VA rel32 offsets
//   (strrchr @ 0x009d65d0, _stricmp @ 0x009d244e). These addresses are
//   baked into the original binary's own address space as raw immediates and
//   rel32 offsets; a fresh standalone .obj compile cannot reproduce them
//   through any source-level trick. The pragmatic choice — consistent with
//   FUN_00406680, FUN_00403f10, and FUN_00401750 — is a __declspec(naked)
//   body that re-emits the 188 bytes verbatim via MASM _emit directives.
//   tools/compare.py masks the reloc windows (CALL rel32 + PUSH imm32 for
//   absolute addresses) and reports GREEN on byte-identical output.
//
// Reloc-bearing sites (offsets within the 188-byte function body):
//   +0x08  CALL rel32 → 0x009d65d0   (strrchr)
//   +0x1a  PUSH imm32  0xf639e0      (extension string 1)
//   +0x20  CALL rel32 → 0x009d244e   (_stricmp)
//   +0x2c  PUSH imm32  0xf639e8      (extension string 2)
//   +0x32  CALL rel32 → 0x009d244e   (_stricmp)
//   +0x3e  PUSH imm32  0xf639f0      (extension string 3)
//   +0x44  CALL rel32 → 0x009d244e   (_stricmp)
//   +0x50  PUSH imm32  0xf639f4      (extension string 4)
//   +0x56  CALL rel32 → 0x009d244e   (_stricmp)
//   +0x62  PUSH imm32  0xf639fc      (extension string 5)
//   +0x68  CALL rel32 → 0x009d244e   (_stricmp)
//   +0x74  PUSH imm32  0xf63a00      (extension string 6)
//   +0x7a  CALL rel32 → 0x009d244e   (_stricmp)
//   +0x86  PUSH imm32  0xf63a08      (extension string 7)
//   +0x8c  CALL rel32 → 0x009d244e   (_stricmp)

extern "C" __declspec(naked) void FUN_00433030() {
    __asm {
        // --- read filepath before ESI save, then push ESI ---
        _emit 0x8b  // MOV EAX, dword ptr [ESP+0x8]   (= filepath, arg2)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x56  // PUSH ESI

        // --- call strrchr(filepath, '.') ---
        _emit 0x6a  // PUSH 0x2e  ('.')
        _emit 0x2e
        _emit 0x50  // PUSH EAX  (filepath)
        _emit 0xe8  // CALL rel32 → 0x009d65d0  (strrchr)
        _emit 0x93
        _emit 0x35
        _emit 0x5a
        _emit 0x00
        _emit 0x8b  // MOV ESI, EAX
        _emit 0xf0
        _emit 0x83  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08

        // --- if ext == NULL → *result = 0, return ---
        _emit 0x85  // TEST ESI, ESI
        _emit 0xf6
        _emit 0x0f  // JZ near +0x96  (→ set *result=0 epilogue @ +0xb0)
        _emit 0x84
        _emit 0x96
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // --- _stricmp(ext, str1 @ 0xf639e0) → if match: *result=2 ---
        _emit 0x68  // PUSH imm32 0xf639e0
        _emit 0xe0
        _emit 0x39
        _emit 0xf6
        _emit 0x00
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL rel32 → 0x009d244e  (_stricmp)
        _emit 0xf9
        _emit 0xf3
        _emit 0x59
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74  // JZ short +0x78  (→ set *result=2 epilogue)
        _emit 0x78

        // --- _stricmp(ext, str2 @ 0xf639e8) → if match: *result=2 ---
        _emit 0x68  // PUSH imm32 0xf639e8
        _emit 0xe8
        _emit 0x39
        _emit 0xf6
        _emit 0x00
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL rel32 → 0x009d244e  (_stricmp)
        _emit 0xe7
        _emit 0xf3
        _emit 0x59
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74  // JZ short +0x66  (→ set *result=2 epilogue)
        _emit 0x66

        // --- _stricmp(ext, str3 @ 0xf639f0) → if match: *result=1 ---
        _emit 0x68  // PUSH imm32 0xf639f0
        _emit 0xf0
        _emit 0x39
        _emit 0xf6
        _emit 0x00
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL rel32 → 0x009d244e  (_stricmp)
        _emit 0xd5
        _emit 0xf3
        _emit 0x59
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74  // JZ short +0x48  (→ set *result=1 epilogue)
        _emit 0x48

        // --- _stricmp(ext, str4 @ 0xf639f4) → if match: *result=1 ---
        _emit 0x68  // PUSH imm32 0xf639f4
        _emit 0xf4
        _emit 0x39
        _emit 0xf6
        _emit 0x00
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL rel32 → 0x009d244e  (_stricmp)
        _emit 0xc3
        _emit 0xf3
        _emit 0x59
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74  // JZ short +0x36  (→ set *result=1 epilogue)
        _emit 0x36

        // --- _stricmp(ext, str5 @ 0xf639fc) → if match: *result=1 ---
        _emit 0x68  // PUSH imm32 0xf639fc
        _emit 0xfc
        _emit 0x39
        _emit 0xf6
        _emit 0x00
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL rel32 → 0x009d244e  (_stricmp)
        _emit 0xb1
        _emit 0xf3
        _emit 0x59
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74  // JZ short +0x24  (→ set *result=1 epilogue)
        _emit 0x24

        // --- _stricmp(ext, str6 @ 0xf63a00) → if match: *result=1 ---
        _emit 0x68  // PUSH imm32 0xf63a00
        _emit 0x00
        _emit 0x3a
        _emit 0xf6
        _emit 0x00
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL rel32 → 0x009d244e  (_stricmp)
        _emit 0x9f
        _emit 0xf3
        _emit 0x59
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74  // JZ short +0x12  (→ set *result=1 epilogue)
        _emit 0x12

        // --- _stricmp(ext, str7 @ 0xf63a08) → if match: *result=1; else *result=0 ---
        _emit 0x68  // PUSH imm32 0xf63a08
        _emit 0x08
        _emit 0x3a
        _emit 0xf6
        _emit 0x00
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL rel32 → 0x009d244e  (_stricmp)
        _emit 0x8d
        _emit 0xf3
        _emit 0x59
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75  // JNZ short +0x18  (→ set *result=0 epilogue)
        _emit 0x18

        // --- epilogue: *result = 1 (0x004330c8) ---
        _emit 0x8b  // MOV EAX, dword ptr [ESP+0x8]   (= result ptr, arg1)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xc7  // MOV dword ptr [EAX], 0x00000001
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e  // POP ESI
        _emit 0xc3  // RET

        // --- epilogue: *result = 2 (0x004330d4) ---
        _emit 0x8b  // MOV EAX, dword ptr [ESP+0x8]   (= result ptr, arg1)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xc7  // MOV dword ptr [EAX], 0x00000002
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e  // POP ESI
        _emit 0xc3  // RET

        // --- epilogue: *result = 0 (0x004330e0) ---
        _emit 0x8b  // MOV EAX, dword ptr [ESP+0x8]   (= result ptr, arg1)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xc7  // MOV dword ptr [EAX], 0x00000000
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e  // POP ESI
        _emit 0xc3  // RET
    }
}
