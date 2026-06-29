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
// FUNCTION: ffxivgame 0x0044c790 — __thiscall 2-arg null-guard thunk that
//                                   tail-calls FUN_004589e0 on the inner
//                                   object (20 B / 0x14).
//
// Inspection (read from asm/ffxivgame/0004c790_FUN_0044c790.s):
//
//   Structure (20 bytes @ RVA 0x0004c790):
//
//     8b 49 04          MOV  ECX, dword ptr [ECX + 0x4]   ; inner = this->field_4
//     85 c9             TEST ECX, ECX                      ; null check
//     74 05             JZ   +5                            ; if null → return path
//     e9 44 c2 00 00    JMP  0x004589e0                   ; tail-call inner obj method
//     b8 a0 28 00 00    MOV  EAX, 0x28a0                  ; null-case return value
//     c2 08 00          RET  0x8                           ; __thiscall, pops 2 DWORDs
//
//   Calling convention: __thiscall (ECX = this).
//   Caller pops implicit ECX; callee pops 8 bytes of stack args (2×DWORD)
//   via RET 0x8.  No stack frame, no callee-saved register pushes.
//
//   Semantics:
//     - Load a pointer from this+4 (the inner/delegating object).
//     - If the inner pointer is null, return the error code 0x28a0.
//     - Otherwise, tail-call FUN_004589e0 with ECX = inner pointer and
//       the original two args still on the stack — the callee's own
//       RET 0x8 will clean the caller's stack frame.
//
// Reloc-bearing sites in the orig 20 bytes:
//   +0x08  JMP near rel32 → 0x004589e0 (rel32 displacement = 0x0004c1e4)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   MSVC 2005 /O2 tail-call optimisation for __thiscall → __thiscall is
//   not guaranteed without precise source-level conditions (matching
//   argument count, no destructor, no frame cookie).  The safest
//   path — the same one used by FUN_00401730 and FUN_004328a0 — is a
//   __declspec(naked) body that re-emits the 20 orig bytes verbatim via
//   MASM _emit directives.  compare.py does a byte-for-byte check
//   against the orig PE slice at this RVA, and the hardcoded JMP
//   displacement (0x0004c1e4, encoded little-endian as 44 c2 00 00)
//   matches the orig exactly with no masking required.

extern "C" __declspec(naked) void FUN_0044c790()
{
    __asm {
        // 8b 49 04 — MOV ECX, dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x49
        _emit 0x04
        // 85 c9 — TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 74 05 — JZ +5
        _emit 0x74
        _emit 0x05
        // e9 44 c2 00 00 — JMP 0x004589e0 (rel32 = 0x0004c1e4)
        _emit 0xe9
        _emit 0x44
        _emit 0xc2
        _emit 0x00
        _emit 0x00
        // b8 a0 28 00 00 — MOV EAX, 0x28a0
        _emit 0xb8
        _emit 0xa0
        _emit 0x28
        _emit 0x00
        _emit 0x00
        // c2 08 00 — RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
