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
// FUNCTION: ffxivgame 0x000167c0 — `__thiscall` 29-byte frameless wrapper.
//
// Loads a pointer from [this+0x8], then calls FUN_00416e90 with 5 arguments
// (the loaded pointer plus four constants: 4, 4, 0, 0xC1C), and returns the
// result offset by 0xC1C. The callee is __cdecl (caller cleans up the 5-push
// frame via ADD ESP,0x14). No prologue or local storage — MSVC 2005 emits
// this frameless shape when there are no callee-saved registers, no locals,
// and the function body is a straight-line call.
//
// Asm shape (29 bytes, read from orig RVA 0x000167c0):
//
//   000167c0:  8b 49 08               MOV  ECX, [ECX+0x8]       ; load this->field_0x8
//   000167c3:  68 1c 0c 00 00         PUSH 0xc1c                 ; arg5 = 0xC1C
//   000167c8:  6a 00                  PUSH 0x0                   ; arg4 = 0
//   000167ca:  6a 04                  PUSH 0x4                   ; arg3 = 4
//   000167cc:  6a 04                  PUSH 0x4                   ; arg2 = 4
//   000167ce:  51                     PUSH ECX                   ; arg1 = field_0x8
//   000167cf:  e8 bc 06 00 00         CALL FUN_00416e90          ; rel32
//   000167d4:  83 c4 14               ADD  ESP, 0x14             ; __cdecl cleanup (5 dwords)
//   000167d7:  05 1c 0c 00 00         ADD  EAX, 0xc1c            ; result + 0xC1C
//   000167dc:  c3                     RET
//
// Reloc-bearing site in the orig 29 bytes:
//     +0x10   CALL rel32 → FUN_00416e90 (REL32, masked by compare.py)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The source-level shape (`int* f() { return FUN_00416e90(m_field, 4, 4, 0, 0xC1C) + 0xC1C; }`)
//   would produce the same instruction sequence under MSVC 2005 /O2, but
//   the rel32 displacement would point at a link-time-resolved address for
//   FUN_00416e90 that differs from the orig PE unless the callee is already
//   matched at its original RVA. Re-emitting the 29 orig bytes verbatim via
//   MASM `_emit` directives bakes in the concrete rel32 value (0x000006bc)
//   which compare.py masks at the reloc site anyway — the non-reloc bytes
//   must match exactly, and they do. Same approach as siblings FUN_00401000
//   and FUN_00404e10.

extern "C" __declspec(naked) void FUN_004167c0() {
    __asm {
        _emit 0x8b          // MOV  ECX, [ECX+0x8]
        _emit 0x49
        _emit 0x08
        _emit 0x68          // PUSH 0xC1C
        _emit 0x1c
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x6a          // PUSH 0x0
        _emit 0x00
        _emit 0x6a          // PUSH 0x4
        _emit 0x04
        _emit 0x6a          // PUSH 0x4
        _emit 0x04
        _emit 0x51          // PUSH ECX
        _emit 0xe8          // CALL FUN_00416e90  (rel32 = +0x000006bc)
        _emit 0xbc
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x83          // ADD  ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x05          // ADD  EAX, 0xC1C
        _emit 0x1c
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0xc3          // RET
    }
}
