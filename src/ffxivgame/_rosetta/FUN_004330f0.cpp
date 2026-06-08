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
// FUNCTION: ffxivgame 0x004330f0 — match a dot-prefixed file extension against
//                                   a small global table and return its 1-based
//                                   index, or 0 if not found (83 B / 0x53,
//                                   __cdecl, no stack frame, saves ESI+EDI).
//
// Behaviour read from the disassembly at orig RVA 0x000330f0:
//
//   void __cdecl FUN_004330f0(int *result, const char *str) {
//       const char *dot = strchr(str, '.');   // FUN_009d65d0
//       if (dot == NULL) {
//           *result = 0;
//           return;
//       }
//       for (int i = 1; i < 3; i++) {
//           if (CompareStr(dot, g_ext_table[i]) == 0) {  // FUN_009d244e
//               *result = i;
//               return;
//           }
//       }
//       *result = 0;
//   }
//
//   Stack frame: none (no SUB ESP).  ESI and EDI are saved/restored
//   manually.  All branches are short (±128-byte displacement).
//
//   param1 (int *result)   = [ESP+4] at entry → [ESP+0xc] after two pushes
//   param2 (const char *)  = [ESP+8] at entry (loaded before push prologue)
//
//   Externals:
//     FUN_009d65d0   — strchr (c-runtime)
//     FUN_009d244e   — string comparison function (strcmp / _stricmp)
//     g_ext_table    — global array of string pointers at 0x0126685c;
//                      indices 1 and 2 are checked (index 0 unused by
//                      this function); accessed as [ESI*4 + g_ext_table].
//
//   Notable detail: MSVC 2005 /O2 emits a 4-byte LEA ESP,[ESP+0] NOP
//   (8D 64 24 00) immediately before the loop head at +0x20 to align it
//   to a 16-byte boundary (RVA 0x00033110 ≡ 0 mod 16).  This NOP is
//   reproduced verbatim via _emit; all three reloc-bearing operands
//   (two CALL REL32s and one SIB DIR32) are handled as MASM symbolic
//   references so compare.py can mask them in the diff.

extern "C" {

// Call targets — REL32 COFF relocs in the .obj.
void FUN_009d65d0();  // strchr
void FUN_009d244e();  // extension comparison function

// Global string-pointer table at 0x0126685c (DIR32 reloc in the SIB load).
// Element 0 is never accessed; elements 1 and 2 are the candidate extensions.
extern int g_ext_table;

__declspec(naked) void FUN_004330f0() {
    __asm {
        // --- prologue -------------------------------------------------------
        mov     eax, dword ptr [esp+8]        // 8b 44 24 08  load param2 (str)
        push    esi                            // 56
        push    edi                            // 57

        // --- strchr(str, '.') ----------------------------------------------
        push    0x2e                           // 6a 2e        '.'
        push    eax                            // 50
        call    FUN_009d65d0                   // e8 ...       strchr
        mov     edi, eax                       // 8b f8
        add     esp, 8                         // 83 c4 08
        test    edi, edi                       // 85 ff
        jz      lbl_no_match                  // 74 26

        // --- loop setup -----------------------------------------------------
        mov     esi, 1                         // be 01 00 00 00

        // 16-byte loop-head alignment NOP: LEA ESP,[ESP+0]  (8D 64 24 00)
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00

        // --- loop: for (i = 1; i < 3; i++) ---------------------------------
    lbl_loop:
        mov     eax, dword ptr [esi*4 + g_ext_table]  // 8b 04 b5 [DIR32]
        push    eax                            // 50
        push    edi                            // 57
        call    FUN_009d244e                   // e8 ...       comparison function
        add     esp, 8                         // 83 c4 08
        test    eax, eax                       // 85 c0
        jz      lbl_found                     // 74 15
        add     esi, 1                         // 83 c6 01
        cmp     esi, 3                         // 83 fe 03
        jl      lbl_loop                      // 7c e3

        // --- not found path -------------------------------------------------
    lbl_no_match:
        mov     eax, dword ptr [esp+0xc]       // 8b 44 24 0c  param1 (result)
        pop     edi                            // 5f
        mov     dword ptr [eax], 0             // c7 00 00 00 00 00
        pop     esi                            // 5e
        ret                                    // c3

        // --- found path -----------------------------------------------------
    lbl_found:
        mov     eax, dword ptr [esp+0xc]       // 8b 44 24 0c  param1 (result)
        pop     edi                            // 5f
        mov     dword ptr [eax], esi           // 89 30
        pop     esi                            // 5e
        ret                                    // c3
    }
}

}  // extern "C"
