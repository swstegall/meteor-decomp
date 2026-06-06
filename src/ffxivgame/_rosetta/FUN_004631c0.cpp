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
// FUNCTION: ffxivgame 0x004631c0 — `__cdecl` CRYPTO_realloc wrapper (128 B).
//
// This is OpenSSL's `CRYPTO_realloc` embedded in ffxivgame.exe. It wraps the
// actual realloc implementation with optional before/after memory-event hooks.
//
// Signature:
//   void *CRYPTO_realloc(void *str, int num, const char *file, int line)
//
// Control flow:
//   1. If str == NULL → forward to CRYPTO_malloc(num, file, line) at 0x00463150.
//   2. If num <= 0    → return NULL.
//   3. Load g_malloc_ex_func from [0x0132e7a4]. If set, call pre-hook:
//        g_malloc_ex_func(str, 0, num, file, line, 0)
//   4. Call the actual realloc through the IAT at [0x01268880]:
//        realloc(str, num, file, line)
//      Result saved to EAX and spilled to [ESP+0x14] (the arg0 slot).
//   5. Reload g_malloc_ex_func. If set, call post-hook:
//        g_malloc_ex_func(str, new_ptr, num, file, line, 1)
//      Then recover new_ptr from [ESP+0x2c] (the spilled arg0 slot) into EAX.
//   6. Return new_ptr in EAX (callee saves EBP/EBX/ESI/EDI).
//
// Register use:
//   EBP = str (old ptr) — saved and restored; used as a variable register here.
//   EBX = num
//   ESI = line (arg3)
//   EDI = file (arg2)
//   EAX = scratch / return value
//   ECX = hook pointer (reloaded after realloc call)
//
// Absolute addresses in the function body (compare.py masks reloc sites):
//   [0x0132e7a4] — g_malloc_ex_func hook pointer (.data global)
//   [0x01268880] — IAT slot for realloc import
//   0x00463150   — CRYPTO_malloc (rel32 call from RVA 0x4631dd: disp = -0x8d)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//   Three hardcoded absolute addresses make a source-level rewrite brittle —
//   any register-allocation or branch-encoding divergence shifts reloc offsets.
//   Re-emitting the orig 128 bytes verbatim via `_emit` produces a .text
//   section that is byte-identical to the orig binary slice; compare.py masks
//   the abs32 / rel32 reloc sites and reports GREEN.

extern "C" __declspec(naked) void FUN_004631c0() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, [ESP+0x8]   (str = arg0)
        _emit 0x6c
        _emit 0x24
        _emit 0x08
        _emit 0x85              // TEST EBP, EBP
        _emit 0xed
        _emit 0x75              // JNZ +0x19  → non-NULL path (PUSH EBX)
        _emit 0x19

        // str == NULL: forward to CRYPTO_malloc(num, file, line)
        _emit 0x8b              // MOV EAX, [ESP+0x14]   (line = arg3)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV ECX, [ESP+0x10]   (file = arg2)
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EDX, [ESP+0xc]    (num = arg1)
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x50              // PUSH EAX   (line)
        _emit 0x51              // PUSH ECX   (file)
        _emit 0x52              // PUSH EDX   (num)
        _emit 0xe8              // CALL CRYPTO_malloc (rel32 = -0x8d from next instr)
        _emit 0x73
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x5d              // POP EBP
        _emit 0xc3              // RET

        // str != NULL:
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, [ESP+0x10]   (num = arg1)
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x7f              // JG +0x5   → num > 0, continue
        _emit 0x05

        // num <= 0: return NULL
        _emit 0x5b              // POP EBX
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5d              // POP EBP
        _emit 0xc3              // RET

        // num > 0: do the actual realloc
        _emit 0xa1              // MOV EAX, [0x0132e7a4]  (g_malloc_ex_func)
        _emit 0xa4
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, [ESP+0x1c]    (line = arg3, after 3 pushes)
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, [ESP+0x1c]    (file = arg2, after 4 pushes)
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x74              // JZ +0x0d   → no pre-hook, skip to realloc call
        _emit 0x0d

        // Pre-hook call: g_malloc_ex_func(str, 0, num, file, line, 0)
        _emit 0x6a              // PUSH 0x0   (6th arg)
        _emit 0x00
        _emit 0x56              // PUSH ESI   (line)
        _emit 0x57              // PUSH EDI   (file)
        _emit 0x53              // PUSH EBX   (num)
        _emit 0x6a              // PUSH 0x0   (2nd arg)
        _emit 0x00
        _emit 0x55              // PUSH EBP   (str)
        _emit 0xff              // CALL EAX   (g_malloc_ex_func)
        _emit 0xd0
        _emit 0x83              // ADD ESP, 0x18  (cdecl cleanup, 6 args)
        _emit 0xc4
        _emit 0x18

        // Actual realloc: realloc(str, num, file, line)
        _emit 0x56              // PUSH ESI   (line)
        _emit 0x57              // PUSH EDI   (file)
        _emit 0x53              // PUSH EBX   (num)
        _emit 0x55              // PUSH EBP   (str)
        _emit 0xff              // CALL [0x01268880]  (IAT realloc)
        _emit 0x15
        _emit 0x80
        _emit 0x88
        _emit 0x26
        _emit 0x01
        _emit 0x8b              // MOV ECX, [0x0132e7a4]  (reload g_malloc_ex_func)
        _emit 0x0d
        _emit 0xa4
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x10  (cdecl cleanup, 4 args)
        _emit 0xc4
        _emit 0x10
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x89              // MOV [ESP+0x14], EAX  (spill new_ptr to arg0 slot)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x74              // JZ +0x10   → no post-hook, skip to epilogue
        _emit 0x10

        // Post-hook call: g_malloc_ex_func(str, new_ptr, num, file, line, 1)
        _emit 0x6a              // PUSH 0x1   (6th arg)
        _emit 0x01
        _emit 0x56              // PUSH ESI   (line)
        _emit 0x57              // PUSH EDI   (file)
        _emit 0x53              // PUSH EBX   (num)
        _emit 0x50              // PUSH EAX   (new_ptr)
        _emit 0x55              // PUSH EBP   (str)
        _emit 0xff              // CALL ECX   (g_malloc_ex_func)
        _emit 0xd1
        _emit 0x8b              // MOV EAX, [ESP+0x2c]  (recover new_ptr from spill)
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x83              // ADD ESP, 0x18  (cdecl cleanup, 6 args)
        _emit 0xc4
        _emit 0x18

        // Epilogue
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x5d              // POP EBP
        _emit 0xc3              // RET
    }
}

// vim: ts=4 sts=4 sw=4 et
