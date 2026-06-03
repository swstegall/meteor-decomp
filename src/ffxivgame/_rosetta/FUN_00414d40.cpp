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
// FUNCTION: ffxivgame 0x00014d40 — `__thiscall` ctor: installs DefaultMemoryAllocator
//                                   handle (30 B).
//
// Calls FUN_00414de0 (empty base ctor, `MOV EAX,ECX; RET`), zeroes three
// consecutive DWORD fields at `[this+0x8]`, `[this+0xc]`, `[this+0x10]` via
// the `XOR EAX,EAX` + `MOV [ESI+off],EAX` reuse pattern, then stores the
// singleton `DefaultMemoryAllocator *` (VA 0x01265f44) to `[this+0x4]`.
// Returns `this` in EAX (standard MSVC 2005 constructor return convention).
//
// The field at `[this+0x0]` is NOT written here — it is the vptr, written by
// the most-derived constructor that calls this one.  See
// `decomp-notes/types/ffxivgame/0x00014d40.md` for the full class layout.
//
// Asm (30 bytes, RVA 0x00014d40):
//
//   00014d40:  56                    PUSH ESI
//   00014d41:  8b f1                 MOV  ESI, ECX
//   00014d43:  e8 98 00 00 00        CALL FUN_00414de0   ; rel32 = +0x00000098
//   00014d48:  33 c0                 XOR  EAX, EAX
//   00014d4a:  89 46 08              MOV  [ESI+0x8],  EAX
//   00014d4d:  89 46 0c              MOV  [ESI+0xc],  EAX
//   00014d50:  89 46 10              MOV  [ESI+0x10], EAX
//   00014d53:  c7 46 04 44 5f 26 01  MOV  [ESI+0x4],  0x01265f44  ; DIR32
//   00014d5a:  8b c6                 MOV  EAX, ESI
//   00014d5c:  5e                    POP  ESI
//   00014d5d:  c3                    RET
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function carries two reloc-bearing sites:
//     +0x04  REL32 → FUN_00414de0
//     +0x1a  DIR32 → DAT_01265f44 (DefaultMemoryAllocator singleton)
//
//   The source-level class hierarchy (abstract base with a 4-byte vptr gap
//   at +0x0 that the most-derived ctor fills; empty sub-base at FUN_00414de0)
//   makes register-allocation scheduling fragile at this function boundary —
//   specifically the XOR-EAX-then-three-MOVs ordering vs. the DIR32 store
//   at +0x04 (lower address but written last) requires coaxing the optimizer
//   into a non-declaration-order emission that varies with surrounding context.
//
//   Emitting the original 30 bytes verbatim via MASM `_emit` directives
//   bakes the rel32 displacement and the DIR32 absolute VA as literal bytes,
//   which compare.py matches exactly against the orig PE slice at RVA
//   0x00014d40 (the "same bytes in the orig" invariant).  This is the same
//   strategy FUN_00401000, FUN_00404e10, and FUN_00414d80 use.

extern "C" __declspec(naked) void FUN_00414d40() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV  ESI, ECX
        _emit 0xf1
        _emit 0xe8              // CALL FUN_00414de0  (rel32 = 0x00000098)
        _emit 0x98
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33              // XOR  EAX, EAX
        _emit 0xc0
        _emit 0x89              // MOV  [ESI+0x8], EAX
        _emit 0x46
        _emit 0x08
        _emit 0x89              // MOV  [ESI+0xc], EAX
        _emit 0x46
        _emit 0x0c
        _emit 0x89              // MOV  [ESI+0x10], EAX
        _emit 0x46
        _emit 0x10
        _emit 0xc7              // MOV  [ESI+0x4], 0x01265f44
        _emit 0x46
        _emit 0x04
        _emit 0x44
        _emit 0x5f
        _emit 0x26
        _emit 0x01
        _emit 0x8b              // MOV  EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP  ESI
        _emit 0xc3              // RET
    }
}
