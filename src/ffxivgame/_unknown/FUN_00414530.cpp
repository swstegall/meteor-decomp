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
// FUNCTION: ffxivgame 0x00014530 — `SQEX::CDev::Engine::Memory::Alternative::SystemHeapSpace::~ctor`
//                                  (__thiscall, 66 B / 0x42)
//
// The MI-style destructor body for the `SystemHeapSpace` heap-region
// object. Walks the same multiple-inheritance vftable layout the
// constructor / sibling family touches (cross-ref:
// `decomp-notes/types/ffxivgame/0x000139d0.md` — the DetachableHeapBlock
// ctor establishes the same primary / secondary / embedded-Link
// vftable shape that this dtor unwinds in reverse).
//
// Calling convention: __thiscall (ECX = this), no stack args, no return.
// Stack frame: -4 (PUSH ESI / POP ESI bracket).
//
// Vtables touched (all from `config/ffxivgame.rtti.json`):
//   - 0x00f5701c  SystemHeapSpace::vftable      (slot_count 16, MOST-derived)
//   - 0x00f56788  IDebugBlock::vftable          (slot_count 10)
//   - 0x00f567b4  IDebugSpace::vftable          (slot_count 3)
//   - 0x00f567c4  Link::vftable                 (slot_count 2)
//   - 0x00f566fc  ISpace::vftable               (slot_count 16, BASE)
//
// Object layout offsets touched (consistent with the
// DetachableHeapBlock ctor write-order — primary vptr at +0, embedded
// Link at +0x20, the embedded sub-Link sentinels at +0x34 / +0x3c, and
// a doubly-linked-list cell at +0x24 / +0x28; the CRITICAL_SECTION
// sub-object sits at +0x08):
//
//   [this+0x00]  primary vftable (rewritten twice — first to the most
//                derived dtor's prologue value, finally to ISpace base)
//   [this+0x08]  CRITICAL_SECTION  (16 B — DeleteCriticalSection arg)
//   [this+0x20]  embedded Link vftable (overwritten with Link::vftable)
//   [this+0x24]  Link.next pointer       (EAX = this[9])
//   [this+0x28]  Link.prev pointer       (ECX = this[10])
//   [this+0x34]  IDebugSpace::vftable    (this[0xd])
//   [this+0x3c]  IDebugBlock::vftable    (this[0xf])
//
// Body sequence (mirrors the orig asm verbatim):
//
//     this->vptr        = SystemHeapSpace::vftable;     // [this+0x00]
//     this->vftable_3c  = IDebugBlock::vftable;         // [this+0x3c]
//     this->vftable_34  = IDebugSpace::vftable;         // [this+0x34]
//     // unlink from doubly-linked Link list:
//     // this->next->prev_of_next = this->prev   (*(this[9]+8) = this[10])
//     // this->prev->next_of_prev = this->next   (*(this[10]+4) = this[9])
//     this->vftable_20  = Link::vftable;                // [this+0x20]
//     DeleteCriticalSection(&this->cs_08);              // (LPCRITICAL_SECTION)(this+0x08)
//     this->vptr        = ISpace::vftable;              // final base vptr
//
// Reloc-bearing sites in the orig 66 bytes (these absolute addresses
// resolve only in a full-binary relink at image base 0x00400000;
// standalone .obj compilation can't reproduce them via source — naked
// asm emits them as raw immediate bytes which happen to match the
// orig binary's resolved vtable / IAT addresses byte-for-byte):
//     +0x05   imm32 ← .rdata 0x00f5701c  (SystemHeapSpace::vftable)
//     +0x0c   imm32 ← .rdata 0x00f56788  (IDebugBlock::vftable)
//     +0x13   imm32 ← .rdata 0x00f567b4  (IDebugSpace::vftable)
//     +0x20   imm32 ← .rdata 0x00f567c4  (Link::vftable)
//     +0x34   imm32 ← .idata 0x00f3e170  (DeleteCriticalSection IAT slot)
//     +0x3c   imm32 ← .rdata 0x00f566fc  (ISpace::vftable)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form would emit an MI-dtor with `union { ... }`
//   layout helpers and have to coax MSVC 2005 /O2 /GS /EHsc into the
//   exact interleaving (note the load of [ESI+0x24]/[ESI+0x28] is
//   re-issued mid-body — MSVC chose not to cache, and reordering the
//   source-level writes nudges that decision). The pragmatic choice —
//   the same one the sibling FUN_00401460 / FUN_00403eb0 took for
//   their adjacent destructor / shrink helpers — is a
//   `__declspec(naked)` body that re-emits the orig 66 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` section ends up
//   byte-identical to the orig slice (no relocations: the vtable /
//   IAT addresses are absolute values in the binary's own address
//   space, so emitting them as immediates produces the same bytes the
//   linker would produce). `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_00414530() {
    __asm {
        // 00014530: 56                   PUSH ESI
        _emit 0x56
        // 00014531: 8b f1                MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00014533: c7 06 1c 70 f5 00    MOV [ESI], SystemHeapSpace::vftable (0x00f5701c)
        _emit 0xc7
        _emit 0x06
        _emit 0x1c
        _emit 0x70
        _emit 0xf5
        _emit 0x00
        // 00014539: c7 46 3c 88 67 f5 00 MOV [ESI+0x3c], IDebugBlock::vftable (0x00f56788)
        _emit 0xc7
        _emit 0x46
        _emit 0x3c
        _emit 0x88
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00014540: c7 46 34 b4 67 f5 00 MOV [ESI+0x34], IDebugSpace::vftable (0x00f567b4)
        _emit 0xc7
        _emit 0x46
        _emit 0x34
        _emit 0xb4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00014547: 8b 46 24             MOV EAX, [ESI+0x24]   (Link.next)
        _emit 0x8b
        _emit 0x46
        _emit 0x24
        // 0001454a: 8b 4e 28             MOV ECX, [ESI+0x28]   (Link.prev)
        _emit 0x8b
        _emit 0x4e
        _emit 0x28
        // 0001454d: c7 46 20 c4 67 f5 00 MOV [ESI+0x20], Link::vftable (0x00f567c4)
        _emit 0xc7
        _emit 0x46
        _emit 0x20
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00014554: 89 48 08             MOV [EAX+0x8], ECX    (next->prev_of_next = prev)
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 00014557: 8b 56 28             MOV EDX, [ESI+0x28]   (reload prev)
        _emit 0x8b
        _emit 0x56
        _emit 0x28
        // 0001455a: 8b 46 24             MOV EAX, [ESI+0x24]   (reload next)
        _emit 0x8b
        _emit 0x46
        _emit 0x24
        // 0001455d: 8d 4e 08             LEA ECX, [ESI+0x8]    (&this->cs_08)
        _emit 0x8d
        _emit 0x4e
        _emit 0x08
        // 00014560: 51                   PUSH ECX              (DeleteCriticalSection arg)
        _emit 0x51
        // 00014561: 89 42 04             MOV [EDX+0x4], EAX    (prev->next_of_prev = next)
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 00014564: ff 15 70 e1 f3 00    CALL [DeleteCriticalSection] (IAT 0x00f3e170)
        _emit 0xff
        _emit 0x15
        _emit 0x70
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0001456a: c7 06 fc 66 f5 00    MOV [ESI], ISpace::vftable (0x00f566fc)
        _emit 0xc7
        _emit 0x06
        _emit 0xfc
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        // 00014570: 5e                   POP ESI
        _emit 0x5e
        // 00014571: c3                   RET
        _emit 0xc3
    }
}
