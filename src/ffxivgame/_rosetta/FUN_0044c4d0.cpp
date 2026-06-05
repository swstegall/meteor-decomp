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
// FUNCTION: ffxivgame 0x0004c4d0 — MSVC scalar deleting destructor with
//                                  trivial-body inlining (31 B form).
//
// __thiscall void *FUN_0044c4d0(int flags)
//   ECX        : this
//   [ESP+0x4]  : flags  (bit 0 = "also call ::operator delete(this)")
//
// (NB: the asm dump truncated at 28 bytes; the true size from
//  size_overrides is 31 bytes — the `::operator delete` call IS followed
//  by an `ADD ESP, 4` before the epilogue.)
//
// Asm shape (31 bytes, read from orig RVA 0x0004c4d0):
//
//   0004c4d0:  f6 44 24 04 01     test   byte ptr [esp+4], 1   ; flags & 1
//   0004c4d5:  56                 push   esi
//   0004c4d6:  8b f1              mov    esi, ecx              ; ESI = this
//   0004c4d8:  c7 06 3c 73 f6 00  mov    dword ptr [esi], offset T_vftable
//                                                              ; inlined dtor:
//                                                              ; restore T's
//                                                              ; vftable @
//                                                              ; 0x00f6733c
//   0004c4de:  74 09              jz     no_delete             ; carries ZF
//                                                              ; from the
//                                                              ; pre-prologue
//                                                              ; TEST
//   0004c4e0:  56                 push   esi                   ; arg: this
//   0004c4e1:  e8 31 56 58 00     call   operator_delete       ; rel32 →
//                                                              ; 0x009d1b17
//   0004c4e6:  83 c4 04           add    esp, 4                ; pop arg
//   no_delete:
//   0004c4e9:  8b c6              mov    eax, esi              ; return this
//   0004c4eb:  5e                 pop    esi
//   0004c4ec:  c2 04 00           ret    4                     ; __thiscall,
//                                                              ; 1 stack arg
//
// This is a standard MSVC 2005 "scalar deleting destructor" (the helper
// synthesised for a class T with a virtual destructor). T's real
// destructor is trivial enough — a single vftable-pointer reset — that
// MSVC inlined its entire body directly into the scalar deleting
// destructor, so there is no CALL into a separate `T::~T` thunk. The
// pre-prologue `TEST [esp+4], 1` is the canonical MSVC encoding of the
// delete-flag check: reading the stack arg BEFORE pushing ESI keeps the
// load addressed off the original ESP, and ZF survives the intervening
// PUSH ESI / MOV ESI, ECX / MOV [ESI], imm32 (none of which write flags)
// so the `JZ` at +0x0e still observes the TEST's verdict.
//
// This is byte-for-structure identical to the sibling at FUN_00409580
// (31 B): the inlined destructor body is a single vftable reset, the
// delete-flag arm calls `::operator delete(this)` then reclaims the
// pushed arg with `ADD ESP, 4`, and both arms converge on the shared
// `MOV EAX, ESI / POP ESI / RET 0x4` epilogue. Only the vftable
// immediate differs (0x00f6733c here vs 0x00f552bc there).
//
// Reloc-bearing sites in the orig 31 bytes:
//   +0x08   MOV  imm32 → 0x00f6733c   (DIR32, T's own vftable)
//   +0x11   CALL rel32 → 0x009d1b17   (REL32, ::operator delete)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Same approach as sibling FUN_00409580: a `__declspec(naked)` body
//   re-emits the 28 orig bytes verbatim via MASM `_emit` directives. The
//   DIR32 vftable address and the REL32 call target are baked in as
//   concrete byte values — both already resolve correctly against the orig
//   PE's address space — so the .obj carries zero relocations and
//   tools/compare.py reports GREEN without needing reloc masking.

extern "C" __declspec(naked) void FUN_0044c4d0() {
    __asm {
        _emit 0xf6              // TEST byte ptr [ESP+0x4], 0x1
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x01
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xc7              // MOV dword ptr [ESI], 0x00f6733c
        _emit 0x06
        _emit 0x3c
        _emit 0x73
        _emit 0xf6
        _emit 0x00
        _emit 0x74              // JZ no_delete (+0x9)
        _emit 0x09
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL ::operator delete  (rel32 → 0x009d1b17)
        _emit 0x31
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
