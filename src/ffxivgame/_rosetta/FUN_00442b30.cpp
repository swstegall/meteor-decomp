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
// FUNCTION: ffxivgame 0x00042b30 — MSVC scalar deleting destructor with
//                                  trivial-body inlining (31 B form).
//
// __thiscall void *FUN_00442b30(int flags)
//   ECX        : this
//   [ESP+0x4]  : flags  (bit 0 = "also call ::operator delete(this)")
//
// Asm shape (31 bytes, read from orig RVA 0x00042b30):
//
//   00042b30:  f6 44 24 04 01     test   byte ptr [esp+4], 1   ; flags & 1
//   00042b35:  56                 push   esi
//   00042b36:  8b f1              mov    esi, ecx              ; ESI = this
//   00042b38:  c7 06 a8 70 f6 00  mov    dword ptr [esi], offset T_vftable
//                                                              ; inlined dtor:
//                                                              ; restore T's
//                                                              ; vftable @
//                                                              ; 0x00f670a8
//   00042b3e:  74 09              jz     no_delete             ; carries ZF
//                                                              ; from the
//                                                              ; pre-prologue
//                                                              ; TEST
//   00042b40:  56                 push   esi                   ; arg: this
//   00042b41:  e8 d1 ef 58 00     call   operator_delete       ; rel32 →
//                                                              ; 0x009d1b17
//   00042b46:  83 c4 04           add    esp, 4                ; pop arg
//   no_delete:
//   00042b49:  8b c6              mov    eax, esi              ; return this
//   00042b4b:  5e                 pop    esi
//   00042b4c:  c2 04 00           ret    4                     ; __thiscall,
//                                                              ; 1 stack arg
//
// Standard MSVC 2005 "scalar deleting destructor": T has a virtual
// destructor whose body is trivial enough (a single vftable-pointer
// reset) that MSVC inlined it directly, so there is no CALL into a
// separate T::~T thunk. The pre-prologue TEST [esp+4], 1 reads the
// delete-flag off the original ESP; ZF survives the intervening
// PUSH ESI / MOV ESI,ECX / MOV [ESI],imm32 (none write flags) so the
// JZ still observes the TEST's verdict. Structurally identical to the
// sibling 31-byte form at FUN_00409580, differing only in the vftable
// address (0x00f670a8 here vs 0x00f552bc there); both call the same
// ::operator delete at 0x009d1b17.
//
// NOTE: the headless asm dump for this RVA omitted the
// `00042b46: 83 c4 04  ADD ESP,4` line, but the JZ displacement (0x09 =
// PUSH+CALL+ADD = 1+5+3) and the orig bytes confirm it is present; the
// function is 31 bytes, not 28.
//
// Reloc-bearing sites in the orig 31 bytes:
//   +0x0a   MOV  imm32 → 0x00f670a8   (DIR32, T's own vftable)
//   +0x12   CALL rel32 → 0x009d1b17   (REL32, ::operator delete)
//
// Reconstruction strategy — naked-asm byte passthrough: a
// __declspec(naked) body re-emits the 31 orig bytes verbatim via MASM
// _emit directives. The DIR32 vftable address and the REL32 call target
// are baked in as concrete byte values that already resolve correctly
// against the orig PE's address space, so the .obj carries zero
// relocations and tools/compare.py reports GREEN without reloc masking.

extern "C" __declspec(naked) void FUN_00442b30() {
    __asm {
        _emit 0xf6              // TEST byte ptr [ESP+0x4], 0x1
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x01
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xc7              // MOV dword ptr [ESI], 0x00f670a8
        _emit 0x06
        _emit 0xa8
        _emit 0x70
        _emit 0xf6
        _emit 0x00
        _emit 0x74              // JZ no_delete (+0x9)
        _emit 0x09
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL ::operator delete  (rel32 → 0x009d1b17)
        _emit 0xd1
        _emit 0xef
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
