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
// FUNCTION: ffxivgame 0x00409260 — singleton get-or-set accessor with
// SEH-protected lazy initialization (__cdecl, 104 B).
//
// Semantics (recovered from asm):
//
//   void *FUN_00409260(void *replacement) {
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
//   0x01327ae4  g_flag       — initialized-once flag (bit 0 is "init done")
//   0x01327ae0  g_instance   — singleton pointer (set lazily, optionally replaced)
//
// The init CALL targets RVA 0x0040e500 (rel32 from this site). Its second
// stack slot ([ESP+8] inside this function's frame) is preset to 0 before
// the call; that's the trylevel slot AND the first argument of init —
// either MSVC reused the SEH frame's trylevel slot as the arg, or the
// init function is __cdecl-with-one-arg(0).
//
// Original 104 bytes (per asm/ffxivgame/00009260_FUN_00409260.s):
//
//   00009260: 64 a1 00 00 00 00 6a ff 68 7e 49 e5 00 50 b8 01
//   00009270: 00 00 00 64 89 25 00 00 00 00 84 05 e4 7a 32 01
//   00009280: 75 18 09 05 e4 7a 32 01 c7 44 24 08 00 00 00 00
//   00009290: e8 6b 52 00 00 a3 e0 7a 32 01 8b 44 24 10 85 c0
//   000092a0: 74 13 a3 e0 7a 32 01 8b 0c 24 64 89 0d 00 00 00
//   000092b0: 00 83 c4 0c c3 8b 0c 24 a1 e0 7a 32 01 64 89 0d
//   000092c0: 00 00 00 00 83 c4 0c c3
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
//   +0x09  scope_table              (.rdata 0x00e5497e)
//   +0x1c  g_flag                   (.data  0x01327ae4)
//   +0x24  g_flag                   (.data  0x01327ae4)
//   +0x31  init function CALL       (rel32 to .text 0x0040e500)
//   +0x36  g_instance               (.data  0x01327ae0)
//   +0x43  g_instance               (.data  0x01327ae0)
//   +0x59  g_instance               (.data  0x01327ae0)

extern "C" {
    // .rdata — MSVC-emitted EH3 scope table (FuncInfo) for this fn.
    extern int g_scope_table_00409260;

    // .data — initialize-once flag (bit 0 is "init done"). Declared
    // as `int` so the dword OR works; the byte TEST uses an explicit
    // `byte ptr` override.
    extern int g_singleton_flag_00409260;

    // .data — singleton pointer (lazily initialized; optionally
    // replaced by the function's argument).
    extern void *g_singleton_instance_00409260;

    // .text — RVA 0x0040e500. First-time init helper. Returns the new
    // singleton instance pointer (stored to g_singleton_instance).
    void *FUN_0040e500();
}

extern "C" __declspec(naked) void FUN_00409260() {
    __asm {
        // --- EH3-style SEH prolog (no /GS cookie) ----------------------
        mov     eax, fs:[0]                                 // 64 a1 00 00 00 00 (6 B)
        push    -1                                          // 6a ff           (2 B)
        push    offset g_scope_table_00409260               // 68 ?? ?? ?? ??  (5 B, reloc +1)
        push    eax                                         // 50              (1 B)
        mov     eax, 1                                      // b8 01 00 00 00  (5 B)
        mov     fs:[0], esp                                 // 64 89 25 00 00 00 00 (7 B)

        // --- if ((g_flag & 1) == 0) lazy-init g_instance ---------------
        test    byte ptr g_singleton_flag_00409260, al      // 84 05 ?? ?? ?? ?? (6 B, reloc +2)
        jnz     already_init                                // 75 18           (2 B)
        or      g_singleton_flag_00409260, eax              // 09 05 ?? ?? ?? ?? (6 B, reloc +2)
        mov     dword ptr [esp + 8], 0                      // c7 44 24 08 00 00 00 00 (8 B)
        call    FUN_0040e500                                // e8 ?? ?? ?? ??  (5 B, reloc +1)
        mov     g_singleton_instance_00409260, eax          // a3 ?? ?? ?? ??  (5 B, reloc +1)
    already_init:

        // --- if (arg != 0) set; else return current --------------------
        mov     eax, [esp + 0x10]                           // 8b 44 24 10     (4 B)
        test    eax, eax                                    // 85 c0           (2 B)
        jz      get_path                                    // 74 13           (2 B)
        mov     g_singleton_instance_00409260, eax          // a3 ?? ?? ?? ??  (5 B, reloc +1)
        mov     ecx, [esp]                                  // 8b 0c 24        (3 B)
        mov     fs:[0], ecx                                 // 64 89 0d 00 00 00 00 (7 B)
        add     esp, 0xc                                    // 83 c4 0c        (3 B)
        ret                                                 // c3              (1 B)

    get_path:
        mov     ecx, [esp]                                  // 8b 0c 24        (3 B)
        mov     eax, g_singleton_instance_00409260          // a1 ?? ?? ?? ??  (5 B, reloc +1)
        mov     fs:[0], ecx                                 // 64 89 0d 00 00 00 00 (7 B)
        add     esp, 0xc                                    // 83 c4 0c        (3 B)
        ret                                                 // c3              (1 B)
    }
}
