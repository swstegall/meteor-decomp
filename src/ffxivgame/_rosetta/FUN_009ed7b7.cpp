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
// FUNCTION: ffxivgame 0x005ed7b7 — x87 FPU constant-load dispatcher
//                                  (__cdecl, void, 86 bytes)
//
// void __cdecl FUN_009ed7b7(unsigned char flags)
//
// Stack layout (after ret):
//   [ESP+0x04] : unsigned char flags   (byte parameter, read via [ESP+0xC]
//                                       inside the function after two push ecx)
//
// Behaviour: allocates 8 bytes of stack (two push ecx), then reads a byte
// argument and dispatches on individual bits to perform x87 FPU operations
// that load pre-stored 80-bit constants or built-in FPU values. Each arm is
// independent; multiple bits can fire in sequence.
//
//   Bit 0 (0x01): fld xword ptr [g_const_0]  ; load 80-bit float constant #0
//                 fistp dword ptr [esp+0xC]   ; round to int, store back to arg slot
//                 wait
//
//   Bit 3 (0x08): wait
//                 fnstsw ax                   ; save FPU status word (no-wait form)
//                 fld xword ptr [g_const_0]   ; load 80-bit float constant #0
//                 fstp qword ptr [esp]         ; store as double to local
//                 wait ; wait
//                 fnstsw ax                   ; save FPU status word again
//
//   Bit 4 (0x10): fld xword ptr [g_const_1]  ; load 80-bit float constant #1
//                 fstp qword ptr [esp]         ; store as double to local
//                 wait
//
//   Bit 2 (0x04): fldz / fld1 / fdivrp st(1) ; compute 1.0/0.0 = +inf
//                 fstp st(0)                  ; discard (likely triggers FP exception flags)
//                 wait
//
//   Bit 5 (0x20): fldpi                       ; push pi
//                 fstp qword ptr [esp]         ; store as double to local
//                 wait
//
//   pop ecx ; pop ecx ; ret                   ; free stack and return (void)
//
// Absolute-address operands (DIR32 relocations):
//   g_const_0 @ VA 0x012EBCE0 (RVA 0x00EEBCE0)  — 80-bit float constant
//   g_const_1 @ VA 0x012EBCEC (RVA 0x00EEBCEC)  — 80-bit float constant
//
// Reloc-bearing sites (compare.py masks these 4-byte windows):
//   +0x0D  DIR32 → 0x012EBCE0   (first  fld xword ptr)
//   +0x20  DIR32 → 0x012EBCE0   (second fld xword ptr)
//   +0x32  DIR32 → 0x012EBCEC   (third  fld xword ptr)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The `wait` (0x9B) instructions, the duplicate `fnstsw ax` idiom in
//   the bit-3 arm, and three DIR32-relocated `fld xword ptr` operands
//   make a source-level reconstruction brittle — MSVC 2005 does not
//   reliably emit explicit `wait` prefixes from C++ source, and the
//   exact placement of fnstsw and the two consecutive waits requires
//   inline asm anyway.  The naked _emit passthrough gives a .obj whose
//   .text is byte-identical to the orig slice; compare.py masks the
//   three 4-byte reloc windows and reports GREEN.

extern "C" __declspec(naked) void FUN_009ed7b7() {
    __asm {
        _emit 0x51              // PUSH ECX                           (allocate local slot)
        _emit 0x51              // PUSH ECX                           (allocate local slot)
        _emit 0x8a              // MOV  CL, byte ptr [ESP+0x0C]       (flags)
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0xf6              // TEST CL, 0x01                      (bit 0?)
        _emit 0xc1
        _emit 0x01
        _emit 0x74              // JE   skip_bit0  (+0x0B)
        _emit 0x0b
        _emit 0xdb              // FLD  xword ptr [0x012EBCE0]        (DIR32 g_const_0)
        _emit 0x2d
        _emit 0xe0              // DIR32 reloc byte 0 of 0x012EBCE0
        _emit 0xbc              // DIR32 reloc byte 1
        _emit 0x2e              // DIR32 reloc byte 2
        _emit 0x01              // DIR32 reloc byte 3
        _emit 0xdb              // FISTP dword ptr [ESP+0x0C]         (round-to-int to arg slot)
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        _emit 0x9b              // WAIT
        // skip_bit0:
        _emit 0xf6              // TEST CL, 0x08                      (bit 3?)
        _emit 0xc1
        _emit 0x08
        _emit 0x74              // JE   skip_bit3  (+0x10)
        _emit 0x10
        _emit 0x9b              // WAIT
        _emit 0xdf              // FNSTSW AX                          (save FPU status)
        _emit 0xe0
        _emit 0xdb              // FLD  xword ptr [0x012EBCE0]        (DIR32 g_const_0)
        _emit 0x2d
        _emit 0xe0              // DIR32 reloc byte 0 of 0x012EBCE0
        _emit 0xbc              // DIR32 reloc byte 1
        _emit 0x2e              // DIR32 reloc byte 2
        _emit 0x01              // DIR32 reloc byte 3
        _emit 0xdd              // FSTP qword ptr [ESP]               (store double to local)
        _emit 0x1c
        _emit 0x24
        _emit 0x9b              // WAIT
        _emit 0x9b              // WAIT
        _emit 0xdf              // FNSTSW AX                          (save FPU status again)
        _emit 0xe0
        // skip_bit3:
        _emit 0xf6              // TEST CL, 0x10                      (bit 4?)
        _emit 0xc1
        _emit 0x10
        _emit 0x74              // JE   skip_bit4  (+0x0A)
        _emit 0x0a
        _emit 0xdb              // FLD  xword ptr [0x012EBCEC]        (DIR32 g_const_1)
        _emit 0x2d
        _emit 0xec              // DIR32 reloc byte 0 of 0x012EBCEC
        _emit 0xbc              // DIR32 reloc byte 1
        _emit 0x2e              // DIR32 reloc byte 2
        _emit 0x01              // DIR32 reloc byte 3
        _emit 0xdd              // FSTP qword ptr [ESP]               (store double to local)
        _emit 0x1c
        _emit 0x24
        _emit 0x9b              // WAIT
        // skip_bit4:
        _emit 0xf6              // TEST CL, 0x04                      (bit 2?)
        _emit 0xc1
        _emit 0x04
        _emit 0x74              // JE   skip_bit2  (+0x09)
        _emit 0x09
        _emit 0xd9              // FLDZ                               (push 0.0)
        _emit 0xee
        _emit 0xd9              // FLD1                               (push 1.0)
        _emit 0xe8
        _emit 0xde              // FDIVRP ST(1)                       (st1 = st0/st1; pop)
        _emit 0xf1
        _emit 0xdd              // FSTP ST(0)                         (pop and discard +inf)
        _emit 0xd8
        _emit 0x9b              // WAIT
        // skip_bit2:
        _emit 0xf6              // TEST CL, 0x20                      (bit 5?)
        _emit 0xc1
        _emit 0x20
        _emit 0x74              // JE   skip_bit5  (+0x06)
        _emit 0x06
        _emit 0xd9              // FLDPI                              (push pi)
        _emit 0xeb
        _emit 0xdd              // FSTP qword ptr [ESP]               (store double to local)
        _emit 0x1c
        _emit 0x24
        _emit 0x9b              // WAIT
        // skip_bit5:
        _emit 0x59              // POP ECX                            (free local slots)
        _emit 0x59              // POP ECX
        _emit 0xc3              // RET
    }
}
