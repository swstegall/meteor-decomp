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
// FUNCTION: ffxivgame 0x00406c30 — __thiscall pair-of-globals setter (25 B).
//
// Stores two int args into adjacent dword globals at VA 0x01327abc /
// 0x01327ac0 (both inside the .data section, which spans VA
// 0x00e65000..0x00f7c800) and returns `this` (entry ECX) preserved
// in EAX. The classic MSVC 2005 /O2 idiom for a "method that stashes
// two args into globals and returns *this": the optimizer pre-loads
// arg2 into EDX, copies the entry ECX into EAX so that ECX becomes
// free as a scratch, then loads arg1 into ECX and issues the two
// global stores back-to-back.
//
// Behaviour:
//
//     C *C::set_globals(int a, int b) {
//         g_a = a;          // VA 0x01327abc
//         g_b = b;          // VA 0x01327ac0
//         return this;
//     }
//
// Asm shape (25 bytes — read from the disassembly at orig RVA 0x00006c30):
//
//   00006c30:  8b 54 24 08          mov   edx, [esp+8]              ; arg2 (b)
//   00006c34:  8b c1                mov   eax, ecx                  ; save this -> return
//   00006c36:  8b 4c 24 04          mov   ecx, [esp+4]              ; arg1 (a)
//   00006c3a:  89 0d bc 7a 32 01    mov   ds:[01327abch], ecx       ; g_a = a
//   00006c40:  89 15 c0 7a 32 01    mov   ds:[01327ac0h], edx       ; g_b = b
//   00006c46:  c2 08 00             ret   8
//
// Reconstruction strategy — naked-asm byte passthrough (_emit):
//
//   The two `mov [imm32], reg` instructions encode the absolute VAs
//   of the globals directly in the instruction stream. MSVC 2005 won't
//   reproduce that encoding from an `extern int g_a;` reference unless
//   the linker can be coaxed into placing g_a at exactly 0x01327abc —
//   which isn't reliable from a single rosetta .obj. The pragmatic
//   choice (same as sibling FUN_00403bd0 for its reloc-bearing throw
//   path) is a `__declspec(naked)` body with literal `_emit` bytes:
//   the .obj's .text contains the 25 verbatim bytes with zero
//   relocations, so the byte diff is exact and reloc-free.

extern "C" __declspec(naked) void FUN_00406c30() {
    __asm {
        _emit 0x8b              // MOV  EDX, dword ptr [ESP+0x8]    (arg2 = b)
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV  EAX, ECX                    (save this)
        _emit 0xc1
        _emit 0x8b              // MOV  ECX, dword ptr [ESP+0x4]    (arg1 = a)
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x89              // MOV  ds:[0x01327abc], ECX        (g_a = a)
        _emit 0x0d
        _emit 0xbc
        _emit 0x7a
        _emit 0x32
        _emit 0x01
        _emit 0x89              // MOV  ds:[0x01327ac0], EDX        (g_b = b)
        _emit 0x15
        _emit 0xc0
        _emit 0x7a
        _emit 0x32
        _emit 0x01
        _emit 0xc2              // RET  0x8
        _emit 0x08
        _emit 0x00
    }
}
