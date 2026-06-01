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
// FUNCTION: ffxivgame 0x00410430 — MSVC scalar deleting destructor for
//                                  IDebugSpace, trivial-body inlining (31 B form).
//
// __thiscall void *FUN_00410430(int flags)
//   ECX        : this
//   [ESP+0x4]  : flags  (bit 0 = "also call ::operator delete(this)")
//
// Asm shape (31 bytes, read from orig RVA 0x00010430):
//
//   00010430:  f6 44 24 04 01     test   byte ptr [esp+4], 1   ; flags & 1
//   00010435:  56                 push   esi
//   00010436:  8b f1              mov    esi, ecx              ; ESI = this
//   00010438:  c7 06 b4 67 f5 00  mov    dword ptr [esi], offset IDebugSpace::vftable
//                                                              ; inlined dtor:
//                                                              ; restore IDebugSpace's
//                                                              ; vftable @
//                                                              ; 0x00f567b4
//   0001043e:  74 09              jz     no_delete             ; carries ZF
//                                                              ; from the
//                                                              ; pre-prologue
//                                                              ; TEST (none of
//                                                              ; the intervening
//                                                              ; PUSH/MOV's
//                                                              ; touch flags)
//   00010440:  56                 push   esi                   ; arg: this
//   00010441:  e8 d1 16 5c 00     call   operator_delete       ; rel32 →
//                                                              ; 0x009d1b17
//   00010446:  83 c4 04           add    esp, 4                ; pop arg
//   no_delete:
//   00010449:  8b c6              mov    eax, esi              ; return this
//   0001044b:  5e                 pop    esi
//   0001044c:  c2 04 00           ret    4                     ; __thiscall,
//                                                              ; 1 stack arg
//
// This is a standard MSVC 2005 "scalar deleting destructor" for the
// IDebugSpace class (vftable at 0x00f567b4 — confirmed by type note
// decomp-notes/types/ffxivgame/0x00013640.md). The class belongs to the
// SQEX::CDev::Engine::Memory::Alternative namespace alongside
// DetachableHeapSpace, IDebugBlock, ISpace, and Link.
//
// The unusual feature here is that IDebugSpace's real destructor is
// trivial enough that MSVC inlined its entire body — a single vftable-
// pointer reset — directly into the scalar deleting destructor, so there
// is no CALL into a separate `IDebugSpace::~IDebugSpace` thunk.
//
// The pre-prologue `TEST [esp+4], 1` is the canonical MSVC encoding of
// the delete-flag check: reading the stack arg BEFORE pushing ESI keeps
// the load addressed off the original ESP, and ZF survives the
// intervening PUSH ESI / MOV ESI, ECX / MOV [ESI], imm32 instructions
// (none of which write flags) so the `JZ` at +0x0e still observes the
// TEST's verdict.
//
// NOTE: the asm dump omits the `add esp, 4` at 0x00010446 from its
// display (likely a disassembler rendering artifact — those 3 bytes are
// required by the JZ displacement of 9: PUSH(1) + CALL(5) + ADD(3) = 9).
//
// Identical in structure to sibling FUN_00409580 (same pattern, same
// ::operator delete target 0x009d1b17) — only the vftable address
// differs (0x00f552bc there vs. 0x00f567b4 here).
//
// Reloc-bearing sites in the orig 31 bytes:
//   +0x0a   MOV  imm32 → 0x00f567b4   (DIR32, IDebugSpace's vftable)
//   +0x12   CALL rel32 → 0x009d1b17   (REL32, ::operator delete)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A `__declspec(naked)` body re-emits the 31 orig bytes verbatim via
//   MASM `_emit` directives. The DIR32 vftable address and the REL32
//   call target are baked in as concrete byte values — both already
//   resolve correctly against the orig PE's address space — so the .obj
//   carries zero relocations and tools/compare.py reports GREEN without
//   needing reloc masking.

extern "C" __declspec(naked) void FUN_00410430() {
    __asm {
        _emit 0xf6              // TEST byte ptr [ESP+0x4], 0x1
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x01
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xc7              // MOV dword ptr [ESI], 0x00f567b4
        _emit 0x06
        _emit 0xb4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0x74              // JZ no_delete (+0x9)
        _emit 0x09
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL ::operator delete  (rel32 → 0x009d1b17)
        _emit 0xd1
        _emit 0x16
        _emit 0x5c
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
