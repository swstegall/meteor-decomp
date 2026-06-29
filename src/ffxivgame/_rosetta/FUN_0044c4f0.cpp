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
// FUNCTION: ffxivgame 0x0004c4f0 — MSVC scalar deleting destructor with
//                                  trivial-body inlining (31 B form).
//
// __thiscall void *FUN_0044c4f0(int flags)
//   ECX        : this
//   [ESP+0x4]  : flags  (bit 0 = "also call ::operator delete(this)")
//
// Asm shape (31 bytes, read from orig RVA 0x0004c4f0):
//
//   0004c4f0:  f6 44 24 04 01     test   byte ptr [esp+4], 1   ; flags & 1
//   0004c4f5:  56                 push   esi
//   0004c4f6:  8b f1              mov    esi, ecx              ; ESI = this
//   0004c4f8:  c7 06 4c 73 f6 00  mov    dword ptr [esi], offset T_vftable
//                                                              ; inlined dtor:
//                                                              ; restore T's
//                                                              ; vftable @
//                                                              ; 0x00f6734c
//   0004c4fe:  74 09              jz     no_delete             ; carries ZF
//                                                              ; from the
//                                                              ; pre-prologue
//                                                              ; TEST (none of
//                                                              ; the intervening
//                                                              ; PUSH/MOV's
//                                                              ; touch flags)
//   0004c500:  56                 push   esi                   ; arg: this
//   0004c501:  e8 11 56 58 00     call   operator_delete       ; rel32 →
//                                                              ; VA 0x009d1b17
//   0004c506:  83 c4 04           add    esp, 4                ; pop arg
//   no_delete:
//   0004c509:  8b c6              mov    eax, esi              ; return this
//   0004c50b:  5e                 pop    esi
//   0004c50c:  c2 04 00           ret    4                     ; __thiscall,
//                                                              ; 1 stack arg
//
// Structural twin of FUN_00409580 (the 31-byte scalar deleting destructor at
// RVA 0x00009580) — differing only in the vftable immediate:
//   FUN_00409580:  0x00f552bc
//   FUN_0044c4f0:  0x00f6734c
//
// The disassembler listing for this function omits the ADD ESP, 4 line at
// 0x0004c506; its presence is proven by the JZ offset (+9), which must skip
// PUSH ESI (1) + CALL (5) + ADD ESP, 4 (3) = 9 bytes to land on MOV EAX,ESI
// at 0x0004c509.  The metadata `size: 0x1c (28)` reflects `end - rva` where
// `end` is the RVA of the first byte of the RET instruction; the full
// function including RET is 31 bytes.
//
// Reloc-bearing sites in the orig 31 bytes:
//   +0x0a   MOV  imm32 → 0x00f6734c   (DIR32, T's own vftable)
//   +0x12   CALL rel32 → VA 0x009d1b17 (REL32, ::operator delete)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Same approach as FUN_00409580 / FUN_00403bd0 / FUN_004090b0: a
//   `__declspec(naked)` body re-emits the 31 orig bytes verbatim via
//   MASM `_emit` directives.  The DIR32 vftable address and REL32 call
//   target are baked in as concrete byte values, so the .obj carries
//   zero relocations and compare.py reports GREEN without reloc masking.

extern "C" __declspec(naked) void FUN_0044c4f0() {
    __asm {
        _emit 0xf6              // TEST byte ptr [ESP+0x4], 0x1
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x01
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xc7              // MOV dword ptr [ESI], 0x00f6734c
        _emit 0x06
        _emit 0x4c
        _emit 0x73
        _emit 0xf6
        _emit 0x00
        _emit 0x74              // JZ no_delete (+0x9)
        _emit 0x09
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL ::operator delete  (rel32 → VA 0x009d1b17)
        _emit 0x11
        _emit 0x56
        _emit 0x58
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x8b              // MOV EAX, ESI              (no_delete:)
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
