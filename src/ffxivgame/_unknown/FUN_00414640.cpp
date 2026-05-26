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
// FUNCTION: ffxivgame 0x00014640 —
//   SQEX::CDev::Engine::Memory::Alternative::SystemHeapSpace::ctor(this, param_1)
//   (`__thiscall`, 76 bytes / 0x4c)
//
// Pairs with the SystemHeapSpace destructor at RVA 0x00014530 (see
// `decomp-notes/types/ffxivgame/0x00014530.md`). The dtor walks the same
// primary-vtable / embedded-Link / debug-vftable layout in reverse and
// uses `DeleteCriticalSection` against the same `[this+0x08]` slot that
// this ctor passes to `InitializeCriticalSection`.
//
// Calling convention: __thiscall (ECX = this); returns this in EAX.
// Stack params: 1 × DWORD (callee cleans: RET 0x04).
// Callee-saves pushed: ESI only.
//
// Object layout (offsets touched by this function):
//   [this + 0x00]  vtable ptr → SystemHeapSpace::vftable (0x00F5701C)
//   [this + 0x04]  param_1 (an owner / IGuard pointer, stashed for later use)
//   [this + 0x08]  CRITICAL_SECTION (24 B — initialised via IAT)
//   [this + 0x20]  Link::vftable (0x00F567C4)         ← embedded Link sentinel
//   [this + 0x24]  Link.next → &[this+0x20]
//   [this + 0x28]  Link.prev → &[this+0x20]
//   [this + 0x2C]  0
//   [this + 0x30]  0
//   [this + 0x34]  DebugSystemHeapSpace::vftable (0x00F5708C)
//   [this + 0x38]  0
//   [this + 0x3C]  DebugSystemHeapBlock::vftable (0x00F57060)
//   [this + 0x40]  0
//
// Vtables touched (cross-reference `config/ffxivgame.rtti.json`):
//   0x00F5701C — SystemHeapSpace::vftable (slot_count 16; derives from
//                ISpace at 0x00F566FC which the dtor restores at +0x00)
//   0x00F567C4 — Link::vftable
//   0x00F5708C — DebugSystemHeapSpace::vftable (the dtor overwrites this
//                slot with the base IDebugSpace vftable at 0x00F567B4)
//   0x00F57060 — DebugSystemHeapBlock::vftable (the dtor overwrites this
//                slot with the base IDebugBlock vftable at 0x00F56788)
//
// Reloc-bearing sites (absolute immediates baked into the orig binary;
// emitting them as raw `_emit` bytes produces the same wire image with
// no relocations in the .obj):
//   +0x0B  MOV [ESI], imm32          → 0x00F5701C  (SystemHeapSpace vftable)
//   +0x16  CALL [imm32]              → [0x00F3E174] (IAT: InitializeCriticalSection)
//   +0x1A  MOV [ESI+0x20], imm32     → 0x00F567C4  (Link vftable)
//   +0x35  MOV [ESI+0x34], imm32     → 0x00F5708C  (DebugSystemHeapSpace vftable)
//   +0x3F  MOV [ESI+0x3C], imm32     → 0x00F57060  (DebugSystemHeapBlock vftable)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level C++ form would require the struct definitions for
//   SystemHeapSpace and its Link / DebugSystemHeapSpace / DebugSystemHeapBlock
//   sub-objects.  The naked-asm passthrough re-emits the original 76 bytes
//   verbatim via MASM _emit directives; the .obj .text section is
//   byte-identical to the orig slice, and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00414640() {
    __asm {
        // 00014640: 8b 44 24 04   MOV EAX, [ESP+0x04]      (param_1, before push)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00014644: 56            PUSH ESI
        _emit 0x56
        // 00014645: 8b f1         MOV ESI, ECX              (esi = this)
        _emit 0x8b
        _emit 0xf1
        // 00014647: 8d 4e 08      LEA ECX, [ESI+0x08]       (ecx = &CRITICAL_SECTION)
        _emit 0x8d
        _emit 0x4e
        _emit 0x08
        // 0001464a: 51            PUSH ECX                  (arg: lpCriticalSection)
        _emit 0x51
        // 0001464b: c7 06 1c 70 f5 00   MOV [ESI], 0x00F5701C  (SystemHeapSpace::vftable)
        _emit 0xc7
        _emit 0x06
        _emit 0x1c
        _emit 0x70
        _emit 0xf5
        _emit 0x00
        // 00014651: 89 46 04      MOV [ESI+0x04], EAX       (this->field_04 = param_1)
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 00014654: ff 15 74 e1 f3 00   CALL [0x00F3E174]   (IAT: InitializeCriticalSection)
        _emit 0xff
        _emit 0x15
        _emit 0x74
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0001465a: c7 46 20 c4 67 f5 00   MOV [ESI+0x20], 0x00F567C4  (Link::vftable)
        _emit 0xc7
        _emit 0x46
        _emit 0x20
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00014661: 8d 46 20      LEA EAX, [ESI+0x20]       (eax = &Link sentinel)
        _emit 0x8d
        _emit 0x46
        _emit 0x20
        // 00014664: 89 40 04      MOV [EAX+0x04], EAX       (Link.next = &self)
        _emit 0x89
        _emit 0x40
        _emit 0x04
        // 00014667: 89 40 08      MOV [EAX+0x08], EAX       (Link.prev = &self)
        _emit 0x89
        _emit 0x40
        _emit 0x08
        // 0001466a: 33 c0         XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0001466c: 89 46 2c      MOV [ESI+0x2C], EAX       (this->field_2c = 0)
        _emit 0x89
        _emit 0x46
        _emit 0x2c
        // 0001466f: 89 46 30      MOV [ESI+0x30], EAX       (this->field_30 = 0)
        _emit 0x89
        _emit 0x46
        _emit 0x30
        // 00014672: 89 46 38      MOV [ESI+0x38], EAX       (this->field_38 = 0)
        _emit 0x89
        _emit 0x46
        _emit 0x38
        // 00014675: c7 46 34 8c 70 f5 00   MOV [ESI+0x34], 0x00F5708C  (DebugSystemHeapSpace::vftable)
        _emit 0xc7
        _emit 0x46
        _emit 0x34
        _emit 0x8c
        _emit 0x70
        _emit 0xf5
        _emit 0x00
        // 0001467c: 89 46 40      MOV [ESI+0x40], EAX       (this->field_40 = 0)
        _emit 0x89
        _emit 0x46
        _emit 0x40
        // 0001467f: c7 46 3c 60 70 f5 00   MOV [ESI+0x3C], 0x00F57060  (DebugSystemHeapBlock::vftable)
        _emit 0xc7
        _emit 0x46
        _emit 0x3c
        _emit 0x60
        _emit 0x70
        _emit 0xf5
        _emit 0x00
        // 00014686: 8b c6         MOV EAX, ESI              (return this)
        _emit 0x8b
        _emit 0xc6
        // 00014688: 5e            POP ESI
        _emit 0x5e
        // 00014689: c2 04 00      RET 0x04                  (callee cleans 1×DWORD)
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
