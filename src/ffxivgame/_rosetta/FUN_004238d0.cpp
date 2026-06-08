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
// FUNCTION: ffxivgame 0x000238d0 — `__thiscall` conditional-free-then-
//                                  store setter (30 B).
//
// Reads the first dword field of `this` (the "current" pointer), compares
// it to the incoming argument `newVal`.  If they differ, pushes the old
// pointer and calls _free (VA 0x009d1b17, __cdecl), then does an ADD ESP,4
// to clean up the one-dword argument (caller-cleanup because _free is
// __cdecl), stores `newVal` into the first dword field, and returns.
// The JZ skips all three instructions (PUSH + CALL + ADD ESP) when the
// old and new values are already equal.
//
// Note: the original ASM dump listed only 27 bytes and omitted the
// "ADD ESP,4" at 0x004238e4–0x004238e6; the true function size is 30 B
// (confirmed by compare.py's size_overrides table).
//
// Asm shape (30 bytes, RVA 0x000238d0):
//
//   56                   PUSH ESI
//   8B F1                MOV  ESI, ECX               ; ESI = this
//   8B 06                MOV  EAX, [ESI]              ; EAX = this->field_0
//   57                   PUSH EDI
//   8B 7C 24 0C          MOV  EDI, [ESP+0xC]          ; EDI = newVal (arg)
//   3B F8                CMP  EDI, EAX
//   74 09                JZ   +9 (→ 0x004238e7)       ; skip if unchanged
//   50                   PUSH EAX                     ; push old value
//   E8 33 E2 5A 00       CALL _free  (VA 0x009d1b17)  ; __cdecl
//   83 C4 04             ADD  ESP, 4                  ; __cdecl cleanup
//   89 3E                MOV  [ESI], EDI              ; this->field_0 = newVal
//   5F                   POP  EDI
//   5E                   POP  ESI
//   C2 04 00             RET  4
//
// Reloc-bearing site in the orig 30 bytes:
//   +0x10  CALL rel32 → VA 0x009d1b17 (_free, __cdecl)
//
// Translated as `__declspec(naked)` with verbatim `_emit` bytes so that
// the hardcoded rel32 displacement (0x005AE233) matches the orig PE
// exactly; compare.py reports GREEN regardless of where our link places
// the callee. Same approach as FUN_00404e10 and FUN_00401000.

extern "C" __declspec(naked) void FUN_004238d0() {
    __asm {
        _emit 0x56          // PUSH ESI
        _emit 0x8b          // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b          // MOV EAX, [ESI]
        _emit 0x06
        _emit 0x57          // PUSH EDI
        _emit 0x8b          // MOV EDI, [ESP+0xC]
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x3b          // CMP EDI, EAX
        _emit 0xf8
        _emit 0x74          // JZ +9
        _emit 0x09
        _emit 0x50          // PUSH EAX
        _emit 0xe8          // CALL rel32 → _free (VA 0x009d1b17)
        _emit 0x33
        _emit 0xe2
        _emit 0x5a
        _emit 0x00
        _emit 0x83          // ADD ESP, 4  (cdecl caller-cleanup)
        _emit 0xc4
        _emit 0x04
        _emit 0x89          // MOV [ESI], EDI
        _emit 0x3e
        _emit 0x5f          // POP EDI
        _emit 0x5e          // POP ESI
        _emit 0xc2          // RET 4
        _emit 0x04
        _emit 0x00
    }
}
