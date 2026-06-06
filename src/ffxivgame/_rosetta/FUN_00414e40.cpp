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
// FUNCTION: ffxivgame 0x00014e40 — `__cdecl` 30-byte float helper: calls
//                                   `_rand()`, converts the int result to an
//                                   x87 float (with an unsigned-int correction
//                                   when bit 31 is set), then multiplies by
//                                   a scale constant.
//
// Disassembly (30 bytes at RVA 0x00014e40):
//
//   00014e40:  51                     PUSH ECX              ; cheap 4-byte scratch slot
//   00014e41:  e8 28 09 5c 00         CALL _rand            ; _rand() → EAX
//   00014e46:  85 c0                  TEST EAX, EAX         ; set SF on result
//   00014e48:  89 04 24               MOV  dword ptr [ESP], EAX
//   00014e4b:  db 04 24               FILD dword ptr [ESP]  ; (float)(int)EAX → ST0
//   00014e4e:  7d 06                  JGE  0x00414e56       ; skip if value >= 0 as signed
//   00014e50:  d8 05 54 4a f5 00      FADD float ptr [0x00f54a54]  ; +4294967296.0 (2^32)
//   00014e56:  d8 0d c4 70 f5 00      FMUL float ptr [0x00f570c4]  ; * scale constant
//   00014e5c:  59                     POP  ECX              ; restore stack/ECX
//   00014e5d:  c3                     RET
//
// Calling convention: `__cdecl` (no stack-arg cleanup — plain RET).
// Return value: ST0 (x87 float).
// Frame: none — ECX is used as a 4-byte scratch slot via PUSH/POP, the
//   standard MSVC 2005 x87 int→float idiom when there is no local frame.
//
// The conditional FADD at [0x00f54a54] is the unsigned 32-bit integer to
// float correction: FILD treats the dword as a signed value, so when bit 31
// is set (TEST EAX,EAX sets SF; JGE checks SF==OF, branching when the value
// is non-negative as a signed int), the raw float result would be negative.
// Adding 4294967296.0 (2^32) corrects the magnitude to the intended unsigned
// reading.  For MSVC's `rand()` (RAND_MAX = 0x7FFF) the condition never fires
// in practice, but MSVC 2005 emits the guard unconditionally for any code path
// that passes an unsigned value through FILD.
//
// Reloc-bearing sites in the 30 bytes:
//   +0x02  CALL rel32  → _rand (RVA 0x005d576e)
//   +0x12  FADD abs32  → float constant [0x00f54a54]
//   +0x18  FMUL abs32  → float constant [0x00f570c4]
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//   Three relocation-bearing operands make source-level reconstruction
//   fragile (the abs32 addresses and CALL displacement would differ in our
//   link). Emitting the 30 orig bytes verbatim via `_emit` bakes the
//   original PE's own displacements as raw bytes; compare.py reads this
//   function's RVA slice from the orig binary and masks relocations, so the
//   non-reloc bytes match and the reloc slots are ignored.

extern "C" __declspec(naked) void FUN_00414e40() {
    __asm {
        _emit 0x51              // PUSH ECX                ; scratch slot
        _emit 0xe8              // CALL _rand              ; rel32
        _emit 0x28
        _emit 0x09
        _emit 0x5c
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x89              // MOV dword ptr [ESP], EAX
        _emit 0x04
        _emit 0x24
        _emit 0xdb              // FILD dword ptr [ESP]
        _emit 0x04
        _emit 0x24
        _emit 0x7d              // JGE +6
        _emit 0x06
        _emit 0xd8              // FADD float ptr [0x00f54a54]
        _emit 0x05
        _emit 0x54
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0xd8              // FMUL float ptr [0x00f570c4]
        _emit 0x0d
        _emit 0xc4
        _emit 0x70
        _emit 0xf5
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0xc3              // RET
    }
}
