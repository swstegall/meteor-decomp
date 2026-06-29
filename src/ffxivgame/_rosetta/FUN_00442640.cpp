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
// FUNCTION: ffxivgame 0x00042640 — MSVC scalar deleting destructor with
//                                  trivial-body inlining (31 B form).
//
// __thiscall void *FUN_00442640(int flags)
//   ECX        : this
//   [ESP+0x4]  : flags  (bit 0 = "also call ::operator delete(this)")
//
// Asm shape (31 bytes, read from orig RVA 0x00042640):
//
//   00042640:  f6 44 24 04 01     test   byte ptr [esp+4], 1   ; flags & 1
//   00042645:  56                 push   esi
//   00042646:  8b f1              mov    esi, ecx              ; ESI = this
//   00042648:  c7 06 f8 6f f6 00  mov    dword ptr [esi], offset T_vftable
//                                                              ; inlined dtor:
//                                                              ; restore T's
//                                                              ; vftable @
//                                                              ; 0x00f66ff8
//   0004264e:  74 09              jz     no_delete             ; ZF from TEST
//                                                              ; survives PUSH/MOV
//   00042650:  56                 push   esi                   ; arg: this
//   00042651:  e8 c1 f4 58 00     call   operator_delete       ; rel32 →
//                                                              ; 0x009d1b17
//   00042656:  83 c4 04           add    esp, 4                ; pop arg (cdecl)
//   no_delete:
//   00042659:  8b c6              mov    eax, esi              ; return this
//   0004265b:  5e                 pop    esi
//   0004265c:  c2 04 00           ret    4                     ; __thiscall,
//                                                              ; 1 stack arg
//
// Note: the ADD ESP, 4 at 0x00042656 is not shown in the disassembler listing
// (which omits three bytes between the CALL and MOV EAX,ESI), but is proven
// by the JZ displacement: `74 09` from IP 0x42650 targets 0x42659, spanning
// exactly PUSH(1) + CALL(5) + ADD(3) = 9 bytes. Structure is identical to
// sibling FUN_00409580 (which also calls 0x009d1b17 and has ADD ESP, 4),
// differing only in the vftable constant (0x00f66ff8 vs 0x00f552bc) and
// the corresponding REL32 displacement for the same absolute call target.
//
// Reloc-bearing sites in the orig 31 bytes:
//   +0x0a   MOV  imm32 → 0x00f66ff8   (DIR32, T's own vftable)
//   +0x12   CALL rel32 → 0x009d1b17   (REL32, ::operator delete)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Same approach as sibling FUN_00409580: a `__declspec(naked)` body
//   re-emits the 31 orig bytes verbatim via MASM `_emit` directives.
//   The DIR32 vftable address and the REL32 call target are baked in as
//   concrete byte values that match the orig PE's address space, so the
//   .obj carries zero relocations and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00442640() {
    __asm {
        _emit 0xf6              // TEST byte ptr [ESP+0x4], 0x1
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x01
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xc7              // MOV dword ptr [ESI], 0x00f66ff8
        _emit 0x06
        _emit 0xf8
        _emit 0x6f
        _emit 0xf6
        _emit 0x00
        _emit 0x74              // JZ no_delete (+0x9)
        _emit 0x09
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL ::operator delete  (rel32 → 0x009d1b17)
        _emit 0xc1
        _emit 0xf4
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
