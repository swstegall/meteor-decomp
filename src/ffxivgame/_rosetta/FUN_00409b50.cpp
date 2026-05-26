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
// FUNCTION: ffxivgame 0x00409b50 — singleton get-or-set accessor with
// SEH-protected lazy initialization (__cdecl, 104 B).
//
// Byte-for-byte triplet of FUN_00409260 / FUN_00409840 (same EH3-style
// SEH frame, same init helper CALL target at RVA 0x0040e500, same
// get-or-set shape) — only the scope_table address and the two .data
// globals differ.
//
// Semantics (recovered from asm):
//
//   void *FUN_00409b50(void *replacement) {
//       __try {                                 // SEH frame guards the init CALL
//           if ((g_flag & 1) == 0) {            // 84 05 [flag], al  (al == 1 from MOV EAX,1)
//               g_flag |= 1;                    // OR [flag], eax  (eax == 1)
//               g_instance = FUN_0040e500();    // first-time init: 0 (state) arg via [esp+8]
//           }
//           if (replacement != NULL) {
//               g_instance = replacement;
//               return replacement;             // EAX still holds the arg
//           }
//           return g_instance;
//       }
//       // No __except body — the frame is registered for unwind only.
//   }
//
// Two globals (in .data):
//   0x01327b70  g_flag       — initialized-once flag (bit 0 is "init done")
//   0x01327b6c  g_instance   — singleton pointer (set lazily, optionally replaced)
//
// The init CALL targets RVA 0x0040e500 (rel32 from this site) — the
// same helper called by FUN_00409260 and FUN_00409840.
//
// Original 104 bytes (per asm/ffxivgame/00009b50_FUN_00409b50.s):
//
//   00009b50: 64 a1 00 00 00 00 6a ff 68 1e 4b e5 00 50 b8 01
//   00009b60: 00 00 00 64 89 25 00 00 00 00 84 05 70 7b 32 01
//   00009b70: 75 18 09 05 70 7b 32 01 c7 44 24 08 00 00 00 00
//   00009b80: e8 7b 49 00 00 a3 6c 7b 32 01 8b 44 24 10 85 c0
//   00009b90: 74 13 a3 6c 7b 32 01 8b 0c 24 64 89 0d 00 00 00
//   00009ba0: 00 83 c4 0c c3 8b 0c 24 a1 6c 7b 32 01 64 89 0d
//   00009bb0: 00 00 00 00 83 c4 0c c3
//
// Why naked asm: this is the same EH3-style SEH-frame shape modeled
// after FUN_00401750.cpp — PUSH -1 / PUSH scope_table / PUSH FS:[0] /
// MOV FS:[0], ESP — with no /GS cookie XOR ESP (no buffer locals).
// Source-level C++ wouldn't reproduce the byte-for-byte ordering of
// MOV EAX,FS:[0] BEFORE the PUSH -1 (orig) vs after the two pushes
// (compiler-emitted), nor the `a1`/`a3` short EAX-to-moffs32 encoding
// of the g_instance load/stores, nor the use of `TEST byte ptr` against
// AL while AL == 1 (a 6-byte alternative to the 7-byte
// `TEST byte ptr [...], 1` immediate form — MSVC's optimizer reused
// EAX's low byte after `MOV EAX, 1` to save one byte).
//
// Reloc-bearing sites (compare.py wildcard-masks these 4-byte windows):
//   +0x09  scope_table              (.rdata 0x00e54b1e)
//   +0x1c  g_flag                   (.data  0x01327b70)
//   +0x24  g_flag                   (.data  0x01327b70)
//   +0x31  init function CALL       (rel32 to .text 0x0040e500)
//   +0x36  g_instance               (.data  0x01327b6c)
//   +0x43  g_instance               (.data  0x01327b6c)
//   +0x59  g_instance               (.data  0x01327b6c)

extern "C" {
    // .rdata — MSVC-emitted EH3 scope table (FuncInfo) for this fn.
    extern int g_scope_table_00409b50;

    // .data — initialize-once flag (bit 0 is "init done"). Declared
    // as `int` so the dword OR works; the byte TEST uses an explicit
    // `byte ptr` override.
    extern int g_singleton_flag_00409b50;

    // .data — singleton pointer (lazily initialized; optionally
    // replaced by the function's argument).
    extern void *g_singleton_instance_00409b50;

    // .text — RVA 0x0040e500. First-time init helper. Returns the new
    // singleton instance pointer (stored to g_singleton_instance).
    void *FUN_0040e500();
}

extern "C" __declspec(naked) void FUN_00409b50() {
    __asm {
        // --- EH3-style SEH prolog (no /GS cookie) ----------------------
        mov     eax, fs:[0]                                 // 64 a1 00 00 00 00 (6 B)
        push    -1                                          // 6a ff           (2 B)
        push    offset g_scope_table_00409b50               // 68 ?? ?? ?? ??  (5 B, reloc +1)
        push    eax                                         // 50              (1 B)
        mov     eax, 1                                      // b8 01 00 00 00  (5 B)
        mov     fs:[0], esp                                 // 64 89 25 00 00 00 00 (7 B)

        // --- if ((g_flag & 1) == 0) lazy-init g_instance ---------------
        test    byte ptr g_singleton_flag_00409b50, al      // 84 05 ?? ?? ?? ?? (6 B, reloc +2)
        jnz     already_init                                // 75 18           (2 B)
        or      g_singleton_flag_00409b50, eax              // 09 05 ?? ?? ?? ?? (6 B, reloc +2)
        mov     dword ptr [esp + 8], 0                      // c7 44 24 08 00 00 00 00 (8 B)
        call    FUN_0040e500                                // e8 ?? ?? ?? ??  (5 B, reloc +1)
        mov     g_singleton_instance_00409b50, eax          // a3 ?? ?? ?? ??  (5 B, reloc +1)
    already_init:

        // --- if (arg != 0) set; else return current --------------------
        mov     eax, [esp + 0x10]                           // 8b 44 24 10     (4 B)
        test    eax, eax                                    // 85 c0           (2 B)
        jz      get_path                                    // 74 13           (2 B)
        mov     g_singleton_instance_00409b50, eax          // a3 ?? ?? ?? ??  (5 B, reloc +1)
        mov     ecx, [esp]                                  // 8b 0c 24        (3 B)
        mov     fs:[0], ecx                                 // 64 89 0d 00 00 00 00 (7 B)
        add     esp, 0xc                                    // 83 c4 0c        (3 B)
        ret                                                 // c3              (1 B)

    get_path:
        mov     ecx, [esp]                                  // 8b 0c 24        (3 B)
        mov     eax, g_singleton_instance_00409b50          // a1 ?? ?? ?? ??  (5 B, reloc +1)
        mov     fs:[0], ecx                                 // 64 89 0d 00 00 00 00 (7 B)
        add     esp, 0xc                                    // 83 c4 0c        (3 B)
        ret                                                 // c3              (1 B)
    }
}
