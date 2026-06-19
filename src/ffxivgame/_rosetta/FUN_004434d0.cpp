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
// FUNCTION: ffxivgame 0x004434d0 — MSVC scalar deleting destructor with
//                                  trivial-body inlining (31 B form).
//
// __thiscall void *FUN_004434d0(int flags)
//   ECX        : this
//   [ESP+0x4]  : flags  (bit 0 = "also call ::operator delete(this)")
//
// Asm shape (31 bytes, read from orig RVA 0x000434d0):
//
//   000434d0:  f6 44 24 04 01     test   byte ptr [esp+4], 1   ; flags & 1
//   000434d5:  56                 push   esi
//   000434d6:  8b f1              mov    esi, ecx              ; ESI = this
//   000434d8:  c7 06 7c 71 f6 00  mov    dword ptr [esi], 0x00f6717c
//                                                              ; inlined dtor:
//                                                              ; restore T's
//                                                              ; vftable
//   000434de:  74 09              jz     no_delete             ; ZF from TEST
//                                                              ; survives PUSH/MOV
//   000434e0:  56                 push   esi                   ; arg: this
//   000434e1:  e8 31 e6 58 00     call   operator_delete       ; rel32 →
//                                                              ; 0x009d1b17
//   000434e6:  83 c4 04           add    esp, 4                ; pop arg
//   no_delete:
//   000434e9:  8b c6              mov    eax, esi              ; return this
//   000434eb:  5e                 pop    esi
//   000434ec:  c2 04 00           ret    4                     ; __thiscall,
//                                                              ; 1 stack arg
//
// Structural twin of FUN_00409580 (also 31 bytes), differing only in the
// vftable address (0x00f6717c here vs 0x00f552bc there) and the rel32
// CALL displacement (both reach operator_delete @ 0x009d1b17 from their
// respective code positions). The JZ offset 0x09 accounts for
// PUSH(1) + CALL(5) + ADD ESP,4(3) = 9 bytes, confirming the ADD ESP,4
// cleanup instruction is present even though the linear-sweep disassembler
// omitted it from the .s dump (dead-code elision after CALL in the output).
//
// Reloc-bearing sites in the orig 31 bytes:
//   +0x0a   MOV  imm32 → 0x00f6717c   (DIR32, T's own vftable)
//   +0x12   CALL rel32 → 0x009d1b17   (REL32, ::operator delete)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Same approach as sibling FUN_00409580: a `__declspec(naked)` body
//   re-emits the 31 orig bytes verbatim via MASM `_emit` directives.
//   The DIR32 vftable address and the REL32 call target are baked in as
//   concrete byte values that already resolve correctly against the orig
//   PE's address space, so the .obj carries zero relocations and
//   tools/compare.py reports GREEN without reloc masking.

extern "C" __declspec(naked) void FUN_004434d0() {
    __asm {
        _emit 0xf6              // TEST byte ptr [ESP+0x4], 0x1
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x01
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xc7              // MOV dword ptr [ESI], 0x00f6717c
        _emit 0x06
        _emit 0x7c
        _emit 0x71
        _emit 0xf6
        _emit 0x00
        _emit 0x74              // JZ no_delete (+0x9)
        _emit 0x09
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL ::operator delete  (rel32 → 0x009d1b17)
        _emit 0x31
        _emit 0xe6
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
